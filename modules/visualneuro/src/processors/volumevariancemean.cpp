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

#include <modules/visualneuro/processors/volumevariancemean.h>

#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeVarianceMean::processorInfo_{
    "org.inviwo.VolumeVarianceMean",  // Class identifier
    "Volume Variance Mean",           // Display name
    "Volume Sequence Operation",      // Category
    CodeState::Experimental,          // Code state
    "Statistics",                     // Tags
};
const ProcessorInfo VolumeVarianceMean::getProcessorInfo() const { return processorInfo_; }

VolumeVarianceMean::VolumeVarianceMean()
    : Processor()
    , inport_("inport")
    , meanOutport_("meanoutport")
    , varianceOutport_("sdevoutport")
    , setStatistics_("setstatistics", "Set Statistics:", InvalidationLevel::InvalidResources) {

    addPort(inport_);
    addPort(meanOutport_);
    addPort(varianceOutport_);
    addProperty(setStatistics_);

    setStatistics_.addOption("variance", "Variance", 0);
    setStatistics_.addOption("sdev", "Standard Deviation", 1);
    setStatistics_.setCurrentStateAsDefault();
}

void VolumeVarianceMean::process() {

    auto volumes = inport_.getData();
    auto dims = (volumes->front()->getDimensions());
    auto nVoxels = dims[0] * dims[1] * dims[2];
    auto nVols = volumes->size();

    auto meanRAMRepresentation = std::make_shared<VolumeRAMPrecision<float>>(dims);
    auto sdevRAMRepresentation = std::make_shared<VolumeRAMPrecision<float>>(dims);
    auto meanVolume = std::make_shared<Volume>(meanRAMRepresentation);
    auto sdevVolume = std::make_shared<Volume>(sdevRAMRepresentation);
    float* meanVol = meanRAMRepresentation->getDataTyped();
    float* varianceVol = sdevRAMRepresentation->getDataTyped();

    std::vector<std::vector<float>> voxelVec(nVols);

    for (auto i = 0u; i < nVols; i++) {

        auto volume = (*volumes)[i];
        auto volumeRAM = volume->getRepresentation<VolumeRAM>();

        // Calculate the mean for each voxel value
        volumeRAM->dispatch<void, dispatching::filter::Scalars>(
            [&meanVol, nVoxels, nVols](auto vr) {
                auto data = vr->getDataTyped();

                for (auto j = 0u; j < nVoxels; j++) {
                    meanVol[j] += static_cast<float>(data[j]) / static_cast<float>(nVols);
                }
            });
    }

    for (auto i = 0u; i < nVols; i++) {

        auto volume = (*volumes)[i];
        auto volumeRAM = volume->getRepresentation<VolumeRAM>();

        // Calculate the variance for each voxel value
        volumeRAM->dispatch<void, dispatching::filter::Scalars>(
            [&varianceVol, &meanVol, nVoxels, nVols](auto vr) {
                auto data = vr->getDataTyped();

                for (auto j = 0u; j < nVoxels; j++) {
                    varianceVol[j] += static_cast<float>(pow((data[j] - meanVol[j]), 2)) /
                                      static_cast<float>(nVols - 1);
                }
            });
    }

    if (setStatistics_.get() == 1) {
        for (auto i = 0u; i < nVoxels; i++) {
            varianceVol[i] = std::sqrt(varianceVol[i]);
        }
    }

    meanVolume->dataMap.dataRange = volumes->front()->dataMap.dataRange;
    sdevVolume->dataMap.dataRange = volumes->front()->dataMap.dataRange;
    meanVolume->dataMap.valueRange = meanVolume->dataMap.dataRange;
    sdevVolume->dataMap.valueRange = sdevVolume->dataMap.dataRange;

    meanVolume->setModelMatrix(volumes->front()->getModelMatrix());
    sdevVolume->setModelMatrix(volumes->front()->getModelMatrix());
    meanVolume->setWorldMatrix(volumes->front()->getWorldMatrix());
    sdevVolume->setWorldMatrix(volumes->front()->getWorldMatrix());

    meanOutport_.setData(meanVolume);
    varianceOutport_.setData(sdevVolume);
}

}  // namespace inviwo
