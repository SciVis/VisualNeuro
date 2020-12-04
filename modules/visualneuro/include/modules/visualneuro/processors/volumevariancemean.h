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
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeVarianceMean, Volume Variance Mean}
 * ![](org.inviwo.VolumeVarianceMean.png?classIdentifier=org.inviwo.VolumeVarianceMean)
 * Input a volume sequence to the processor and it gives the mean volume as the first output
 and the variance or standard devation volume as the second output.
 *
 * ### Inports
 *   * __inport__ Inport volume sequence.
 *
 * ### Outports
 *   * __meanOutport__ Outputs the mean of the volume sequence.
 *   * __varianceOutport__ Outputs the variance/standard deviation of the volume sequence.
 *
 * ### Properties
 *   * __setStatistics__ Choose whether to use variance or standard devation for second output.
 */
class IVW_MODULE_VISUALNEURO_API VolumeVarianceMean : public Processor {
public:
    VolumeVarianceMean();
    virtual ~VolumeVarianceMean() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeSequenceInport inport_;
    VolumeOutport meanOutport_;
    VolumeOutport varianceOutport_;
    OptionPropertyInt setStatistics_;
};

}  // namespace inviwo
