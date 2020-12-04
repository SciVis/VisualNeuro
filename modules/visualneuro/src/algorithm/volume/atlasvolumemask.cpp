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

#include <modules/visualneuro/algorithm/volume/atlasvolumemask.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/volumeramutils.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

void atlasVolumeMask(Volume* mask, const Volume& volume, const Volume& atlas, const std::unordered_set<size_t>& indexFilter) {
    auto resMask = dynamic_cast<VolumeRAMPrecision<uint8_t>*>(mask->getEditableRepresentation<VolumeRAM>());
    auto dim = volume.getDimensions();
    
    mat4 indexToWorld = volume.getCoordinateTransformer().getIndexToWorldMatrix();
    mat4 worldToIndex = atlas.getCoordinateTransformer().getWorldToIndexMatrix();
    auto volRamIndexed = atlas.getRepresentation<VolumeRAM>();

    auto maskData = resMask->getDataTyped();
    
    auto brainRAM = volume.getRepresentation<VolumeRAM>();

    constexpr unsigned char maskSelection{1 << 6};  // 0100 0000
    constexpr unsigned char maskBrain{1 << 7};  // 1000 0000
    util::IndexMapper3D im(dim);

    util::forEachVoxelParallel(*resMask, [&](const size3_t& ind) {
        
        uint8_t maskVal = 0;
        if (!indexFilter.empty()) {
            // Convert position in correlation volume to position in indexed volume
            vec3 worldCoordinates(vec3(indexToWorld * ivec4(ind, 1)));
            auto indexCoordinates(size3_t(worldToIndex * vec4(worldCoordinates, 1.0f)));
            
            // Calculate a value for the voxel only if it's part of a selected region
            auto voxelValue =
                static_cast<size_t>(
            volRamIndexed->dispatch<int, dispatching::filter::Scalars>([indexCoordinates](auto vr) {
                return vr->getDataTyped()[vr->posToIndex(indexCoordinates, vr->getDimensions())];
            }));
            if (indexFilter.find(voxelValue) !=
                indexFilter.end()) {
                maskVal |= maskSelection;
            }
        } 
        auto notZero =
        brainRAM->dispatch<bool, dispatching::filter::Scalars>([ind](auto vr) {
            return vr->getDataTyped()[vr->posToIndex(ind, vr->getDimensions())] !=
            0;
        });
        if (notZero) {
            maskVal |= maskBrain;
        }
        
        maskData[im(ind)] = maskVal;
    });

 
}
}  // namespace inviwo
