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

#include <modules/visualneuro/processors/volumeatlasprocessor.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeAtlasProcessor::processorInfo_{
    "org.inviwo.VolumeAtlas",  // Class identifier
    "Volume Atlas",            // Display name
    "Volume Operation",        // Category
    CodeState::Experimental,   // Code state
    Tags::None,                // Tags
};
const ProcessorInfo VolumeAtlasProcessor::getProcessorInfo() const { return processorInfo_; }

VolumeAtlasProcessor::VolumeAtlasProcessor()
    : Processor()
    , atlasVolume_("atlasVolume")
    , atlasLabels_("atlas")
    , atlasRegionCenterpoints_("atlasRegionCenterpoints_")
    , brushingAndLinking_{"AtlasBrushingAndLinking"}
    , outport_("outport")
    , atlasTFOutport_("atlasTF")
    , pickingTFOutport_("pickingTF")
    , selectAllRegions_("selectAllRegions", "Select all regions")
    , deselectAllRegions_("deselectAllRegions", "Deselect all regions")
    , selectedVolumeRegions_("selectedVolumeRegion", "Selected Volume Regions")
    , lookedUpName_("lookedUpName", "Result", "")
    , coordinatesString_("coordinatesString", "Coordinates", "")
    , visualizeAtlas_("visualizeAtlas", "Visualize Atlas", true)
    , hoverColor_("color", "Hover Color", vec4(0, 1.f, 0, 1.f))
    , selectedColor_("selectedColor", "Selected Color", vec4(0, 123.f/255.f, 1.f, 1.f))
    , isotfComposite_("isotfComposite", "Atlas TF & Isovalues")
    , pickingtfComposite_("pickingtfComposite", "Picking TF & Isovalues")
    , worldPosition_("worldPosition", "World Position", vec3(std::numeric_limits<float>::max()), std::pair{-vec3(std::numeric_limits<float>::max()),
                                                                     ConstraintBehavior::Immutable}, std::pair{vec3(std::numeric_limits<float>::max()),
                                                                     ConstraintBehavior::Immutable} )
    , enablePicking_("enablePicking", "Enable Picking", false)
    , pickingTransparency_("pickingTransparency", "Transparency of None-picked regions", 0.05f, 0.0f, 1.0f)
    , pickedLabelID_("pickedLabelID", "Label ID of picked region")
    , pickedLabelName_("pickedLabelName", "Label name of picked region")
    , atlasPicking_(this, 103, [&](PickingEvent *p) { handlePicking(p); }) {

    addPort(atlasVolume_);
    addPort(atlasLabels_);
    addPort(atlasRegionCenterpoints_);
    atlasRegionCenterpoints_.setOptional(true);
    addPort(brushingAndLinking_);
    brushingAndLinking_.setOptional(true);
    addPort(outport_);
    addPort(atlasTFOutport_);
    addPort(pickingTFOutport_);

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });

    addProperty(lookedUpName_);
    lookedUpName_.setReadOnly(true);
    addProperty(coordinatesString_);
    coordinatesString_.setReadOnly(true);
    addProperty(visualizeAtlas_);
    addProperty(hoverColor_);
    hoverColor_.setSemantics(PropertySemantics::Color);
    addProperty(selectedColor_);
    selectedColor_.setSemantics(PropertySemantics::Color);
    addProperty(isotfComposite_);
    addProperty(pickingtfComposite_);

    addProperty(enablePicking_);
    addProperty(pickingTransparency_);
    addProperty(pickedLabelID_);
    pickedLabelID_.setReadOnly(true);
    addProperty(pickedLabelName_);
    pickedLabelName_.setReadOnly(true);
    enablePicking_.onChange([this]() {
        intermediatePickingID_ = -1;
        pickingAtlasId_ = -1;
        pickingTransparency_.setVisible(enablePicking_.get());
        pickedLabelID_.setVisible(enablePicking_.get());
        pickedLabelName_.setVisible(enablePicking_.get());
    });

    addProperty(worldPosition_);
    worldPosition_.setReadOnly(true);

    addProperty(selectAllRegions_);
    addProperty(deselectAllRegions_);
    addProperty(selectedVolumeRegions_);
    selectedVolumeRegions_.onChange([&]() { brushingDirty_ = true; 
        if (atlasRegionCenterpoints_.isReady()) {
            auto regionPositions = atlasRegionCenterpoints_.getData();

            selectedRegions_.clear();
            for (auto region : selectedVolumeRegions_) {
                auto r = dynamic_cast<BoolProperty *>(region);
                if (region->isModified() && r) {
                    int labelId =
                        static_cast<int>((r->getMetaData<IntMetaData>("labelId"))->get());
                    auto labelIdCol = regionPositions->getColumn(1);
                    for (auto row = 0u; row < labelIdCol->getSize(); row++) {
                        if (labelId == static_cast<int>(labelIdCol->getAsDouble(row))) {
                            auto regionPosition = regionPositions->getColumn(2)->getAsDVec3(row);
                            worldPosition_.set(regionPosition);
                            break;
                        }
                    }
                }
            }
        }
    
    });

    deselectAllRegions_.onChange([&]() {
        NetworkLock lock(this);
        pickingAtlasId_ = -1;
        for (auto region : selectedVolumeRegions_) {
            auto r = static_cast<BoolProperty *>(region);
            r->set(false);
        }
        brushingDirty_ = true;
    });
    selectAllRegions_.onChange([&]() {
        NetworkLock lock(this);
        for (auto region : selectedVolumeRegions_) {
            auto r = static_cast<BoolProperty *>(region);
            r->set(true);
        }
        brushingDirty_ = true;
    });

    atlasVolume_.onChange([&]() { updateSelectableRegionProperties(); });
    atlasLabels_.onChange([&]() { updateSelectableRegionProperties(); });
}

void VolumeAtlasProcessor::updateTransferFunction() {
    auto &tf = isotfComposite_.tf_.get();
    auto atlasVolume = atlasVolume_.getData();
    auto atlasLabels = atlasLabels_.getData();

    if (pickingAtlasId_ == -1) {
        pickedLabelID_ = -1;
        pickedLabelName_ = "";
    } else {
        pickedLabelID_ = pickingAtlasId_+1;
        pickedLabelName_ = atlas_->getLabelName(pickedLabelID_);
    }

    auto addSelectedIndex = [&](int labelId, vec4 color) {
        vec2 dataRange = atlasVolume->dataMap_.dataRange;
        delta_ = 1.0 / (dataRange.y - dataRange.x);
        auto normalizedVal = atlas_->getLabelIdNormalized(labelId);

        if (enablePicking_.get() && pickingAtlasId_ >= 0) {
            if (pickingAtlasId_ + 1 != labelId) {
                color.w = pickingTransparency_.get();
            } 
            else {
                color.w = 1.0;
            }
        }

        dvec2 pos1(normalizedVal - delta_ / 2.0, 0);
        dvec2 pos2(normalizedVal, color.w);
        dvec2 pos3(normalizedVal + delta_ / 2.0, 0);

        // clamp the transfer function between 0 and 1
        pos1.x = std::min(std::max(0.0, pos1.x), 1.0);
        pos2.x = std::min(std::max(0.0, pos2.x), 1.0);
        pos3.x = std::min(std::max(0.0, pos3.x), 1.0);

        tf.add(pos1.x, vec4(0, 0, 0, pos1.y));
        tf.add(pos2.x, color);
        tf.add(pos3.x, vec4(0, 0, 0, pos3.y));
    };
    tf.clear();
    if (visualizeAtlas_.get()) {
        addSelectedIndex(hoverAtlasId_, *hoverColor_);
        if (brushingAndLinking_.isConnected()) {
            auto selectedIndicies = brushingAndLinking_.getSelectedIndices();

            for (int selectedIndex : selectedIndicies) {
                auto c = atlas_->getLabelColor(selectedIndex);
                addSelectedIndex(static_cast<int>(selectedIndex), c ? c.value() : *selectedColor_);
            }
        }
    } else {  // hide the visualization through the transfer function if necessary
        auto pos1 = vec2(0, 0);
        auto pos2 = vec2(0, 0);
        auto pos3 = vec2(0, 0);
        tf.add(pos1.x, vec4(0, 0, 0, pos1.y));
        tf.add(pos2.x, vec4(vec3(0), pos2.y));
        tf.add(pos3.x, vec4(0, 0, 0, pos3.y));
    }

    // if voxel val == 0, set alpha to 0 and color to black to hide the atlas
    if (hoverAtlasId_ <= 0) {
        tf.get(1).setColor(vec4(0));
        tf.get(2).setColor(vec4(0));
    }

    auto &pickTF = pickingtfComposite_.tf_.get();
    auto addPickingIndex = [&](int labelId, vec3 color, float alpha) {
        vec2 dataRange = atlasVolume->dataMap_.dataRange;
        delta_ = 1.0 / (dataRange.y - dataRange.x);
        auto normalizedVal = atlas_->getLabelIdNormalized(labelId);

        dvec2 pos1(normalizedVal - delta_ / 2.0, 0);
        dvec2 pos2A(normalizedVal - delta_ / 4.0, alpha);
        dvec2 pos2B(normalizedVal + delta_ / 4.0, alpha);
        dvec2 pos3(normalizedVal + delta_ / 2.0, 0);

        // clamp the transfer function between 0 and 1
        pos1.x = std::min(std::max(0.0, pos1.x), 1.0);
        pos2A.x = std::min(std::max(0.0, pos2A.x), 1.0);
        pos2B.x = std::min(std::max(0.0, pos2B.x), 1.0);
        pos3.x = std::min(std::max(0.0, pos3.x), 1.0);

        pickTF.add(pos1.x, vec4(0, 0, 0, pos1.y));
        pickTF.add(pos2A.x, vec4(color.r, color.g, color.b, pos2A.y));
        pickTF.add(pos2B.x, vec4(color.r, color.g, color.b, pos2B.y));
        pickTF.add(pos3.x, vec4(0, 0, 0, pos3.y));
    };

    if (pickingDirty_) {
        pickTF.clear();
        pickingDirty_ = false;
        addPickingIndex(0, vec3(0.0), 0.f);
        if (brushingAndLinking_.isConnected()) {
            auto regionIndices = atlasLabels_.getData()->getColumn(1);
            for (size_t i = 1; i < regionIndices->getSize()+1; i++) {
                if (brushingAndLinking_.isSelected(i)) {
                    addPickingIndex(i, atlasPicking_.getColor(i - 1), 1.f);
                } else {
                    addPickingIndex(i, vec3(0.0), 1.f);
                }
            }
        }
    }

}  // namespace inviwo

void VolumeAtlasProcessor::process() {
    auto atlasVolume = atlasVolume_.getData();
    auto atlasLabels = atlasLabels_.getData();
    if (atlasVolume_.isChanged() || atlasLabels_.isChanged()) {
        atlas_ = std::make_unique<VolumeAtlas>(atlasVolume, atlasLabels);
        selectedColor_.setVisible(!atlas_->hasColors());
    }
    if (brushingDirty_) updateBrushing();

    // use nearest neighbour interpolation for the volume 3D texture
    TextureUnit unit;
    utilgl::bindTexture(atlasVolume_, unit);
    unit.activate();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    hoverAtlasId_ = atlas_->getLabelId(worldPosition_.get());
    auto area = atlas_->getLabelName(hoverAtlasId_);

    lookedUpName_.set(area);

    // update the coordinates string
    auto pos = worldPosition_.get();

    std::ostringstream strs;
    strs << std::setprecision(1) << std::fixed << "Coordinates: (" << pos.x << ", " << pos.y << ", "
         << pos.z << ")";
    coordinatesString_.set(strs.str());

    updateTransferFunction();

    outport_.setData(atlasVolume);
    atlasTFOutport_.setData(std::make_shared<TransferFunction>(isotfComposite_.tf_.get()));
    pickingTFOutport_.setData(std::make_shared<TransferFunction>(pickingtfComposite_.tf_.get()));
}

void VolumeAtlasProcessor::updateBrushing() {
    brushingDirty_ = false;
    std::unordered_set<size_t> selectedRegions;

    for (auto region : selectedVolumeRegions_) {
        auto r = static_cast<BoolProperty *>(region);
        if (r->get()) {
            int val = static_cast<int>((r->getMetaData<IntMetaData>("labelId"))->get());
            selectedRegions.insert(val);
        }
    }
    brushingAndLinking_.sendSelectionEvent(selectedRegions);

    pickingDirty_ = true;
}

void VolumeAtlasProcessor::updateSelectableRegionProperties() {
    NetworkLock lock(this);
    brushingDirty_ = true;

    std::vector<std::string> propertiesToRemove;
    for (auto regionProperty : selectedVolumeRegions_.getProperties()) {
        propertiesToRemove.push_back(regionProperty->getIdentifier());
    }
    for (auto propertyId : propertiesToRemove) {
        selectedVolumeRegions_.removeProperty(propertyId);
    }

    if (!atlasVolume_.hasData()) return;

    auto regionIndices = atlasLabels_.getData()->getColumn(1);
    auto regionNames = atlasLabels_.getData()->getColumn(2);

    for (size_t i = 0; i < regionIndices->getSize(); i++) {
        auto labelId = static_cast<int>(regionIndices->getAsDouble(i));
        std::string name = regionNames->getAsString(i);

        std::stringstream identifier;
        identifier << labelId << "_" << util::stripIdentifier(name);

        auto newProp = std::make_unique<BoolProperty>(identifier.str(), name, false);
        if (!atlasVolume_.isConnected() || !atlasVolume_.hasData()) newProp->setReadOnly(true);
        newProp->setMetaData<IntMetaData>("labelId", labelId);
        newProp->setMetaData<DoubleMetaData>("rowIndex", static_cast<double>(i));
        newProp->onChange([&]() { brushingDirty_ = true; });
        selectedVolumeRegions_.addProperty(newProp.release());
    }

    atlasPicking_.resize(regionIndices->getSize());
}

void VolumeAtlasProcessor::handlePicking(PickingEvent *p) {
    if (enablePicking_.get()) {
        // Scheme should be to pick a region on double tap
        if (p->getPressState() == PickingPressState::Press &&
            p->getPressItems().count(PickingPressItem::Primary)) {
            int pickId = p->getPickedId();

            //Current picking ID and last pick ID are the same, assume double tap
            if (pickId == intermediatePickingID_) {
                //Same pickId as already chosen, clear picking
                if (pickingAtlasId_ == pickId) {
                    pickingAtlasId_ = -1;
                } else {
                    //New picking was made
                    pickingAtlasId_ = pickId;
                }
                intermediatePickingID_ = -1;
            } else {
                intermediatePickingID_ = pickId;
            }

            p->markAsUsed();
            invalidate(InvalidationLevel::InvalidOutput);
        }
        //Some other picking was made
        //Assuming proper double tap was not made
        int pickId = p->getPickedId();
        if (intermediatePickingID_ != pickId)
            intermediatePickingID_ = -1;
    }
}

}  // namespace inviwo
