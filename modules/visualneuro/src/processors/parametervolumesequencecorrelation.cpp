/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/visualneuro/processors/parametervolumesequencecorrelation.h>
#include <modules/visualneuro/statistics/pearsoncorrelation.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ParameterVolumeSequenceCorrelation::processorInfo_{
    "org.inviwo.ParameterVolumeSequenceCorrelation",  // Class identifier
    "Parameter Volume Sequence Correlation",          // Display name
    "Statistics",                                     // Category
    CodeState::Experimental,                          // Code state
    Tags::None,                                       // Tags
};
const ProcessorInfo ParameterVolumeSequenceCorrelation::getProcessorInfo() const {
    return processorInfo_;
}

ParameterVolumeSequenceCorrelation::ParameterVolumeSequenceCorrelation()
    : PoolProcessor()
    , volumes_("volumes")
    , dataFrame_("dataFrame")
    , brushing_("brushing", BrushingModification::Filtered | BrushingModification::Selected)
    , mask_("mask")
    , resCorrelationVolume_("correlationVolume")
    , correlationMethod_("correlationMethod", "Compute",
                         {{"pearson", "Pearson", stats::CorrelationMethod::Pearson},
                          {"spearman", "Spearman", stats::CorrelationMethod::Spearman}},
                         1)
    , pVal_("pVal", "P-Value", 0.05f, 0.0f, 0.5f, 0.05f)
    , tailTest_("tailTest", "Tail Test",
                {{"twoTailedTest", "Two-tailed test", stats::TailTest::Both},
                 {"rightOneTailedTest", "Right one-tailed test", stats::TailTest::Greater},
                 {"leftOneTailedTest", "Left one-tailed test", stats::TailTest::Less}},
                0) {

    addPort(volumes_);
    addPort(dataFrame_);
    addPort(brushing_);
    addPort(mask_);
    addPort(resCorrelationVolume_);

    addProperty(correlationMethod_);
    addProperty(pVal_);
    addProperty(tailTest_);
}

void ParameterVolumeSequenceCorrelation::process() {

    const auto calc = [volumes = volumes_.getData(), brushing = brushing_.getManager(),
                       dataFrame = dataFrame_.getData(), mask = mask_.getData(),
                       tailTest = *tailTest_, correlationMethod = *correlationMethod_,
                       pVal = *pVal_](pool::Stop stop,
                                      pool::Progress progress) -> std::shared_ptr<Volume> {
        progress(0.f);

        auto getVoxelValues = [](const std::vector<const VolumeRAM*>& volumeRAMs,
                                 std::shared_ptr<const VolumeSequence> volumeSequence,
                                 size_t vxlNmbr, const std::vector<bool>& filter) {
            std::vector<double> values;
            values.reserve(volumeRAMs.size());

            for (size_t index = 0; index < volumeRAMs.size(); index++) {
                if (filter[index]) continue;

                auto dataMap = (*volumeSequence)[index]->dataMap_;
                volumeRAMs[index]->dispatch<void, dispatching::filter::Scalars>(
                    [&values, index, vxlNmbr, &dataMap](auto vr) {
                        auto ptr_first_vxl = vr->getDataTyped();
                        values.emplace_back(dataMap.mapFromDataToValue(
                            static_cast<double>(*(ptr_first_vxl + vxlNmbr))));
                    });
            }
            return values;
        };

        // Calculate total number of voxels in one volume
        auto dims = volumes->front()->getDimensions();
        auto nVoxels = dims[0] * dims[1] * dims[2];

        // Create RAM represenation to be able to read voxels
        std::vector<const VolumeRAM*> volRam;
        std::transform(volumes->begin(), volumes->end(), std::back_inserter(volRam),
                       [](auto vol) { return vol->template getRepresentation<VolumeRAM>(); });
        // Create matrices for conversion from voxelNmbr -> vec3 index (correlation volume) ->
        // worldPos -> vec3 index (indexed volume)
        mat4 indexToWorld = (*volumes->begin())->getCoordinateTransformer().getIndexToWorldMatrix();
        mat4 worldToIndex = mask->getCoordinateTransformer().getWorldToIndexMatrix();
        auto volRamIndexed = mask->getRepresentation<VolumeRAM>();
        // Create volume and dataframe to write values into
        auto vol = std::make_shared<VolumeRAMPrecision<float>>(dims);
        auto resVol = std::make_shared<Volume>(vol);

        // Get pointer to volumes first voxel
        float* res = vol->getDataTyped();

        dvec2 minMax(std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest());
        const auto& selectedColumns = brushing.getSelectedIndices(BrushingTarget::Column);
        if (selectedColumns.empty()) {
            for (size_t vxlNmbr = 0; vxlNmbr < nVoxels; vxlNmbr++) {
                *(res + vxlNmbr) = 0;
            }
        } else {
            size_t selectedParameter = *selectedColumns.begin();
            std::vector<bool> filter(dataFrame->getColumn(selectedParameter)->getSize(), false);
            const auto& brushed = brushing.getFilteredIndices();
            for (const auto& brushedId : brushed) {
                filter[brushedId] = true;
            }
            auto paramValues =
                dataFrame->getColumn(selectedParameter)
                    ->getBuffer()
                    ->getRepresentation<BufferRAM>()
                    ->dispatch<std::vector<double>, dispatching::filter::Scalars>(
                        [filter](auto brprecision) {
                            using ValueType = util::PrecisionValueType<decltype(brprecision)>;

                            const std::vector<ValueType>& data = brprecision->getDataContainer();
                            std::vector<double> values;
                            values.reserve(data.size());
                            for (auto&& [filtered, value] : util::zip(filter, data)) {
                                if (!filtered)
                                    values.emplace_back(util::glm_convert<double>(value));
                            }
                            return values;
                        });
            // Deal with NaN by removing the corresponding subjects
            std::transform(paramValues.begin(), paramValues.end(), filter.begin(),
                           [](const auto& v) -> bool { return std::isnan(v); });
            util::erase_remove_if(paramValues, [](const auto& v) { return std::isnan(v); });
            auto startTime = std::chrono::high_resolution_clock::now();
            for (size_t vxlNmbr = 0; vxlNmbr < nVoxels; vxlNmbr++) {
                // Exit function if this is not the latest job
                if (stop) return resVol;
                // Convert voxelNmbr to ivec3
                size_t x = vxlNmbr % dims.x;
                size_t z = vxlNmbr / (dims.x * dims.y);
                size_t y = (vxlNmbr / dims.x) - (z * dims.y);
                const ivec3 index(x, y, z);

                // Convert position in correlation volume to position in indexed volume
                vec3 worldCoordinates(vec3(indexToWorld * ivec4(index, 1)));
                ivec3 indexCoordinates(ivec3(worldToIndex * vec4(worldCoordinates, 1.0f)));

                // Calculate a value for the voxel only if it's part of a selected region
                auto showVoxel = volRamIndexed->getAsDouble(indexCoordinates) != 0.0;

                if (showVoxel) {

                    // Create a vector with all values for current voxel
                    std::vector<double> voxelValues =
                        getVoxelValues(volRam, volumes, vxlNmbr, filter);
                    auto [corr, p] =
                        stats::corrTest(paramValues, voxelValues, correlationMethod, tailTest);
                    // Check if p-value is statistically significant
                    *(res + vxlNmbr) = (p < pVal) ? static_cast<float>(corr) : 0.f;

                } else {
                    *(res + vxlNmbr) = 0.f;
                }
                minMax.x = std::min(minMax.x, static_cast<double>(*(res + vxlNmbr)));
                minMax.y = std::max(minMax.y, static_cast<double>(*(res + vxlNmbr)));
                if (std::chrono::high_resolution_clock::now() - startTime >
                    std::chrono::milliseconds(200)) {
                    progress(vxlNmbr, nVoxels);
                    startTime = std::chrono::high_resolution_clock::now();
                }
            }
        }

        // Set data range and value range to (-1,1) since that's the range of the Pearson/Spearman
        // correlation.
        resVol->dataMap_.dataRange = dvec2(-1.0, 1.0);
        resVol->dataMap_.valueRange = dvec2(-1.0, 1.0);

        resVol->setModelMatrix(volumes->front()->getModelMatrix());
        resVol->setWorldMatrix(volumes->front()->getWorldMatrix());

        progress(1.f);

        return resVol;
    };


    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        resCorrelationVolume_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
