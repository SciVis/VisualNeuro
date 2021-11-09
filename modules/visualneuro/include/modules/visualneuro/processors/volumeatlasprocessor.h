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
#include <modules/visualneuro/datastructures/volumeatlas.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/interaction/pickingmapper.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeAtlas, Volume Atlas}
 * ![](org.inviwo.VolumeAtlas.png?classIdentifier=org.inviwo.VolumeAtlas)
 * Combines an indexed voxels and a dataframe with labels for each index.
 * The input volume is forwarded to the outport. A transfer function property is modified to
 * create a peak at the selected index, only showing the voxels with that same index.
 *
 *
 * ### Inports
 *   * __atlasVolume__ A volume with indexed voxels.
 *   * __atlas__ A dataframe with labels to the voxel indexes.
 *   * __atlasRegionCenterpoints___ Coordinates with locations of each label.
 *   * __AtlasBrushingAndLinking__ Label selection.
 *
 * ### Outports
 *   * __outport__ The same volume as was sent to inport.
 *   * __atlasTF__ Transfer function showing selected labels.
 *
 * ### Properties
 *   * __Result__ Atlas label at Coordinate.
 *   * __Coordinates__ 3D coordinates of the cursor.
 *   * __Visualize Atlas__ Show/hide the atlas.
 *   * __Hover Color__ Label color when hovering.
 *   * __Selected Color__ Label color when selected.
 *   * __TF & Isovalues__ A transfer function that shows only the voxels with the picked index.
 *   * __World Position__ 3D coordinates of the current lookup in world position (Read only).
 *   * __Select all regions__ Selects all labels.
 *   * __Deselect all regions__ Deselects all labels.
 *   * __Selected Volume Regions__ List of selected labels.
 *   * __Transfer Function__ Describe property.
 *
 */

class IVW_MODULE_VISUALNEURO_API VolumeAtlasProcessor : public Processor {
public:
    VolumeAtlasProcessor();
    virtual ~VolumeAtlasProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void updateTransferFunction();
    void updateBrushing();
    void updateSelectableRegionProperties();
    void handlePicking(PickingEvent*);
    int pickingToLabelId(int pickingID) const;

    dvec3 getLabelCenterPoint(int labelId) const;

    int labelIdToRow(int labelID) const;

    VolumeInport atlasVolume_;
    DataInport<DataFrame> atlasLabels_;
    DataInport<DataFrame> atlasRegionCenterpoints_;
    BrushingAndLinkingInport brushingAndLinking_;
    VolumeOutport outport_;
    DataOutport<TransferFunction> atlasTFOutport_;
    DataOutport<TransferFunction> pickingTFOutport_;

    ButtonProperty selectAllRegions_;
    ButtonProperty deselectAllRegions_;
    CompositeProperty selectedVolumeRegions_;

    StringProperty selectedName_;
    StringProperty hoverName_;
    StringProperty coordinatesString_;
    BoolProperty visualizeAtlas_;
    FloatVec4Property hoverColor_;
    FloatProperty hoverMix_;
    BoolProperty showTooltip_;
    FloatVec3Property selectedColor_;
    FloatProperty selectedOpacity_;
    BoolProperty applyNotSelectedForEmptySelection_;
    FloatVec3Property notSelectedColor_;
    FloatProperty notSelectedMix_;
    FloatProperty notSelectedOpacity_;
    IsoTFProperty isotfComposite_;
    FloatVec3Property worldPosition_;

    BoolProperty enablePicking_;

    PickingMapper atlasPicking_;

    std::unique_ptr<VolumeAtlas> atlas_;
    std::vector<int> pickingToLabelId_;
    std::shared_ptr<TransferFunction> pickingTF_ = std::make_shared<TransferFunction>();

    int hoverAtlasId_ = -1;

    bool brushingDirty_ = true;
};

}  // namespace inviwo
