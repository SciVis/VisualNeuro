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

#include <modules/visualneuro/processors/camerapositioncontroller.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CameraPositionController::processorInfo_{
    "org.inviwo.CameraPositionController",      // Class identifier
    "Camera Position Controller",                // Display name
    "Property Synchronizer",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo CameraPositionController::getProcessorInfo() const { return processorInfo_; }

CameraPositionController::CameraPositionController()
    : Processor()
    , sideViewButton_("sideviewbutton", "Side")
    , frontViewButton_("frontviewbutton", "Front")
    , topViewButton_("topviewbutton", "Top")
    , camera_("camera", "Camera") {

    addProperty(sideViewButton_);
    addProperty(frontViewButton_);
    addProperty(topViewButton_);
    addProperty(camera_);
    camera_.setReadOnly(true);

    sideViewButton_.onChange([&]() {
        float radius = glm::length(camera_.getLookTo() - camera_.getLookFrom());
        camera_.setLookFrom(vec3(-radius, 0.0, 0.0));
        camera_.setLookUp(vec3(0.0, 0.0, 1.0));
    });

    frontViewButton_.onChange([&]() {
        float radius = glm::length(camera_.getLookTo() - camera_.getLookFrom());
        camera_.setLookFrom(vec3(0.0, radius, 0.0));
        camera_.setLookUp(vec3(0.0, 0.0, 1.0));
    });

    topViewButton_.onChange([&]() {
        float radius = glm::length(camera_.getLookTo() - camera_.getLookFrom());
        camera_.setLookFrom(vec3(0.0, 0.0, radius));
        camera_.setLookUp(vec3(0.0, 1.0, 0.0));
    });
}

void CameraPositionController::process() {
}

}  // namespace inviwo
