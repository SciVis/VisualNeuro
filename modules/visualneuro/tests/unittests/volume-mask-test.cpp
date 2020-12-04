/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/visualneuro/algorithm/volume/atlasvolumemask.h>
#include <modules/base/algorithm/volume/volumegeneration.h>


namespace inviwo {

TEST(atlasVolumeMask, atlasVolumeMaskIsCorrect) {
    auto dim = size3_t{256};
    const size3_t mid{(dim - size3_t{1u}) / size_t{2}};
    auto vol = util::makeSphericalVolume<float>(dim);
    auto atlas = util::makeSingleVoxelVolume<uint8_t>(dim); 
    std::unordered_set<size_t> indexFilter = {1, 255};
    auto mask = util::makeSingleVoxelVolume<uint8_t>(dim);
    {
    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    atlasVolumeMask(mask.get(), *vol, *atlas, indexFilter);
    
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    }
    auto resMask = dynamic_cast<VolumeRAMPrecision<uint8_t>*>(mask->getEditableRepresentation<VolumeRAM>());
    auto maskData = resMask->getDataTyped();
    constexpr unsigned char maskSelection{1 << 6};  // 0100 0000
    constexpr unsigned char maskBrain{1 << 7};  // 1000 0000
    EXPECT_EQ(maskData[resMask->posToIndex(mid, resMask->getDimensions())], maskSelection | maskBrain)
        << "Atlas mask computation is not correct.";
}

}  // namespace inviwo
