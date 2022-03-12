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

#include <modules/visualneuro/processors/volumeatlascenterpositions.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeAtlasCenterPositions::processorInfo_{
    "org.inviwo.VolumeAtlasCenterPositions",  // Class identifier
    "Volume Atlas Center Positions",          // Display name
    "Volume Operation",                       // Category
    CodeState::Experimental,                  // Code state
    Tags::None,                               // Tags
};
const ProcessorInfo VolumeAtlasCenterPositions::getProcessorInfo() const { return processorInfo_; }

VolumeAtlasCenterPositions::VolumeAtlasCenterPositions()
    : Processor(), indexedVolume_("indexedVolume"), atlasAggregateInfo_("regionPositions") {

    addPort(indexedVolume_);
    addPort(atlasAggregateInfo_);
}

void VolumeAtlasCenterPositions::process() {

    // Create a map between each index in the volume and all the voxel positions associated to
    // that index.
    std::map<int, std::vector<vec3>> regionVoxels;

    std::shared_ptr<const Volume> indexedVolume = indexedVolume_.getData();
    auto volumeRAM = indexedVolume->getRepresentation<VolumeRAM>();

    const mat4 indexToWorld = indexedVolume->getCoordinateTransformer().getIndexToWorldMatrix();
    size3_t dims = indexedVolume->getDimensions();
    util::IndexMapper3D indexMapper(dims);
    // Fill the region-voxel map with voxels
    volumeRAM->dispatch<void, dispatching::filter::Scalars>(
        [&regionVoxels, dims, indexToWorld, indexMapper](auto vr) {
            size_t nVoxels = dims.x * dims.y * dims.z;
            auto firstVxl = vr->getDataTyped();
            for (size_t vxlNmbr = 0; vxlNmbr < nVoxels; vxlNmbr++) {
                int regionIndex = static_cast<int>(*(firstVxl + vxlNmbr));
                vec3 voxelPos(vec3(indexToWorld * ivec4(indexMapper(vxlNmbr), 1)));
                regionVoxels[regionIndex].push_back(voxelPos);
            }
        });

    // Calculate the mean position for every region
    std::map<int, vec3> regionMeanPositions;
    for (auto region : regionVoxels) {
        vec3 meanPosition(0);
        for (auto voxelPosition : region.second) {
            meanPosition += voxelPosition;
        }
        regionMeanPositions[region.first] = meanPosition / region.second.size();
    }

    std::map<int, vec3> regionCenterpoints;
    std::map<int, size_t> regionVoxelNumber;
    // For every region mean position, find the position of the voxel that is closest to the
    // mean. So that the end position is guaranteed to be inside the region.
    for (auto region : regionVoxels) {
        vec3 regionMean = regionMeanPositions[region.first];
        vec3 regionCenterpoint = regionMean;
        float smallestDistance = std::numeric_limits<float>::max();
        for (vec3 voxel : region.second) {
            if (distance(voxel, regionMean) < smallestDistance) {
                smallestDistance = distance(voxel, regionMean);
                regionCenterpoint = voxel;
            }
        }
        regionCenterpoints[region.first] = regionCenterpoint;
        regionVoxelNumber[region.first] = region.second.size();
    }

    // Create and fill dataframe with calculated region positions
    std::vector<int> indices;
    indices.reserve(regionCenterpoints.size());
    std::vector<double> centerX;
    std::vector<double> centerY;
    std::vector<double> centerZ;
    centerX.reserve(regionCenterpoints.size());
    centerY.reserve(regionCenterpoints.size());
    centerZ.reserve(regionCenterpoints.size());
    std::vector<float> coverage;
    coverage.reserve(regionCenterpoints.size());
    for (auto entry : regionCenterpoints) {
        indices.push_back(entry.first);
        centerX.push_back(entry.second.x);
        centerY.push_back(entry.second.y);
        centerZ.push_back(entry.second.z);
        coverage.push_back(regionVoxelNumber[entry.first] /
                           static_cast<float>(dims.x * dims.y * dims.z));
    }
    auto resDataFrame = std::make_shared<DataFrame>();
    resDataFrame->addColumnFromBuffer("Region index", util::makeBuffer<int>(std::move(indices)));

    resDataFrame->addColumnFromBuffer("Center x", util::makeBuffer<double>(std::move(centerX)));
    resDataFrame->addColumnFromBuffer("Center y", util::makeBuffer<double>(std::move(centerY)));
    resDataFrame->addColumnFromBuffer("Center z", util::makeBuffer<double>(std::move(centerZ)));

    resDataFrame->addColumnFromBuffer("Region volume coverage",
                                      util::makeBuffer<float>(std::move(coverage)));

    resDataFrame->updateIndexBuffer();

    atlasAggregateInfo_.setData(resDataFrame);
}

}  // namespace inviwo
