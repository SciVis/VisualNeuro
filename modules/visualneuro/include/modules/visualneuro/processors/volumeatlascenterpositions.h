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

#ifndef IVW_VOLUMEREGIONPOSITIONS_H
#define IVW_VOLUMEREGIONPOSITIONS_H

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

/**
 * \class VolumeRegionPositions
 * \brief Computes center positions if each label in the input volume.
 */
/** \docpage{org.inviwo.VolumeAtlasCenterPositions, Volume Atlas Center Positions}
 * ![](org.inviwo.VolumeAtlasCenterPositions.png?classIdentifier=org.inviwo.VolumeAtlasCenterPositions)
 *
 * Computes center positions of each label in the input volume.
 * Output is stored in columns "Region position".
 *
 * ### Inports
 *   * __indexedVolume__ Label volume.
 *
 * ### Outports
 *   * __regionPositions__ Center positions of each label in column "Region position".
 *
 */

class IVW_MODULE_VISUALNEURO_API VolumeAtlasCenterPositions : public Processor {
public:
    VolumeAtlasCenterPositions();
    virtual ~VolumeAtlasCenterPositions() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport indexedVolume_;
    DataOutport<DataFrame> atlasAggregateInfo_;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEREGIONPOSITIONS_H
