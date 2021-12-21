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

#include <modules/visualneuro/processors/groupcontroller.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GroupController::processorInfo_{
    "org.inviwo.GroupController",  // Class identifier
    "Group Controller",            // Display name
    "NoBrainer",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo GroupController::getProcessorInfo() const { return processorInfo_; }

GroupController::GroupController()
    : Processor()
    , groupA_("groupA")
    , groupB_("groupB")
    , activeGroup_("activeGroup", "Active Group", {{"A", "A", 0}, {"B", "B", 1}}, 0)
    , parameterSelected_("parameterSelected", "A parameter is selected", false) {

    addPort(groupA_);
    addPort(groupB_);
    addProperty(activeGroup_);
    addProperty(parameterSelected_);
}

void GroupController::process() {
    if (groupA_.getManager().isSelectionModified(BrushingTarget::Column)) {
        getNetwork()->getApplication()->dispatchFront([&]() {
            parameterSelected_.set(
                groupA_.getManager().getNumberOfSelected(BrushingTarget::Column) > 0);
        });
    }
    if (groupB_.getManager().isSelectionModified(BrushingTarget::Column)) {
        getNetwork()->getApplication()->dispatchFront([&]() {
            parameterSelected_.set(
                groupB_.getManager().getNumberOfSelected(BrushingTarget::Column) > 0);
        });
    }
}
}  // namespace inviwo
