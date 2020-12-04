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

#include <modules/visualneuro/processors/joindataframes.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo JoinDataFrames::processorInfo_{
    "org.inviwo.JoinDataFrames",    // Class identifier
    "Join Data Frames",             // Display name
    "Data Frame",                    // Category
    CodeState::Stable,        // Code state
    "CPU, DataFrame, Spreadsheet",  // Tags
};
const ProcessorInfo JoinDataFrames::getProcessorInfo() const { return processorInfo_; }

JoinDataFrames::JoinDataFrames()
    : Processor(), dataFrames("dataFrames"), joinedFrame("joinedDataFrame") {

    addPort(dataFrames);
    addPort(joinedFrame);
}

void JoinDataFrames::process() {

    auto res = std::make_shared<DataFrame>(*(*dataFrames.begin()));
    for (auto dataFrame = ++dataFrames.begin(); dataFrame != dataFrames.end(); ++dataFrame) {
        for (auto row = 0u; row < dataFrame->getNumberOfRows(); row++) {
            auto dataItems = dataFrame->getDataItem(row, true);
            std::vector<std::string> rowItems;
            rowItems.reserve(dataItems.size());
            for (auto item = ++dataItems.begin(); item != dataItems.end(); item++) {
                rowItems.emplace_back((*item)->toString());
            }
            res->addRow(rowItems);
        }
    }
    auto fromPortColumn = res->addCategoricalColumn("Processor", res->getNumberOfRows());
    auto idx = 0;
    for (auto elem : dataFrames.getSourceVectorData()) {
        for (auto row = 0u; row < elem.second->getNumberOfRows(); row++) {
            fromPortColumn->set(idx++, elem.first->getProcessor()->getIdentifier());
        }
    }
    res->updateIndexBuffer();
    joinedFrame.setData(res);
}

}  // namespace inviwo
