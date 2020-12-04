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
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/visualneuro/statistics/ttest.h>

#include <future>

namespace inviwo {

/** \docpage{org.inviwo.VolumeTTest, Volume TTest}
 * ![](org.inviwo.VolumeTTest.png?classIdentifier=org.inviwo.VolumeTTest)
 * Calculates a T-Test for the difference between two different volume sequences. 
 * Input two volume sequences (n > 20 preferably).
 * The p-value property can be set to decide the significance of the difference and the user can set
 whether it should be a two-tailed, right one-tailed or left one-tailed test in the
 tail-test-property.
 * The processor uses the first input volume to get the model matrix.
 * The output volume will contain the t-value where the t-Test passes (lower p-value) and zero
 otherwise.
 * ### Inports
 *   * __inport1__ First group.
 *   * __inport2__ Second group.
 *
 * ### Outports
 *   * __outport__ A volume representing correlation values between 0 and 1.
 *
 * ### Properties
 *   * __P-Value__ p-value for the calculation of the t-test.
 *   * __Tail-Test__ Two-tailed, right one-tailed, left one-tailed.
 */
class IVW_MODULE_VISUALNEURO_API VolumeTTest : public PoolProcessor {
public:
    VolumeTTest();
    virtual ~VolumeTTest() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeSequenceInport volumeSequenceInport1_;
    VolumeSequenceInport volumeSequenceInport2_;
    VolumeOutport outport_;

    FloatProperty pVal_;
    TemplateOptionProperty<stats::TailTest> tailTest_;
};

}  // namespace inviwo
