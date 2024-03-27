/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/visualneuro/processors/volumeregionparametercorrelation.h>
#include <modules/visualneuro/statistics/pearsoncorrelation.h>
#include <modules/visualneuro/statistics/spearmancorrelation.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionParameterCorrelation::processorInfo_{
    "org.inviwo.VolumeRegionParameterCorrelation",  // Class identifier
    "Volume Region Parameter Correlation",          // Display name
    "Statistics",                                   // Category
    CodeState::Experimental,                        // Code state
    Tags::None,                                     // Tags
};
const ProcessorInfo VolumeRegionParameterCorrelation::getProcessorInfo() const {
    return processorInfo_;
}

VolumeRegionParameterCorrelation::VolumeRegionParameterCorrelation()
    : PoolProcessor()
    , volumes_("volumes")
    , dataFrame_("dataFrame")
    , brushing_("brushing", {
        {{BrushingTarget::Row}, BrushingModification::Filtered, InvalidationLevel::InvalidOutput}
    })
    , atlas_("atlas")
    , atlasBrushing_("atlasBrushing", {
        {{BrushingTarget::Row}, BrushingModification::Selected, InvalidationLevel::InvalidOutput}
    })
    , correlations_("correlations")
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
    addPort(atlas_);
    addPort(atlasBrushing_);
    addPort(correlations_);

    addProperty(correlationMethod_);
    addProperty(pVal_);
    addProperty(tailTest_);
}

void VolumeRegionParameterCorrelation::process() {

    const auto calc = [volumes = volumes_.getData(), brushing = brushing_.getManager(),
                       dataFrame = dataFrame_.getData(), atlas = atlas_.getData(),
                       atlasBrushing = atlasBrushing_.getManager(), tailTest = *tailTest_,
                       correlationMethod = *correlationMethod_, pVal = *pVal_](
                          pool::Stop stop, pool::Progress progress) -> std::shared_ptr<DataFrame> {
        progress(0.f);
        auto getFormattedData = [brushing = brushing, dataFrame = dataFrame](
                                    std::vector<std::vector<double>>& parameterValues,
                                    std::vector<std::string>& parameterNames) {
            auto brushedIndices = brushing.getFilteredIndices();

            auto dfSize = dataFrame->getNumberOfRows();
            auto nParameters = dataFrame->getNumberOfColumns();
            parameterValues.resize(nParameters);

            for (auto i = 0u; i < dfSize; i++) {
                if (brushing.isFiltered(i)) continue;

                for (auto j = 0u; j < nParameters; j++) {
                    double value = dataFrame->getColumn(j)->getAsDouble(i);
                    parameterValues[j].push_back(value);
                }
            }

            for (const auto& col : *dataFrame) parameterNames.push_back(col->getHeader());
        };

        auto getVoxelValues = [](const std::vector<const VolumeRAM*>& volumeRAMs,
                                 std::shared_ptr<const VolumeSequence> volumeSequence,
                                 size_t vxlNmbr) {
            std::vector<double> values;
            values.reserve(volumeRAMs.size());

            for (size_t index = 0; index < volumeRAMs.size(); index++) {
                auto dataMap = (*volumeSequence)[index]->dataMap;
                volumeRAMs[index]->dispatch<void, dispatching::filter::Scalars>(
                    [&values, vxlNmbr, &dataMap](auto vr) {
                        auto ptr_first_vxl = vr->getDataTyped();
                        values.emplace_back(dataMap.mapFromDataToValue(
                            static_cast<double>(*(ptr_first_vxl + vxlNmbr))));
                    });
            }
            return values;
        };

        std::vector<std::vector<double>> parameterCorrelations;
        parameterCorrelations.resize(dataFrame->getNumberOfColumns());
        if (atlasBrushing.getNumberOfSelected() == 0) {
            for (auto i = 0u; i < parameterCorrelations.size(); i++) {
                // Save correlation between parameter i and current voxel
                std::transform(std::begin(parameterCorrelations[i]),
                               std::end(parameterCorrelations[i]),
                               std::begin(parameterCorrelations[i]), [](auto&) { return 0; });
            }
        } else {
            std::vector<std::vector<double>> parameterValues;
            std::vector<std::string> parameterNames;

            getFormattedData(parameterValues, parameterNames);
            std::vector<double> parameterMeans;
            for (auto parameter : parameterValues)
                parameterMeans.push_back(calculateMeanIgnoreNaNs(parameter));

            // Deal with NaN by removing the corresponding subjects
            std::vector<std::vector<bool>> ignoreSubjects(dataFrame->getNumberOfColumns());
            for (auto&& [paramValues, ignoreSubject] : util::zip(parameterValues, ignoreSubjects)) {
                std::transform(paramValues.begin(), paramValues.end(),
                               std::back_inserter(ignoreSubject),
                               [](const auto& v) -> bool { return std::isnan(v); });
                util::erase_remove_if(paramValues, [](const auto& v) { return std::isnan(v); });
            }

            // Calculate total number of voxels in one volume
            auto dims = volumes->front()->getDimensions();
            auto nVoxels = dims[0] * dims[1] * dims[2];
            // Create RAM represenation to be able to read voxels
            std::vector<const VolumeRAM*> volRam;
            std::transform(volumes->begin(), volumes->end(), std::back_inserter(volRam),
                           [](auto vol) { return vol->template getRepresentation<VolumeRAM>(); });
            // Create matrices for conversion from voxelNmbr -> vec3 index (correlation volume) ->
            // worldPos -> vec3 index (indexed volume)
            mat4 indexToWorld =
                (*volumes->begin())->getCoordinateTransformer().getIndexToWorldMatrix();
            mat4 worldToIndex = atlas->getCoordinateTransformer().getWorldToIndexMatrix();
            // Create matrices and RAM representation of index volume only if that inport is
            // connected

            auto atlasRam = atlas->getRepresentation<VolumeRAM>();

            auto startTime = std::chrono::high_resolution_clock::now();
            for (size_t vxlNmbr = 0; vxlNmbr < nVoxels; vxlNmbr++) {
                // Exit function if this is not the latest job
                if (stop) return std::make_shared<DataFrame>();

                bool showVoxel = true;
                // Convert voxelNmbr to ivec3
                size_t x = vxlNmbr % dims.x;
                size_t z = vxlNmbr / (dims.x * dims.y);
                size_t y = (vxlNmbr / dims.x) - (z * dims.y);
                const ivec3 index(x, y, z);

                // Convert position in correlation volume to position in indexed volume
                vec3 worldCoordinates(vec3(indexToWorld * ivec4(index, 1)));
                ivec3 indexCoordinates(ivec3(worldToIndex * vec4(worldCoordinates, 1.0f)));

                // Calculate a value for the voxel only if it's part of a selected region
                double voxelValue = atlasRam->getAsDouble(indexCoordinates);
                if (!atlasBrushing.isSelected(static_cast<int>(voxelValue)))
                    showVoxel = false;
                if (showVoxel) {
                    // Create a vector with all values for current voxel
                    std::vector<double> voxelValues = getVoxelValues(volRam, volumes, vxlNmbr);

                    // Calculate the correlation between the voxel and all parameters
                    for (auto i = 0u; i < parameterValues.size(); i++) {
                        std::vector<double> voxelValuesNoNaN;
                        voxelValuesNoNaN.reserve(voxelValues.size());
                        for (auto&& [filter, val] : util::zip(ignoreSubjects[i], voxelValues)) {
                            if (!filter) {
                                voxelValuesNoNaN.push_back(val);
                            }
                        }
                        auto [corr, p] = stats::corrTest(parameterValues[i], voxelValuesNoNaN,
                                                         correlationMethod, tailTest);
                        if (p < pVal) {
                            parameterCorrelations[i].push_back(corr);
                        }
                    }
                }
                // Update the progress
                if (std::chrono::high_resolution_clock::now() - startTime >
                    std::chrono::milliseconds(200)) {
                    progress(vxlNmbr, nVoxels);
                    startTime = std::chrono::high_resolution_clock::now();
                }
            }
        }
        // Create dataframe from correlations
        auto resDataFrame = std::make_shared<DataFrame>();
        std::vector<std::string> parameters; 
        std::vector<float> medians, maxCorrs, minCorrs, firstQuartiles, thirdQuartiles;
        for (auto&& [i, parameter] : util::enumerate(parameterCorrelations)) {
            parameters.push_back(dataFrame->getColumn(i)->getHeader());
            if (parameter.empty()) {
                medians.push_back(std::numeric_limits<float>::quiet_NaN());
                maxCorrs.push_back(std::numeric_limits<float>::quiet_NaN());
                minCorrs.push_back(std::numeric_limits<float>::quiet_NaN());
                firstQuartiles.push_back(std::numeric_limits<float>::quiet_NaN());
                thirdQuartiles.push_back(std::numeric_limits<float>::quiet_NaN());
                continue;
            }

            std::sort(parameter.begin(), parameter.end());

            double min = parameter[0];
            double max = parameter[parameter.size() - 1];
            size_t a = static_cast<int>(parameter.size() / 2.0);
            double median = parameter[a];
            size_t b = static_cast<int>(parameter.size() * 0.25);
            double firstQuartile = parameter[b];
            size_t c = static_cast<int>(parameter.size() * 0.75);
            double thirdQuartile = parameter[c];
            
            minCorrs.push_back(static_cast<float>(min));
            maxCorrs.push_back(static_cast<float>(max));
            medians.push_back(static_cast<float>(median));
            firstQuartiles.push_back(static_cast<float>(firstQuartile));
            thirdQuartiles.push_back(static_cast<float>(thirdQuartile));
        }
        resDataFrame->addCategoricalColumn("Parameter", parameters);
        resDataFrame->addColumn<float>("Median_correlation", medians);
        resDataFrame->addColumn<float>("Max_correlation", maxCorrs);
        resDataFrame->addColumn<float>("Min_correlation", minCorrs);
        resDataFrame->addColumn<float>("First_quartile", firstQuartiles);
        resDataFrame->addColumn<float>("Third_quartile", thirdQuartiles);
        resDataFrame->updateIndexBuffer();

        progress(1.f);

        return resDataFrame;
    };
    dispatchOne(calc, [this](std::shared_ptr<DataFrame> result) {
        correlations_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
