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

#pragma once

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.CameraPositionController, Camera Position Controller}
 * ![](org.inviwo.CameraPositionController.png?classIdentifier=org.inviwo.CameraPositionController)
 * Controls the camera property through three buttons. Each button aligns the camera with
 * corresponding axis. This processor can has no inports or outports but is meant to be synchronized
 * to other processors with the camera property to give the user a way to quickly jump between
 * different camera orientations or to reset the camera to one of the three default states given by
 * this processor.
 *
 * ### Properties
 *   * __Side__ Moves the camera to the side of the volume (1, 0, 0).
 *   * __Front__ Moves the camera to the front of the volume (0, 0, 1).
 *   * __Top__ Moves the camera to the top of the volume (0, 1, 0).
 *   * __Camera__ Property that is to be synchronized to other camera property which to control.
 */
class IVW_MODULE_VISUALNEURO_API CameraPositionController : public Processor {
public:
    CameraPositionController();
    virtual ~CameraPositionController() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ButtonProperty sideViewButton_;
    ButtonProperty frontViewButton_;
    ButtonProperty topViewButton_;
    CameraProperty camera_;
};

}  // namespace inviwo

