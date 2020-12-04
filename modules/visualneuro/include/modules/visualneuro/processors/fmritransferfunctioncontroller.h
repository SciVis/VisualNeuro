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
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/dataframe/properties/colormapproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {


/** \docpage{org.inviwo.fmritransferfunctioncontroller, fMRI Transfer Function Controller}
* ![](org.inviwo.fmritransferfunctioncontroller.png?classIdentifier=org.inviwo.fmritransferfunctioncontroller)
* 
 * This processor controls the data flow in the network depending on currently selected mode.
 * It also provides simplified controls for manipulating Transfer functions.
 * Depending on which type of data is selected different transfer functions and controls will appear
 * as properties in the processor.
* 
* 
* ### Inports
*   * __volume__ Group average.
*   * __correlationVolume__ Correlation between groups.
*   * __tTest__ t-Test between groups.
* 
* ### Outports
*   * __activeVolume__ Volume depending on selected method.
* 
* ### Properties
*   * __fMRI data type__ Properties of the numerical data, linear/diverging around zero
*   * __Active Volume__ Input data selector
*   * __Selected TF (2D)__ Color map for 2D slices of selected volume.
*   * __Selected TF (3D)__ Color map for selected volume.
*   * __Colormap__ Type of color map - Continous/Categorical.
*   * __Threshold__ Threshold for showing data.
*   * __Opacity__ Opacity (visibility) of transfer function.
* 
*/
class IVW_MODULE_VISUALNEURO_API fMRITransferFunctionController : public Processor {
public:
    fMRITransferFunctionController();
    virtual ~fMRITransferFunctionController() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual bool isConnectionActive(Inport*, Outport*) const override;

private:
    enum class TFtype { Linear, Symmetric };
    enum class ActiveInput { Mean, tTest, Correlation };

    ActiveInput getActiveInput() const;

    VolumeInport meanVolume_;
    VolumeInport correlationVolume_;  // Computed group correlation
    VolumeInport tTestVolume_;        // Computed t-Test
    VolumeOutport activeVolume_;      // Active volume

    OptionPropertyInt inputDataTFtype_;  // Blood flow, PCA, Functional connectivity
    OptionPropertyInt selectedTFtype_;   // Depends on user interaction, e.g. correlation
    ColormapProperty linearColormapSelector_;

    CompositeProperty linearTFProperties_;
    CompositeProperty symmetricTFProperties_;
    CompositeProperty correlationTFProperties_;
    CompositeProperty tTestTFProperties_;

    TransferFunctionProperty selectedTF2D_;
    TransferFunctionProperty selectedTF3D_;
    BoolProperty parameterSelected_;
    std::shared_ptr<std::function<void()>> activeTF2DChanged_;
    std::shared_ptr<std::function<void()>> activeTF3DChanged_;
    TransferFunctionProperty linearTF2D_;
    TransferFunctionProperty linearTF3D_;
    TransferFunctionProperty symmetricTF2D_;
    TransferFunctionProperty symmetricTF3D_;
    TransferFunctionProperty correlationTF2D_;  // Pearson correlation
    TransferFunctionProperty correlationTF3D_;
    TransferFunctionProperty tTestTF2D_;  // t-Test p value
    TransferFunctionProperty tTestTF3D_;

    FloatVec4Property leftColor_;
    FloatVec4Property rightColor_;

    DoubleProperty opacityActiveTF_;
    DoubleProperty thresholdActiveTF_;

    DoubleProperty opacityLinearTF_;
    DoubleProperty opacitySymmetricTF_;
    DoubleProperty opacityCorrelationTF_;
    DoubleProperty opacitytTestTF_;
    DoubleProperty thresholdLinearTF_;
    DoubleProperty thresholdSymmetricTF_;
    DoubleProperty thresholdCorrelationTF_;
    DoubleProperty thresholdtTestTF_;

    void updateTFs(ActiveInput type);
    void updateLinearTF(TransferFunction& tf, const bool slicesTF, double threshold, double opacity,
                        const DataMapper& dataMap);

    // Default colormap, reversed, RdBu_8
    void updateSymmetricTF(TransferFunction& tf, const bool slicesTF, double threshold,
                           double opacity, const DataMapper& dataMap,
                           std::vector<vec3> leftColors =
                               {vec3(0.12941176470588237, 0.4, 0.6745098039215687),
                                vec3(0.2627450980392157, 0.5764705882352941, 0.7647058823529411),
                                vec3(0.5725490196078431, 0.7725490196078432, 0.8705882352941177)},
                           std::vector<vec3> rightColors = {
                               vec3(0.9568627450980393, 0.6470588235294118, 0.5098039215686274),
                               vec3(0.8392156862745098, 0.3764705882352941, 0.30196078431372547),
                               vec3(0.6980392156862745, 0.09411764705882353, 0.16862745098039217)});
    void updateMeanVolumeSliders(const bool updateSetValue = false);
};

}  // namespace inviwo

