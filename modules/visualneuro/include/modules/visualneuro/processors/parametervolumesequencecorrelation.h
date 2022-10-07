/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/columnoptionproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/visualneuro/statistics/ttest.h>
#include <modules/visualneuro/statistics/correlation.h>

namespace inviwo {
class VolumeRAM;

/** \docpage{org.inviwo.ParameterVolumeSequenceCorrelation, Parameter Volume Sequence Correlation}
 * ![](org.inviwo.ParameterVolumeSequenceCorrelation.png?classIdentifier=org.inviwo.ParameterVolumeSequenceCorrelation)
 *
 * Computes correlation between the selected parameter and each voxel where mask is not zero.
 * Each row in the data frame must correspond to an input volume.
 * Brushing and linking can be connected to filter the volume sequence and the dataframe.
 *
 * If one parameter is missing data for a specific volume, this volume is not included in the
 * correlation calculation, neither is it included if it its corresponding row in the dataframe is
 * brushed away.
 *
 * ### Inports
 *   * __volumes__ Volumes for computing correlation.
 *   * __dataFrame__ Parameters with each row corresponding to an input volume.
 *   * __brushing__ Brushing inport for filtering volumes and parameter values.
 *   * __mask__ Skip computation for voxel if 0.
 *
 * ### Outports
 *   * __correlationVolume__ A volume representing correlation values between 0 and 1.
 *
 * ### Properties
 *   * __Compute__ Correlation method.
 *   * __P-Value__ Filter output by p-value.
 *   * __Tail Test__ Two-tailed, right one-tailed, left one-tailed.
 *
 */
class IVW_MODULE_VISUALNEURO_API ParameterVolumeSequenceCorrelation : public PoolProcessor {
public:
    ParameterVolumeSequenceCorrelation();
    virtual ~ParameterVolumeSequenceCorrelation() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // ports
    VolumeSequenceInport volumes_;
    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport brushing_;
    VolumeInport mask_;

    VolumeOutport resCorrelationVolume_;  // Correlation between selected parameter/input volumes

    // properties
    OptionProperty<stats::CorrelationMethod> correlationMethod_;
    FloatProperty pVal_;
    OptionProperty<stats::TailTest> tailTest_;
};

}  // namespace inviwo
