/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/visualneuro/processors/volumettest.h>
#include <math.h>
#include <stdio.h>
#include <thread>

#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/progressbar.h>
#include <inviwo/core/util/stdextensions.h>

#if defined(ENABLE_OPENMP)
#include <omp.h>
#endif

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeTTest::processorInfo_{
    "org.inviwo.VolumeTTest",  // Class identifier
    "Volume T-Test",           // Display name
    "Volume Operation",        // Category
    CodeState::Experimental,   // Code state
    Tags::None,                // Tags
};
const ProcessorInfo VolumeTTest::getProcessorInfo() const { return processorInfo_; }

VolumeTTest::VolumeTTest()
    : PoolProcessor()
    , volumeSequenceInport1_("volumeSequenceInport1")
    , volumeSequenceInport2_("volumeSequenceInport2")
    , outport_("outport")
    , pVal_("pVal", "P-Value", 0.05f, 0.0f, 0.5f, 0.05f)
    , tailTest_("tailTest", "Tail Test",
                {{"twoTailedTest", "Both", stats::TailTest::Both},
                 {"rightOneTailedTest", "Greater than", stats::TailTest::Greater},
                 {"leftOneTailedTest", "Less than", stats::TailTest::Less}},
                0)
    , equalVariance_("equalVarince", "Assume equal variance", false) {

    addPort(volumeSequenceInport1_);
    addPort(volumeSequenceInport2_);
    addPort(outport_);

    addProperties(pVal_, tailTest_, equalVariance_);
}

void VolumeTTest::process() {

    const auto calc = [volumesA = volumeSequenceInport1_.getData(),
                       volumesB = volumeSequenceInport2_.getData(), tailTest = tailTest_.get(),
                       equalVariance = equalVariance_.get() ? stats::EqualVariance::Yes
                                                            : stats::EqualVariance::No,
                       p_val = pVal_.get()](pool::Stop stop,
                                            pool::Progress progress) -> std::shared_ptr<Volume> {
        auto calculateScores = [](const std::vector<const VolumeRAM*>& volumes, int vxlNmbr) {
            std::vector<double> scores(volumes.size());

            for (auto index = 0u; index < volumes.size(); index++) {
                volumes[index]->dispatch<void, dispatching::filter::Scalars>(
                    [&scores, index, vxlNmbr,
                     dataMap = &volumes[index]->getOwner()->dataMap](auto vr) {
                        auto ptr_first_vxl = vr->getDataTyped();
                        scores[index] = dataMap->mapFromDataToValue(
                            static_cast<double>(*(ptr_first_vxl + vxlNmbr)));
                    });
            }
            return scores;
        };

        auto dims = volumesA->front()->getDimensions();

        auto nVoxels = dims[0] * dims[1] * dims[2];

        std::vector<const VolumeRAM*> volRamA;
        std::vector<const VolumeRAM*> volRamB;

        std::transform(volumesA->begin(), volumesA->end(), std::back_inserter(volRamA),
                       [](auto vol) { return vol->template getRepresentation<VolumeRAM>(); });
        std::transform(volumesB->begin(), volumesB->end(), std::back_inserter(volRamB),
                       [](auto vol) { return vol->template getRepresentation<VolumeRAM>(); });

        auto vol = std::make_shared<VolumeRAMPrecision<float>>(dims);
        auto resVol = std::make_shared<Volume>(vol);

        float* res = vol->getDataTyped();

        dvec2 minMax(std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest());

        auto startTime = std::chrono::high_resolution_clock::now();
        for (auto vxlNmbr = 0u; vxlNmbr < nVoxels; vxlNmbr++) {

            if (stop) return resVol;

            std::vector<double> groupA = calculateScores(volRamA, vxlNmbr);
            std::vector<double> groupB = calculateScores(volRamB, vxlNmbr);

            auto [t, p] = stats::tTest(groupA, groupB, equalVariance, tailTest);

            // Check if p-value is statistically significant
            // The sign indicate if directedness of the t-Test, e.g. A is greater than B.
            // Other sources:
            // The t distribution is symmetric about zero, so there's really nothing to
            // interpret about the sign of the test statistic.  If the p-value is small enough,
            // you have a significant difference, and otherwise you don't.
            //*(res + vxlNmbr) = p < p_val ? static_cast<float>(std::abs(t)) : 0.f;
            *(res + vxlNmbr) = p < p_val ? static_cast<float>(t) : 0.f;

            minMax.x = std::min(minMax.x, static_cast<double>(*(res + vxlNmbr)));
            minMax.y = std::max(minMax.y, static_cast<double>(*(res + vxlNmbr)));

            if (std::chrono::high_resolution_clock::now() - startTime >
                std::chrono::milliseconds(200)) {
                progress(vxlNmbr, nVoxels);
                startTime = std::chrono::high_resolution_clock::now();
            }
        }

        progress(1.f);
        if (std::abs(minMax.y - minMax.x) < std::numeric_limits<double>::denorm_min()) {
            // Prevent division by zero errors
            minMax.y += std::numeric_limits<double>::denorm_min();
        }
        resVol->dataMap.dataRange = minMax;
        resVol->dataMap.valueRange = minMax;

        resVol->setModelMatrix(volumesA->front()->getModelMatrix());
        resVol->setWorldMatrix(volumesA->front()->getWorldMatrix());

        return resVol;
    };
    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
