/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <modules/visualneuro/processors/volumesequencemean.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming
// scheme
const ProcessorInfo VolumeSequenceMean::processorInfo_{
    "org.inviwo.VolumeSequenceMerger",  // Class identifier
    "Volume Sequence Mean",             // Display name
    "Volume Sequence Operation",        // Category
    CodeState::Experimental,            // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo VolumeSequenceMean::getProcessorInfo() const { return processorInfo_; }

VolumeSequenceMean::VolumeSequenceMean() : PoolProcessor(), inport_("inport"), outport_("outport") {
    addPort(inport_);
    addPort(outport_);
}

void VolumeSequenceMean::process() {
    const auto calc = [volumes = inport_.getData()](
                          pool::Stop stop, pool::Progress progress) -> std::shared_ptr<Volume> {
        progress(0.f);
        auto dims = (volumes->front()->getDimensions());
        auto nVoxels = dims[0] * dims[1] * dims[2];

        auto newVolume = std::make_shared<VolumeRAMPrecision<float>>(dims);
        auto resVolume = std::make_shared<Volume>(newVolume);
        // Points to first voxel, changes on 'res' changes 'result'
        float* res = newVolume->getDataTyped();
        double residual = 1.0 / static_cast<float>(volumes->size());
        dvec2 dataRange(0);
        dvec2 valueRange(0);

        for (auto i = 0; i < static_cast<int>(volumes->size()); i++) {
            auto volume = (*volumes)[i];
            auto volumeRAM = volume->getRepresentation<VolumeRAM>();
            auto& dataMap = volume->dataMap_;
            dataRange += residual * dataMap.dataRange;
            valueRange += residual * dataMap.valueRange;
            // Dispatch function
            volumeRAM->dispatch<void, dispatching::filter::Scalars>([&res, dataMap, nVoxels,
                                                                     residual](auto vr) {
                auto data = vr->getDataTyped();  // pointer to first voxel

                for (auto index = 0u; index < nVoxels; index++) {
                    *(res + index) +=
                        static_cast<float>(residual * dataMap.mapFromDataToValue(
                                                          static_cast<double>(*(data + index))));
                }
            });
            // Exit function if this is not the latest job
            if (stop) return resVolume;
            // Update the progress
            progress(i, volumes->size());
        }
        // Data is stored in value domain
        resVolume->dataMap_.dataRange = valueRange;
        resVolume->dataMap_.valueRange = valueRange;

        resVolume->setModelMatrix(volumes->front()->getModelMatrix());
        resVolume->setWorldMatrix(volumes->front()->getWorldMatrix());
        progress(1.f);

        return resVolume;
    };

    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
