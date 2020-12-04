/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/visualneuro/processors/brainraycaster.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/algorithm/boundingbox.h>


namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BrainRayCaster::processorInfo_{
    "org.inviwo.BrainRayCaster",    // Class identifier
    "Brain Ray Caster",             // Display name
    "Volume Rendering",             // Category
    CodeState::Stable,              // Code state
    "GL, DVR, Raycasting",          // Tags
};
const ProcessorInfo BrainRayCaster::getProcessorInfo() const { return processorInfo_; }

BrainRayCaster::BrainRayCaster()
    : PoolProcessor()
    , shader_("brainraycaster.frag", false)
    , volumePort_("volume")
    , activityPort_("activity")
    , entryPort_("entry")
    , exitPort_("exit")
    , atlasPort_("atlas")
    , atlasTransferFunction_("atlasTransferFunction")
    , backgroundPort_("bg")
    , outport_("outport")
    , channel_("channel", "Render Channel", {{"Channel 1", "Channel 1", 0}}, 0)
    , raycasting_("raycaster", "Raycasting")
    , enableAtlas_("enableAtlas", "Enable Atlas", true, InvalidationLevel::InvalidResources)
    , isotfComposite_("isotfComposite", "TF & Isovalues", &volumePort_,
                      InvalidationLevel::InvalidResources)
    , activityTransferFunction_("activityTransferFunction", "Activity transfer function",
                                TransferFunction())
    , camera_("camera", "Camera", util::boundingBox(volumePort_))
    , lighting_("lighting", "Lighting", &camera_)
    , positionIndicator_("positionindicator", "Position Indicator") {

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });

    addPort(volumePort_, "VolumePortGroup");
    addPort(activityPort_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(backgroundPort_, "ImagePortGroup1");
    addPort(atlasPort_);
    addPort(atlasTransferFunction_);
    addPort(outport_, "ImagePortGroup1");


    backgroundPort_.setOptional(true);

    channel_.setSerializationMode(PropertySerializationMode::All);

    volumePort_.onChange([this]() {
        if (volumePort_.hasData()) {
            size_t channels = volumePort_.getData()->getDataFormat()->getComponents();

            if (channels == channel_.size()) return;

            std::vector<OptionPropertyIntOption> channelOptions;
            for (size_t i = 0; i < channels; i++) {
                channelOptions.emplace_back("Channel " + toString(i + 1),
                                            "Channel " + toString(i + 1), static_cast<int>(i));
            }
            channel_.replaceOptions(channelOptions);
            channel_.setCurrentStateAsDefault();
        }
    });
    backgroundPort_.onConnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });
    backgroundPort_.onDisconnect([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    // change the currently selected channel when a pre-computed gradient is selected
    raycasting_.gradientComputation_.onChange([this]() {
        if (channel_.size() == 4) {
            if (raycasting_.gradientComputation_.get() ==
                RaycastingProperty::GradientComputation::PrecomputedXYZ) {
                channel_.set(3);
            } else if (raycasting_.gradientComputation_.get() ==
                       RaycastingProperty::GradientComputation::PrecomputedYZW) {
                channel_.set(0);
            }
        }
    });

    addProperty(channel_);
    addProperty(raycasting_);
    addProperty(isotfComposite_);

    addProperty(enableAtlas_);

    addProperty(camera_);
    addProperty(lighting_);
    addProperty(positionIndicator_);
    addProperty(activityTransferFunction_);
}

void BrainRayCaster::initializeResources() {
    utilgl::addDefines(shader_, raycasting_, isotfComposite_, camera_, lighting_,
                       positionIndicator_);
    utilgl::addShaderDefinesBGPort(shader_, backgroundPort_);
    if (enableAtlas_)
        shader_[ShaderType::Fragment]->addShaderDefine("HAS_ATLAS");
    else
        shader_[ShaderType::Fragment]->removeShaderDefine("HAS_ATLAS");

    shader_.build();
}

void BrainRayCaster::process() {
    if (volumePort_.isChanged() || activityPort_.isChanged() || atlasPort_.isChanged()) {
        dispatchOne(
            [volume = volumePort_.getData(), activity = activityPort_.getData(), 
             atlas = atlasPort_.getData()]() 
            -> std::array<std::shared_ptr<const Volume>, 3> {
                volume->getRep<kind::GL>();
                activity->getRep<kind::GL>();
                atlas->getRep<kind::GL>();
                glFinish();
                return {volume, activity, atlas};
            },
            [this](std::array<std::shared_ptr<const Volume>, 3> volumes) {
                raycast(*volumes[0],*volumes[1], *volumes[2]);
                newResults();
            });
    } else {
        raycast(*volumePort_.getData(), *activityPort_.getData(), *atlasPort_.getData());
    }
}

void BrainRayCaster::raycast(const Volume& volume, const Volume& activity, const Volume& atlas) {
    if (!volume.getRep<kind::GL>()) {
        throw Exception("Could not find VolumeGL representation", IVW_CONTEXT);
    }
    utilgl::activateAndClearTarget(outport_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, volume, "volume");
    utilgl::bindAndSetUniforms(shader_, units, activity, "activity");
    utilgl::bindAndSetUniforms(shader_, units, atlas, "atlas");
    utilgl::bindAndSetUniforms(shader_, units, isotfComposite_);
    utilgl::bindAndSetUniforms(shader_, units, activityTransferFunction_);
    utilgl::bindAndSetUniforms(shader_, units, 
                               *(atlasTransferFunction_.getData()->getData()->getRepresentation<LayerGL>()->getTexture()),
                               "atlasTransferFunction");

    utilgl::bindAndSetUniforms(shader_, units, entryPort_, ImageType::ColorDepthPicking);
    utilgl::bindAndSetUniforms(shader_, units, exitPort_, ImageType::ColorDepth);
    if (backgroundPort_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, units, backgroundPort_, ImageType::ColorDepthPicking);
    }
    utilgl::setUniforms(shader_, outport_, camera_, lighting_, raycasting_, positionIndicator_,
                        channel_, isotfComposite_);

    utilgl::singleDrawImagePlaneRect();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
