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
const ProcessorInfo& DataFrameWebBrowserProcessor::getProcessorInfo() const {
    return processorInfo_;
}

DataFrameWebBrowserProcessor::DataFrameWebBrowserProcessor(InviwoApplication* app)
    : Processor()
    // Output from CEF is 8-bits per channel
    , dataFramePort_("dataFrames")
    , background_{"background"}
    , outport_{"webpage", DataVec4UInt8::get()}
    , sourceType_{"sourceType", "Source",
                  OptionPropertyState<SourceType>{
                      .options = {{"localFile", "Local File", SourceType::LocalFile},
                                  {"webAddress", "Web Address", SourceType::WebAddress}},
                      .invalidationLevel = InvalidationLevel::Valid,
                  }
                      .setSelectedValue(SourceType::WebAddress)}
    , fileName_{"fileName", "HTML file", {}, "html", InvalidationLevel::Valid}
    , autoReloadFile_{"autoReloadFile", "Auto Reload", true, InvalidationLevel::Valid}
    , url_{"URL", "URL", "https://www.inviwo.org", InvalidationLevel::Valid}
    , reload_{"reload", "Reload", InvalidationLevel::Valid}
    , zoom_{"zoom", "Zoom Factor", 1.0, 0.2, 5.0}
    , runJS_{"runJS", "Run JS"}
    , js_{"js", "JavaScript", "", InvalidationLevel::Valid}
    , browser_{new WebBrowserBase(app, *this, outport_, &background_)} {

    addPorts(background_, dataFramePort_, outport_);
    dataFramePort_.setOptional(true);

    background_.setOptional(true);
    addProperties(sourceType_, fileName_, autoReloadFile_, url_, reload_, zoom_, runJS_, js_);

    fileName_.visibilityDependsOn(sourceType_, [](auto& p) { return p == SourceType::LocalFile; });
    autoReloadFile_.visibilityDependsOn(sourceType_,
                                        [](auto& p) { return p == SourceType::LocalFile; });
    url_.visibilityDependsOn(sourceType_, [](auto& p) { return p == SourceType::WebAddress; });

    sourceType_.onChange([this]() { updateSource(); });
    fileName_.onChange([this]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        }
        updateSource();
    });
    autoReloadFile_.onChange([this]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        } else {
            fileObserver_.stop();
        }
    });

    url_.onChange([this]() { updateSource(); });
    reload_.onChange([this]() { updateSource(); });

    fileObserver_.onChange([this]() {
        if (sourceType_ == SourceType::LocalFile) {
            updateSource();
        }
    });

    addInteractionHandler(browser_->getInteractionHandler());
    updateSource();
}

void DataFrameWebBrowserProcessor::process() {
    reloaded_ |= fileName_.isModified() || url_.isModified() || reload_.isModified() ||
                    sourceType_.isModified();
    if (browser_->isLoading()) {
        return;
    }
    if (js_.isModified() && !js_.get().empty()) {
        browser_->executeJavaScript(js_.get(), 1);
    }
    if (reloaded_ || dataFramePort_.isChanged()) {
        auto changed = dataFramePort_.getChangedOutports();

        auto dataFrames = dataFramePort_.getSourceVectorData();
        for (const auto& elem : dataFrames) {
            if (reloaded_ || util::contains(changed, elem.first)) {
                auto dataFrame = elem.second;
                json root = *dataFrame;

                std::stringstream data("var data = ", std::ios_base::app | std::ios_base::out);
                data << root.dump() << ";";

                json port = {{"port", elem.first->getIdentifier()},
                             {"processor", elem.first->getProcessor()->getIdentifier()}};

                data << "onInviwoDataChanged(data," << port.dump() << ");";

                browser_->executeJavaScript(data.str(), 0);
            }
        }
    } 
    reloaded_ = false;
}

void DataFrameWebBrowserProcessor::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    // Must reload page to connect property with Frame, see PropertyCefSynchronizer::OnLoadEnd
    updateSource();
}

void DataFrameWebBrowserProcessor::updateSource() {
    switch (sourceType_) {
        case SourceType::LocalFile:
            browser_->load(fileName_);
            break;
        case SourceType::WebAddress:
            browser_->load(url_);
            break;
        default:
            browser_->load(std::string_view{"https://www.inviwo.org"});
            break;
    }
}

}  // namespace inviwo
