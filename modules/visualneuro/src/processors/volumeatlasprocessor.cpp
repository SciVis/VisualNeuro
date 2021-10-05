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

OrdinalPropertyState<float> ordinalAlpha(
    const float &value, InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput) {
    return {value,
            {0.0f, ConstraintBehavior::Immutable},
            {1.0f, ConstraintBehavior::Immutable},
            0.01f,
            invalidationLevel,
            PropertySemantics::Default};
}

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
    , selectedName_("selectedName", "Selected", "")
    , hoverName_("lookedUpName", "Hover name", "")
    , coordinatesString_("coordinatesString", "Coordinates", "")
    , visualizeAtlas_("visualizeAtlas", "Visualize Atlas", true)
    , hoverColor_("color", "Hover Color", vec4(0, 1.f, 0, 1.f))
    , hoverMix_("hoverMix", "Hover Mix", ordinalAlpha(0.4f))
    , selectedColor_("selectedColor", "Selected Color", vec3(0, 123.f / 255.f, 1.f))
    , selectedOpacity_("selectedOpacity", "Opacity Selected", ordinalAlpha(1.f))
    , notSelectedColor_("notSelectedColor", "Not Selected Color", vec3(0.2))
    , notSelectedMix_("notSelectedMix", "Not Selected Mix", ordinalAlpha(1.f))
    , notSelectedOpacity_("notSelectedOpacity", "Opacity Not selected", ordinalAlpha(0.f))
    , isotfComposite_("isotfComposite", "Atlas TF & Isovalues")
    , worldPosition_(
          "worldPosition", "World Position", vec3(std::numeric_limits<float>::max()),
          std::pair{-vec3(std::numeric_limits<float>::max()), ConstraintBehavior::Immutable},
          std::pair{vec3(std::numeric_limits<float>::max()), ConstraintBehavior::Immutable})
    , enablePicking_("enablePicking", "Enable Picking", false)
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
    selectedName_.setReadOnly(true);
    hoverName_.setReadOnly(true);
    addProperty(selectedName_);
    addProperty(hoverName_);
    
    addProperty(coordinatesString_);
    coordinatesString_.setReadOnly(true);
    addProperty(visualizeAtlas_);
    addProperty(hoverColor_);
    addProperty(hoverMix_);
    hoverColor_.setSemantics(PropertySemantics::Color);
    addProperty(selectedColor_);
    selectedColor_.setSemantics(PropertySemantics::Color);
    addProperty(selectedOpacity_);
    addProperty(notSelectedColor_);
    notSelectedColor_.setSemantics(PropertySemantics::Color);
    addProperty(notSelectedMix_);
    addProperty(notSelectedOpacity_);

    isotfComposite_.setSerializationMode(PropertySerializationMode::None);
    addProperty(isotfComposite_);

    addProperty(enablePicking_);

    addProperty(worldPosition_);
    worldPosition_.setReadOnly(true);

    worldPosition_.onChange([&]() {
        if (!atlas_) {
            return;
        }
        hoverAtlasId_ = atlas_->getLabelId(worldPosition_.get());
        auto area = atlas_->getLabelName(hoverAtlasId_);
        hoverName_.set(area);

        // update the coordinates string
        auto pos = worldPosition_.get();

        std::ostringstream strs;
        strs << std::setprecision(1) << std::fixed << "Coordinates: (" << pos.x << ", " << pos.y
             << ", " << pos.z << ")";
        coordinatesString_.set(strs.str());
    });

    addProperty(selectAllRegions_);
    addProperty(deselectAllRegions_);
    addProperty(selectedVolumeRegions_);
    selectedVolumeRegions_.onChange([&]() {
        brushingDirty_ = true;
        if (atlasRegionCenterpoints_.isReady()) {
            auto regionPositions = atlasRegionCenterpoints_.getData();

            for (auto region : selectedVolumeRegions_) {
                auto r = dynamic_cast<BoolProperty *>(region);
                if (region->isModified() && r) {
                    int labelId = static_cast<int>((r->getMetaData<IntMetaData>("labelId"))->get());
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
        std::stringstream selectedRegions;
        auto oj = util::make_ostream_joiner(selectedRegions, ",");
        for (auto region : selectedVolumeRegions_) {
            if (auto r = dynamic_cast<BoolProperty *>(region); *r) {
                oj = r->getDisplayName();
            }
        }
        selectedName_.set(selectedRegions.str());
    });

    deselectAllRegions_.onChange([&]() {
        NetworkLock lock(this);
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
}

void VolumeAtlasProcessor::updateTransferFunction() {
    auto &tf = isotfComposite_.tf_.get();
    auto atlasVolume = atlasVolume_.getData();
    auto atlasLabels = atlasLabels_.getData();

    auto setLabelColor = [&](int labelId, vec4 color) {
        if (labelId <= 0) return;

        vec2 dataRange = atlasVolume->dataMap_.dataRange;
        auto normalizedVal = atlas_->getLabelIdNormalized(labelId);

        auto it = std::lower_bound(tf.begin(), tf.end(), normalizedVal);
        it->setColor(color);
    };
    for (auto &p : tf) {
        p.setColor(vec4(0.f));
    }
    if (visualizeAtlas_.get()) {
        if (brushingAndLinking_.isConnected()) {
            for (auto label : *atlas_) {
                auto labelId = label.first;
                auto c = atlas_->getLabelColor(labelId);
                vec3 labelColor(c ? vec3(c.value()) : *selectedColor_);
                if (brushingAndLinking_.isSelected(labelId)) {
                    setLabelColor(static_cast<int>(labelId), vec4(labelColor, *selectedOpacity_));

                } else {
                    setLabelColor(static_cast<int>(labelId),
                                  vec4(glm::mix(labelColor, *notSelectedColor_, *notSelectedMix_),
                                       *notSelectedOpacity_));
                }
            }
        }
        auto c = atlas_->getLabelColor(hoverAtlasId_);
        setLabelColor(hoverAtlasId_,
                      c ? glm::mix(c.value(), *hoverColor_, *hoverMix_) : *hoverColor_);
    }

}  // namespace inviwo

void VolumeAtlasProcessor::process() {
    auto atlasVolume = atlasVolume_.getData();
    auto atlasLabels = atlasLabels_.getData();
    if (atlasVolume_.isChanged() || atlasLabels_.isChanged()) {
        atlas_ = std::make_unique<VolumeAtlas>(atlasVolume, atlasLabels);
        selectedColor_.setVisible(!atlas_->hasColors());
        updateSelectableRegionProperties();
    }
    if (brushingDirty_) updateBrushing();

    // use nearest neighbour interpolation for the volume 3D texture
    TextureUnit unit;
    utilgl::bindTexture(atlasVolume_, unit);
    unit.activate();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    updateTransferFunction();

    outport_.setData(atlasVolume);
    atlasTFOutport_.setData(std::make_shared<TransferFunction>(isotfComposite_.tf_.get()));
    if (enablePicking_.get()) {
        pickingTFOutport_.setData(pickingTF_);
    } else {
        pickingTFOutport_.setData(std::make_shared<TransferFunction>());
    }
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
}

void VolumeAtlasProcessor::updateSelectableRegionProperties() {
    NetworkLock lock(this);
    brushingDirty_ = true;
    Property::OnChangeBlocker block(selectedVolumeRegions_);
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
    int maxLabelId = -1;
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
        maxLabelId = std::max(labelId, maxLabelId);
    }

    atlasPicking_.resize(regionIndices->getSize() + 1);
    auto atlasVolume = atlasVolume_.getData();

    auto resizeLabelControlPoints = [&](TransferFunction &tf, size_t nSegments) {
        while (tf.size() > 3 * (nSegments + 1)) {
            tf.remove(tf.back());
        }
        while (tf.size() < 3 * (nSegments + 1)) {
            tf.add(1.0, vec4{0.0});
        }
        std::vector<TFPrimitive *> primitives;
        for (auto &p : tf) {
            primitives.push_back(&p);
        }
        vec2 dataRange = atlasVolume->dataMap_.dataRange;
        auto delta_ = 1.0 / (dataRange.y - dataRange.x);
        for (size_t i = 0; i < regionIndices->getSize(); i++) {
            auto labelId = static_cast<int>(regionIndices->getAsDouble(i));
            auto normalizedVal = atlas_->getLabelIdNormalized(labelId);
            double pos1(normalizedVal - delta_ / 2.0);
            double pos2(normalizedVal);
            double pos3(normalizedVal + delta_ / 2.0);

            // clamp the transfer function between 0 and 1
            pos1 = std::clamp(pos1, 0.0, 1.0);
            pos2 = std::clamp(pos2, std::numeric_limits<double>::epsilon(),
                              1.f - std::numeric_limits<double>::epsilon());
            pos3 = std::clamp(pos3, 0.0, 1.0);

            primitives[i * 3]->setPosition(pos1);
            primitives[i * 3 + 1]->setPosition(pos2);
            primitives[i * 3 + 2]->setPosition(pos3);
        }
    };

    Property::OnChangeBlocker tfblock(isotfComposite_.tf_);
    resizeLabelControlPoints(isotfComposite_.tf_.get(), regionIndices->getSize());

    auto resizeLabelPickingControlPoints = [&](TransferFunction &tf, size_t nSegments) {
        while (tf.size() > 2 * (nSegments + 1)) {
            tf.remove(tf.back());
        }
        while (tf.size() < 2 * (nSegments + 1)) {
            tf.add(1.0, vec4{1.0});
        }
        pickingToLabelId_.clear();
        std::vector<TFPrimitive *> primitives;
        for (auto &p : tf) {
            primitives.push_back(&p);
        }
        vec2 dataRange = atlasVolume->dataMap_.dataRange;
        auto delta_ = 1.0 / (dataRange.y - dataRange.x);
        for (size_t i = 0; i < regionIndices->getSize(); i++) {
            auto labelId = static_cast<int>(regionIndices->getAsDouble(i));
            auto normalizedVal = atlas_->getLabelIdNormalized(labelId);
            pickingToLabelId_.push_back(labelId);
            auto color = atlasPicking_.getColor(i);

            double pos1(normalizedVal - delta_ / 2.0);
            // Ensure no overlap with next point
            double pos2(normalizedVal + delta_ / 2.0 - std::numeric_limits<double>::epsilon());

            // clamp the transfer function between 0 and 1
            pos1 = std::clamp(pos1, 0.0, 1.f - std::numeric_limits<double>::epsilon());
            pos2 = std::clamp(pos2, 0.0 + 2.0 * std::numeric_limits<double>::epsilon(), 1.0);

            primitives[i * 2 + 0]->setData({pos1, vec4(color, 1.f)});
            primitives[i * 2 + 1]->setData({pos2, vec4(color, 1.f)});
        }
    };
    resizeLabelPickingControlPoints(*pickingTF_, regionIndices->getSize());
}

void VolumeAtlasProcessor::handlePicking(PickingEvent *e) {
    auto id = pickingToLabelId(e->getPickedId());

    if (e->getHoverState() == PickingHoverState::Enter) {
        e->setToolTip(fmt::format("Label {}", id));
        hoverAtlasId_ = id;
        auto area = atlas_->getLabelName(hoverAtlasId_);
        hoverName_.set(area);
    } else if (e->getHoverState() == PickingHoverState::Exit) {
        e->setToolTip("");
        hoverAtlasId_ = -1;
        hoverName_.set("");
    }

    if (e->getPressState() == PickingPressState::Release &&
        e->getPressItem() == PickingPressItem::Primary &&
        glm::distance(e->getPressedPosition(), e->getPosition()) <
            2.f / glm::min(e->getCanvasSize().x, e->getCanvasSize().y)) {
        if (!e->modifiers().contains(KeyModifier::Control)) {
            // Deselect all others
            for (auto region : selectedVolumeRegions_) {
                auto r = dynamic_cast<BoolProperty *>(region);
                int labelId = static_cast<int>((r->getMetaData<IntMetaData>("labelId"))->get());
                if (labelId != id) {
                    r->set(false);
                }
            }
        }
        for (auto region : selectedVolumeRegions_) {
            auto r = dynamic_cast<BoolProperty *>(region);
            int labelId = static_cast<int>((r->getMetaData<IntMetaData>("labelId"))->get());
            if (labelId == id) {
                r->set(!r->get());
            }
        }
        e->markAsUsed();
    } else if (e->getPressState() == PickingPressState::Release &&
               e->getPressItem() == PickingPressItem::Primary &&
               e->modifiers().contains(KeyModifier::Shift)) {

        auto filtered = brushingAndLinking_.getFilteredIndices();

        if (brushingAndLinking_.isFiltered(id - 1)) {
            filtered.erase(id - 1);
        } else {
            filtered.insert(id - 1);
        }
        brushingAndLinking_.sendFilterEvent(filtered);

        e->markAsUsed();
    }
}

int VolumeAtlasProcessor::pickingToLabelId(int pickingID) const {
    if (pickingID >= 0) {
        return pickingToLabelId_[pickingID];
    } else {
        return -1;
    }
}

dvec3 inviwo::VolumeAtlasProcessor::getLabelCenterPoint(int labelId) const {
    if (atlasRegionCenterpoints_.isReady()) {
        auto regionPositions = atlasRegionCenterpoints_.getData();
        auto labelIdCol = regionPositions->getColumn(1);
        auto row = labelIdCol->getBuffer()->getRepresentation<BufferRAM>()->dispatch<int>(
            [labelId = labelId](auto typedBuf) {
                using ValueType = util::PrecisionValueType<decltype(typedBuf)>;
                const auto &data = typedBuf->getDataContainer();
                auto found = std::find(data.begin(), data.end(), ValueType(labelId));
                return std::distance(data.begin(), found);
            });
        auto posCol = regionPositions->getColumn(2);
        if (row >= 0 && row < posCol->getSize()) {
            return posCol->getAsDVec3(row);
        }
    }

    return dvec3(std::numeric_limits<double>::infinity());
}

int inviwo::VolumeAtlasProcessor::labelIdToRow(int labelID) const {
    auto labels = atlasLabels_.getData();
    return labels->getColumn(1)->getBuffer()->getRepresentation<BufferRAM>()->dispatch<int>(
        [labelID = labelID](auto typedBuf) {
            using ValueType = util::PrecisionValueType<decltype(typedBuf)>;

            const auto &left = typedBuf->getDataContainer();
            auto found = std::find(left.begin(), left.end(), ValueType(labelID));
            return std::distance(left.begin(), found);
        });
}

}  // namespace inviwo
