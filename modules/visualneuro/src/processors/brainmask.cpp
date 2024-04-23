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

#include <modules/visualneuro/processors/brainmask.h>
#include <modules/visualneuro/algorithm/volume/atlasvolumemask.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BrainMask::processorInfo_{
    "org.inviwo.BrainMask",   // Class identifier
    "Brain Mask",             // Display name
    "Atlas",                  // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo BrainMask::getProcessorInfo() const { return processorInfo_; }

BrainMask::BrainMask() : PoolProcessor(), volumes_("brainVol"), maskPort_("mask") {

    addPort(volumes_);
    addPort(maskPort_);
}

void BrainMask::process() {
    const auto calc = [volumes = volumes_.getData()](
                          pool::Stop stop, pool::Progress progress) -> std::shared_ptr<Volume> {
        if (volumes->empty()) {
            return nullptr;
        } else {
            progress(0.f);
            auto firstVol = *volumes->begin();
            auto mask = std::make_shared<Volume>(firstVol->getDimensions(), DataUInt8::get());
            mask->setModelMatrix(firstVol->getModelMatrix());
            mask->setWorldMatrix(firstVol->getWorldMatrix());
            auto resMask = dynamic_cast<VolumeRAMPrecision<uint8_t>*>(
                mask->getEditableRepresentation<VolumeRAM>());
            auto maskData = resMask->getDataTyped();
            util::IndexMapper3D im(mask->getDimensions());
            constexpr unsigned char maskBrain{1 << 7};  // 1000 0000
            for (const auto& volume : *volumes) {
                
                if (stop) return nullptr;

                auto dim = volume->getDimensions();
                if (glm::any(dim != mask->getDimensions())) {
                    throw inviwo::Exception("Expected all volumes to have same resolution");
                }
                auto brainRAM = volume->getRepresentation<VolumeRAM>();
                brainRAM->dispatch<void, dispatching::filter::Scalars>([&](auto vr) {
                    auto data = vr->getDataTyped();
                    using ValueType = util::PrecisionValueType<decltype(vr)>;
                    util::forEachVoxelParallel(*resMask, [&](const size3_t& ind) {
                        if (data[im(ind)] != ValueType{0}) {
                            maskData[im(ind)] = maskBrain;
                        }
                    });
                });
            }
            progress(1.f);
            return mask;
        }
    };

    maskPort_.clear();
    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        maskPort_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
