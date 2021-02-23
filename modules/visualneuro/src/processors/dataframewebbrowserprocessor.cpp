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

#include <modules/visualneuro/processors/dataframewebbrowserprocessor.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/webbrowsermodule.h>
#include <modules/webbrowser/webbrowserutil.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/utilities.h>

#include <inviwo/dataframe/jsondataframeconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <nlohmann/json.hpp>
#include <include/cef_app.h>
#include <warn/pop>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameWebBrowserProcessor::processorInfo_{
    "org.inviwo.dataframewebbrowser",  // Class identifier
    "Web browser Data Frame",          // Display name
    "Web",                             // Category
    CodeState::Stable,                 // Code state
    "GL, Web Browser, DataFrame",      // Tags
};
const ProcessorInfo DataFrameWebBrowserProcessor::getProcessorInfo() const {
    return processorInfo_;
}

DataFrameWebBrowserProcessor::DataFrameWebBrowserProcessor(InviwoApplication* app)
    : WebBrowserProcessor(app)
    // Output from CEF is 8-bits per channel
    , dataFramePort_("dataFrames") {
    addPort(dataFramePort_);
}

void DataFrameWebBrowserProcessor::process() {
    if (isLoading_) {
        return;
    }
    if (js_.isModified() && !js_.get().empty()) {
        browser_->GetMainFrame()->ExecuteJavaScript(js_.get(), "", 1);
    }
    if (fileName_.isModified() || url_.isModified() || reload_.isModified() ||
        sourceType_.isModified() || dataFramePort_.isChanged()) {
        using json = nlohmann::json;

        auto changed = dataFramePort_.getChangedOutports();

        auto dataFrames = dataFramePort_.getSourceVectorData();
        for (const auto& elem : dataFrames) {
            if (util::contains(changed, elem.first)) {
                auto dataFrame = elem.second;
                json root = *dataFrame;

                std::stringstream data("var data = ", std::ios_base::app | std::ios_base::out);
                data << root.dump() << ";";

                auto frame = browser_->GetMainFrame();
                json port = {{"port", elem.first->getIdentifier()},
                             {"processor", elem.first->getProcessor()->getIdentifier()}};

                data << "onInviwoDataChanged(data," << port.dump() << ");";

                frame->ExecuteJavaScript(data.str(), frame->GetURL(), 0);
            }
        }
    }
    // Vertical flip of CEF output image
    cefToInviwoImageConverter_.convert(renderHandler_->getTexture2D(browser_), outport_,
                                       &background_);
}

}  // namespace inviwo
