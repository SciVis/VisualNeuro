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

#ifndef IVW_VOLUMESEQUENCEFILTER_H
#define IVW_VOLUMESEQUENCEFILTER_H

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/base/properties/volumeinformationproperty.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeSequenceFilter, Volume Sequence Filter}
 * ![](org.inviwo.VolumeSequenceFilter.png?classIdentifier=org.inviwo.VolumeSequenceFilter)
 *
 * Filter a sequence of volumes with brushing & linking. 
 * If all volumes are filtered a single zero-valued volume of same dimensions as the first 
 * input volume will be set as output.
 *
 * ### Inports
 *   * __inport__ Sequence of volumes
 *	 * __inport__ .csv-file with volume file id's in one of the columns
 *	 * __BrushingAndLinking__ inport for brushing & linking interactions
 *
 * ### Outports
 *   * __outport__ Filtered volume sequence
 *   * __selectedVolumesOutport__ Dataframe containing the id's of the currently selected volumes
 *
 * ### Properties
 *	 * __fMRI Scan id__ Choose the column of the .csv file that contains the volume id
 *   * __Volume file id keyword__ A keyword that appears in the volume filename right before the
 *      unique volume id
 *   * __Length of patient id__ Enter the length (number of characters) of the volume id as it is in
 *      the filename
 */
class IVW_MODULE_VISUALNEURO_API VolumeSequenceFilter : public Processor {
public:
    VolumeSequenceFilter();
    virtual ~VolumeSequenceFilter() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeSequenceInport inport_;
    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport brushingAndLinking_;
    VolumeSequenceOutport volumesOutport_;
    DataOutport<DataFrame> dataOutport_;

    DataFrameColumnProperty filenameIdColumn_;

    size_t numberOfBrushed_ = 0;
};

}  // namespace inviwo

#endif  // IVW_VOLUMESEQUENCEFILTER_H
