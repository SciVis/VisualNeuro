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

#pragma once

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/visualneuro/statistics/correlation.h>
#include <modules/visualneuro/statistics/ttest.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeRegionParameterCorrelation, Volume Region Parameter Correlation}
 * ![](org.inviwo.VolumeRegionParameterCorrelation.png?classIdentifier=org.inviwo.VolumeRegionParameterCorrelation)
 * Computes correlation between selected regions and all parameters in input DataFrame.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
 * DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE
 */
/** \docpage{org.inviwo.VolumeRegionParameterCorrelation, Volume Region Parameter Correlation}
 * ![](org.inviwo.VolumeRegionParameterCorrelation.png?classIdentifier=org.inviwo.VolumeRegionParameterCorrelation)
 *
 * Computes correlation between selected regions and all parameters in input DataFrame.
 *
 *
 * ### Inports
 *   * __volumes__ Volumes for computing correlation.
 *   * __dataFrame__ Parameters with each row corresponding to an input volume.
 *   * __brushing__ Brushing inport for filtering volumes and parameter values.
 *   * __atlas__ Region label volume.
 *   * __atlasBrushing__ Selected label regions.
 *
 * ### Outports
 *   * __correlations__ Correlations between selected regions and all parameters.
 *
 * ### Properties
 *   * __Compute__ Correlation method.
 *   * __P-Value__ Filter output by p-value.
 *   * __Tail Test__ Two-tailed, right one-tailed, left one-tailed.
 *
 */

class IVW_MODULE_VISUALNEURO_API VolumeRegionParameterCorrelation : public PoolProcessor {
public:
    VolumeRegionParameterCorrelation();
    virtual ~VolumeRegionParameterCorrelation() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // ports
    VolumeSequenceInport volumes_;
    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport brushing_;
    VolumeInport atlas_;
    BrushingAndLinkingInport atlasBrushing_;

    DataOutport<DataFrame> correlations_;

    // properties
    TemplateOptionProperty<stats::CorrelationMethod> correlationMethod_;
    FloatProperty pVal_;
    TemplateOptionProperty<stats::TailTest> tailTest_;
};

}  // namespace inviwo
