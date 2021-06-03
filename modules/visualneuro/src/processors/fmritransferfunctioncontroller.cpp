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

#include <modules/visualneuro/processors/fmritransferfunctioncontroller.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/colorbrewer.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo fMRITransferFunctionController::processorInfo_{
    "org.inviwo.fmritransferfunctioncontroller",  // Class identifier
    "fMRI Transfer Function Controller",          // Display name
    "Transfer Function",                          // Category
    CodeState::Experimental,                      // Code state
    Tags::None,                                   // Tags
};
const ProcessorInfo fMRITransferFunctionController::getProcessorInfo() const {
    return processorInfo_;
}

fMRITransferFunctionController::fMRITransferFunctionController()
    : Processor()
    , meanVolume_("volume")
    , correlationVolume_("correlationVolume")
    , tTestVolume_("tTest")
    , activeVolume_("activeVolume")
    , inputDataTFtype_("inputDataTFType", "fMRI data type",
                       {{"linear", "Low to high", static_cast<int>(TFtype::Linear)},
                        {"diverging", "Diverging around zero (-0+)",
                         static_cast<int>(TFtype::Symmetric)}},
                       0)
    , selectedTFtype_(
          "selectedfMRItype", "Active Volume",
          {{"mean", "Mean", static_cast<int>(ActiveInput::Mean)},
           {"ttest", "Group t-Test", static_cast<int>(ActiveInput::tTest)},
           {"paramVolume", "Parameter-Brain", static_cast<int>(ActiveInput::Correlation)}},
          0)
    , linearColormapSelector_("linearColormap", "Colormap", ColormapType::Continuous,
                              colorbrewer::Family::YlOrRd, 8)
    , linearTFProperties_("linearTFProperties", "Linear TF properties")
    , symmetricTFProperties_("symmetricTFProperties", "Symmetric TF properties")
    , correlationTFProperties_("correlationTFProperties", "Correlation TF Properties")
    , tTestTFProperties_("tTestTFProperties", "t-Test TF Properties")
    , selectedTF2D_("selectedTF2D", "Selected TF (2D)", TransferFunction{})
    , selectedTF3D_("selectedTF3D", "Selected TF (3D)", TransferFunction{})
    , parameterSelected_("parameterSelected", "A parameter is selected", false)
    , linearTF2D_("linearTF2D", "Transfer Function (Slices)",
                  linearColormapSelector_.getTransferFunction(), &meanVolume_,
                  InvalidationLevel::Valid)
    , linearTF3D_("linearTF3D", "Transfer Function (Volume)",
                  linearColormapSelector_.getTransferFunction(), &meanVolume_,
                  InvalidationLevel::Valid)
    , symmetricTF2D_("symmetricTF2D", "Transfer Function (Slices)", &meanVolume_,
                     InvalidationLevel::Valid)
    , symmetricTF3D_("symmetricTF3D", "Transfer Function (Volume)", &meanVolume_,
                     InvalidationLevel::Valid)
    , correlationTF2D_("correlationTF2D", "Transfer Function (Slices)", &correlationVolume_,
                       InvalidationLevel::Valid)
    , correlationTF3D_("correlationTF3D", "Transfer Function (Volume)", &correlationVolume_,
                       InvalidationLevel::Valid)
    , tTestTF2D_("tTestTF2D", "Transfer Function (Slices)",
                 linearColormapSelector_.getTransferFunction(), &tTestVolume_,
                 InvalidationLevel::Valid)
    , tTestTF3D_("tTestTF3D", "Transfer Function (Volume)",
                 linearColormapSelector_.getTransferFunction(), &tTestVolume_,
                 InvalidationLevel::Valid)
    , leftColor_("leftColor", "Left Color", vec4(0, 0, 1, 1))
    , rightColor_("rightColor", "Right Color", vec4(1, 0, 0, 1))
    , opacityActiveTF_("opacitySelectedTF", "Opacity", 1.0, 0.0, 1.0)
    , thresholdActiveTF_("thresholdActiveTF", "Threshold", 1.0, 0.0, 1.0)
    , opacityLinearTF_("opacityLinearTF", "Opacity", 1.0, 0.0, 1.0, 0.01, InvalidationLevel::Valid)
    , opacitySymmetricTF_("opacitySymmetricTF", "Opacity", 1.0, 0.0, 1.0, 0.01,
                          InvalidationLevel::Valid)
    , opacityCorrelationTF_("opacitySymmetricTF", "Opacity", 1.0, 0.0, 1.0, 0.01,
                            InvalidationLevel::Valid)
    , opacitytTestTF_("opacityLinearTF", "Opacity", 1.0, 0.0, 1.0, 0.01, InvalidationLevel::Valid)
    , thresholdLinearTF_("thresholdLinearTF", "Threshold", 0.0, 0.0, 1.0, 0.01,
                         InvalidationLevel::Valid)
    , thresholdSymmetricTF_("thresholdSymmetricTF", "Threshold +-", 0.5, 0.0, 1.0, 0.01,
                            InvalidationLevel::Valid)
    , thresholdCorrelationTF_("thresholdCorrelationTF", "Threshold +-", 0.4, 0.0, 1.0, 0.01,
                              InvalidationLevel::Valid)
    , thresholdtTestTF_("thresholdCorrelationTF", "Threshold", 0.005, 0.0, 1.0, 0.005,
                        InvalidationLevel::Valid) {

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });

    addPort(meanVolume_);
    addPort(correlationVolume_);
    addPort(tTestVolume_);
    addPort(activeVolume_);
    auto inportReady = [](const Inport &inport) {
        return (inport.isConnected() && util::all_of(inport.getConnectedOutports(),
                                                     [](Outport *p) { return p->isReady(); }));
    };
    meanVolume_.setIsReadyUpdater([this, inportReady]() {
        return getActiveInput() == ActiveInput::Mean ? inportReady(meanVolume_) : true;
    });
    correlationVolume_.setIsReadyUpdater([this, inportReady]() {
        return getActiveInput() == ActiveInput::Correlation ? inportReady(correlationVolume_)
                                                            : true;
    });
    tTestVolume_.setIsReadyUpdater([this, inportReady]() {
        return getActiveInput() == ActiveInput::tTest ? inportReady(tTestVolume_) : true;
    });

    addProperty(inputDataTFtype_);
    addProperty(selectedTFtype_);

    selectedTF2D_.setReadOnly(true);
    selectedTF2D_.setCurrentStateAsDefault();
    addProperty(selectedTF2D_);
    selectedTF3D_.setReadOnly(true);
    selectedTF3D_.setCurrentStateAsDefault();
    addProperty(selectedTF3D_);
    addProperty(linearColormapSelector_);

    linearColormapSelector_.onChange([&]() {
        auto activeTF = getActiveInput();
        if (activeTF == ActiveInput::Mean) {
            if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
                *linearTF2D_ = linearColormapSelector_.getTransferFunction();
                *linearTF3D_ = linearColormapSelector_.getTransferFunction();
            } else {
            }
        } else if (activeTF == ActiveInput::Correlation) {
            *correlationTF2D_ = linearColormapSelector_.getTransferFunction();
            *correlationTF3D_ = linearColormapSelector_.getTransferFunction();
        } else if (activeTF == ActiveInput::tTest) {
            *tTestTF2D_ = linearColormapSelector_.getTransferFunction();
            *tTestTF3D_ = linearColormapSelector_.getTransferFunction();
        }
    });

    addProperty(thresholdActiveTF_);
    addProperty(opacityActiveTF_);
    thresholdActiveTF_.onChange([&]() {
        switch (getActiveInput()) {
            case ActiveInput::Mean:
                if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
                    thresholdLinearTF_.set(thresholdActiveTF_.get());
                } else {
                    thresholdSymmetricTF_.set(thresholdActiveTF_.get());
                }
                break;
            case ActiveInput::Correlation:
                thresholdCorrelationTF_.set(thresholdActiveTF_.get());
                break;
            case ActiveInput::tTest:
                thresholdtTestTF_.set(thresholdActiveTF_.get());
                break;
        }
    });
    opacityActiveTF_.onChange([&]() {
        switch (getActiveInput()) {
            case ActiveInput::Mean:
                if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
                    opacityLinearTF_.set(opacityActiveTF_.get());
                } else {
                    opacitySymmetricTF_.set(opacityActiveTF_.get());
                }
                break;
            case ActiveInput::Correlation:
                opacityCorrelationTF_.set(opacityActiveTF_.get());
                break;
            case ActiveInput::tTest:
                opacitytTestTF_.set(opacityActiveTF_.get());
                break;
        }
    });

    selectedTFtype_.onChange([this]() {
        NetworkLock lock(this);
        switch (getActiveInput()) {
            case ActiveInput::Mean:
                if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
                    thresholdActiveTF_.set(&thresholdLinearTF_);
                    opacityActiveTF_.set(&opacityLinearTF_);
                    linearColormapSelector_.setVisible(true);
                } else {
                    thresholdActiveTF_.set(&thresholdSymmetricTF_);
                    opacityActiveTF_.set(&opacitySymmetricTF_);
                    linearColormapSelector_.setVisible(false);
                }
                break;
            case ActiveInput::Correlation:
                thresholdActiveTF_.set(&thresholdCorrelationTF_);
                opacityActiveTF_.set(&opacityCorrelationTF_);
                linearColormapSelector_.setVisible(false);
                break;
            case ActiveInput::tTest:
                thresholdActiveTF_.set(&thresholdtTestTF_);
                opacityActiveTF_.set(&opacitytTestTF_);
                linearColormapSelector_.setVisible(true);
                break;
        }
        meanVolume_.readyUpdate();
        correlationVolume_.readyUpdate();
        tTestVolume_.readyUpdate();
        this->notifyObserversActiveConnectionsChange(this);
    });

    addProperty(parameterSelected_);
    parameterSelected_.setReadOnly(true);
    parameterSelected_.onChange([this]() {
        if (*parameterSelected_) {
            selectedTFtype_.setCurrentStateAsDefault();
            selectedTFtype_.set(static_cast<int>(ActiveInput::Correlation));
        } else {
            selectedTFtype_.resetToDefaultState();
        }
    });

    meanVolume_.onChange([this]() {
        if (meanVolume_.isConnected()) {
            updateMeanVolumeSliders();
        }
    });
    meanVolume_.onDisconnect([this]() {
        thresholdLinearTF_.setMinValue(0.0);
        thresholdLinearTF_.setMaxValue(1.0);
        thresholdSymmetricTF_.setMaxValue(1.0);
    });
    meanVolume_.onConnect([this]() { updateMeanVolumeSliders(true); });

    tTestVolume_.onChange([this]() {
        if (tTestVolume_.hasData()) {
            NetworkLock lock(this);
            auto dataMap = tTestVolume_.getData()->dataMap_;
            double maxValueRange =
                std::max(glm::abs(dataMap.valueRange).x, glm::abs(dataMap.valueRange).y);
            thresholdtTestTF_.setMaxValue(maxValueRange);
            if (dataMap.valueRange.x >= 0) {
                *tTestTF2D_ = linearColormapSelector_.getTransferFunction();
                *tTestTF3D_ = linearColormapSelector_.getTransferFunction();
            }
            if (getActiveInput() == ActiveInput::tTest) {
                thresholdActiveTF_.set(&thresholdtTestTF_);
                opacityActiveTF_.set(&opacitytTestTF_);
            }
        }
    });

    linearTFProperties_.addProperty(opacityLinearTF_);
    linearTFProperties_.addProperty(thresholdLinearTF_);
    linearTFProperties_.addProperty(linearTF2D_);
    linearTFProperties_.addProperty(linearTF3D_);
    linearTFProperties_.setVisible(false);

    symmetricTFProperties_.addProperty(opacitySymmetricTF_);
    symmetricTFProperties_.addProperty(thresholdSymmetricTF_);
    symmetricTFProperties_.addProperty(leftColor_);
    symmetricTFProperties_.addProperty(rightColor_);
    symmetricTFProperties_.addProperty(symmetricTF2D_);
    symmetricTFProperties_.addProperty(symmetricTF3D_);
    symmetricTFProperties_.setVisible(false);

    correlationTFProperties_.addProperty(opacityCorrelationTF_);
    correlationTFProperties_.addProperty(thresholdCorrelationTF_);
    correlationTFProperties_.addProperty(correlationTF2D_);
    correlationTFProperties_.addProperty(correlationTF3D_);
    correlationTFProperties_.setVisible(false);

    tTestTFProperties_.addProperty(opacitytTestTF_);
    tTestTFProperties_.addProperty(thresholdtTestTF_);
    tTestTFProperties_.addProperty(tTestTF2D_);
    tTestTFProperties_.addProperty(tTestTF3D_);
    tTestTFProperties_.setVisible(false);

    leftColor_.setSemantics(PropertySemantics::Color);
    rightColor_.setSemantics(PropertySemantics::Color);

    addProperty(linearTFProperties_);
    addProperty(symmetricTFProperties_);
    addProperty(correlationTFProperties_);
    addProperty(tTestTFProperties_);
}

bool fMRITransferFunctionController::isConnectionActive(Inport *from, Outport *to) const {
    switch (getActiveInput()) {
        case ActiveInput::Mean:
            return from == &meanVolume_ && to == meanVolume_.getConnectedOutport();
            break;
        case ActiveInput::Correlation:
            return from == &correlationVolume_ && to == correlationVolume_.getConnectedOutport();
            break;
        case ActiveInput::tTest:
            return from == &tTestVolume_ && to == tTestVolume_.getConnectedOutport();
            break;
        default:
            return true;
    }
}
void fMRITransferFunctionController::process() {

    auto activeTF = getActiveInput();

    updateTFs(activeTF);
    if (activeTF == ActiveInput::Mean) {
        if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
            selectedTF2D_.set(linearTF2D_.get());
            selectedTF3D_.set(linearTF3D_.get());
        } else {
            selectedTF2D_.set(symmetricTF2D_.get());
            selectedTF3D_.set(symmetricTF3D_.get());
        }
        activeVolume_.setData(meanVolume_.getData());
    } else if (activeTF == ActiveInput::Correlation) {
        selectedTF2D_.set(correlationTF2D_.get());
        selectedTF3D_.set(correlationTF3D_.get());
        activeVolume_.setData(correlationVolume_.getData());
    } else if (activeTF == ActiveInput::tTest) {
        selectedTF2D_.set(tTestTF2D_.get());
        selectedTF3D_.set(tTestTF3D_.get());
        activeVolume_.setData(tTestVolume_.getData());
    }
}

fMRITransferFunctionController::ActiveInput fMRITransferFunctionController::getActiveInput() const {
    return ActiveInput(*selectedTFtype_);
}

void fMRITransferFunctionController::updateTFs(ActiveInput input) {
    if (input == ActiveInput::Mean && meanVolume_.hasData()) {
        if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
            updateLinearTF(linearTF2D_.get(), true, thresholdLinearTF_.get(),
                           opacityLinearTF_.get(), meanVolume_.getData()->dataMap_);
            updateLinearTF(linearTF3D_.get(), false, thresholdLinearTF_.get(),
                           opacityLinearTF_.get(), meanVolume_.getData()->dataMap_);
        } else if (TFtype(*inputDataTFtype_) == TFtype::Symmetric && meanVolume_.hasData()) {
            updateSymmetricTF(symmetricTF2D_.get(), true, thresholdSymmetricTF_.get(),
                              opacitySymmetricTF_.get(), meanVolume_.getData()->dataMap_);
            updateSymmetricTF(symmetricTF3D_.get(), false, thresholdSymmetricTF_.get(),
                              opacitySymmetricTF_.get(), meanVolume_.getData()->dataMap_);
        }
    } else if (input == ActiveInput::Correlation && correlationVolume_.hasData()) {
        if (correlationVolume_.getData()->dataMap_.valueRange.x < 0) {
            updateSymmetricTF(correlationTF2D_.get(), true, thresholdCorrelationTF_.get(),
                              opacityCorrelationTF_.get(), correlationVolume_.getData()->dataMap_);
            updateSymmetricTF(correlationTF3D_.get(), false, thresholdCorrelationTF_.get(),
                              opacityCorrelationTF_.get(), correlationVolume_.getData()->dataMap_);
        } else {
            updateLinearTF(correlationTF2D_.get(), true, thresholdCorrelationTF_.get(),
                           opacityCorrelationTF_.get(), correlationVolume_.getData()->dataMap_);
            updateLinearTF(correlationTF3D_.get(), false, thresholdCorrelationTF_.get(),
                           opacityCorrelationTF_.get(), correlationVolume_.getData()->dataMap_);
        }

    } else if (input == ActiveInput::tTest && tTestVolume_.hasData()) {
        if (tTestVolume_.getData()->dataMap_.valueRange.x < 0) {
            // Colormap, reversed, PuOr_8
            auto leftColors = {vec3(0.32941176470588235, 0.15294117647058825, 0.5333333333333333),
                               vec3(0.5019607843137255, 0.45098039215686275, 0.6745098039215687),
                               vec3(0.6980392156862745, 0.6705882352941176, 0.8235294117647058),
                               vec3(0.8470588235294118, 0.8549019607843137, 0.9215686274509803)};
            auto rightColors = {
                vec3(0.996078431372549, 0.8784313725490196, 0.7137254901960784),
                vec3(0.9921568627450981, 0.7215686274509804, 0.38823529411764707),
                vec3(0.8784313725490196, 0.5098039215686274, 0.0784313725490196),
                vec3(0.7019607843137254, 0.34509803921568627, 0.023529411764705882)};

            updateSymmetricTF(tTestTF2D_.get(), true, thresholdtTestTF_.get(),
                              opacitytTestTF_.get(), tTestVolume_.getData()->dataMap_, leftColors,
                              rightColors);
            updateSymmetricTF(tTestTF3D_.get(), false, thresholdtTestTF_.get(),
                              opacitytTestTF_.get(), tTestVolume_.getData()->dataMap_, leftColors,
                              rightColors);
        } else {
            updateLinearTF(tTestTF2D_.get(), true, thresholdtTestTF_.get(), opacitytTestTF_.get(),
                           tTestVolume_.getData()->dataMap_);
            updateLinearTF(tTestTF3D_.get(), false, thresholdtTestTF_.get(), opacitytTestTF_.get(),
                           tTestVolume_.getData()->dataMap_);
        }
    }
}  // namespace inviwo

void fMRITransferFunctionController::updateLinearTF(TransferFunction &tf, const bool slicesTF,
                                                    double threshold, double opacity,
                                                    const DataMapper &dataMap) {

    double startPosition = threshold;
    double endPosition = 1.0;
    double selectedRange = endPosition - startPosition;
    double delta = selectedRange / (tf.size() - 1);

    startPosition = dataMap.mapFromValueToNormalized(startPosition);
    endPosition = dataMap.mapFromValueToNormalized(dataMap.valueRange.y);
    selectedRange = endPosition - startPosition;
    delta = selectedRange / (tf.size() - 1);

    // Create the TF
    size_t pointCount = 0;
    auto sortedPrimitives = linearColormapSelector_.getTransferFunction().get();
    tf.clear();
    for (auto primitive : sortedPrimitives) {
        double position = startPosition + pointCount * delta;
        if (position < 0.0 || position > 1.0) {
            LogWarn("Adding TFPrimitive at " << position << " outside of range [0,1]");
            position = glm::clamp(position, 0.0, 1.0);
        }
        double alpha = slicesTF ? static_cast<double>(std::min(pointCount, size_t(1))) * opacity 
                                : (static_cast<double>(pointCount) / (sortedPrimitives.size() - 1)) * opacity;
        vec4 color = vec4(vec3(primitive.color), alpha);
        tf.add(position, color);
        pointCount++;
    }
}

void fMRITransferFunctionController::updateSymmetricTF(TransferFunction &tf, const bool slicesTF,
                                                       double threshold, double opacity,
                                                       const DataMapper &dataMap,
                                                       std::vector<vec3> leftColors,
                                                       std::vector<vec3> rightColors) {
    double minStep = 1.0 / static_cast<double>(tf.getTextureSize() - 1);
    auto normalizedCenterPnt = dataMap.mapFromValueToNormalized(0.0);
    double normalizedThreshold =
        std::max(dataMap.mapFromValueToNormalized(threshold) - normalizedCenterPnt, 2.0 * minStep);

    vec4 centerColorLeft = vec4(leftColors.back(), 0.0);
    vec4 centerColorRight = vec4(rightColors.front(), 0.0);

    tf.clear();
    auto leftThreshold = std::max(normalizedCenterPnt - normalizedThreshold, 0.0);
    auto rightThreshold = std::min(normalizedCenterPnt + normalizedThreshold, 1.0);
    auto lDs = leftThreshold / (leftColors.size() - 1);
    for (auto &&[ind, c] : util::enumerate(leftColors)) {
        tf.add(ind * lDs, vec4(c, slicesTF ? opacity : opacity* (1.- static_cast<double>(ind) / (leftColors.size()-1))));
    }
    auto smoothing = 0.075;
    // Volume TF is created with two centerpoints in the same colors as the two edge points but
    // with opacity 0.
    tf.add(std::clamp(std::min(leftThreshold + smoothing, normalizedCenterPnt - minStep), 0.0, 1.0), centerColorLeft);
    tf.add(std::clamp(std::max(rightThreshold - smoothing, normalizedCenterPnt + minStep), 0.0, 1.0),
            centerColorRight);

    auto rDs = (1.0 - rightThreshold) / (rightColors.size() - 1);
    for (auto &&[ind, c] : util::enumerate(rightColors)) {
        auto v = std::clamp(rightThreshold + ind * rDs, 0.0, 1.0);
        tf.add(v, vec4(c, slicesTF ? opacity : opacity* ind / (rightColors.size()-1)));
    }
}

void fMRITransferFunctionController::updateMeanVolumeSliders(const bool updateSetValue) {
    if (meanVolume_.hasData()) {
        NetworkLock lock(this);
        auto dataMap = meanVolume_.getData()->dataMap_;
        thresholdLinearTF_.set(thresholdLinearTF_.get(), dataMap.valueRange.x, dataMap.valueRange.y,
                               abs(dataMap.valueRange.y - dataMap.valueRange.x) * 0.01);
        double maxValueRange =
            std::max(glm::abs(dataMap.valueRange).x, glm::abs(dataMap.valueRange).y);
        thresholdSymmetricTF_.setMaxValue(maxValueRange);
        if (updateSetValue) {
            thresholdLinearTF_.set(dataMap.mapFromNormalizedToValue(thresholdLinearTF_.get()));
            auto symmetricThreshold =
                (dataMap.mapFromNormalizedToValue(thresholdSymmetricTF_.get()) -
                 dataMap.valueRange.x) /
                2.0;
            thresholdSymmetricTF_.set(symmetricThreshold);
        }
        if (getActiveInput() == ActiveInput::Mean) {
            if (TFtype(*inputDataTFtype_) == TFtype::Linear) {
                thresholdActiveTF_.set(&thresholdLinearTF_);
                opacityActiveTF_.set(&opacityLinearTF_);
            } else {
                thresholdActiveTF_.set(&thresholdSymmetricTF_);
                opacityActiveTF_.set(&opacitySymmetricTF_);
            }
        }
    }
}

}  // namespace inviwo
