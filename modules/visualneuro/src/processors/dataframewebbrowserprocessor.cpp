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
    : Processor()
    // Output from CEF is 8-bits per channel
    , dataFramePort_("dataFrames")
    , background_("background")
    , outport_("webpage", DataVec4UInt8::get())
    , fileName_("fileName", "HTML file", "")
    , autoReloadFile_("autoReloadFile", "Auto Reload", true)
    , url_("URL", "URL", "http://www.inviwo.org")
    , reload_("reload", "Reload")
    , runJS_("runJS", "Run JS")
    , js_("js", "JavaScript", "", InvalidationLevel::Valid)
    , sourceType_("sourceType", "Source",
                  {{"localFile", "Local File", SourceType::LocalFile},
                   {"webAddress", "Web Address", SourceType::WebAddress}})
    , picking_(this, 1, [&](PickingEvent* p) { cefInteractionHandler_.handlePickingEvent(p); })
    , cefToInviwoImageConverter_(picking_.getColor())
    , renderHandler_{static_cast<RenderHandlerGL*>(
          app->getModuleByType<WebBrowserModule>()->getBrowserClient()->GetRenderHandler().get())} {
    dataFramePort_.setOptional(true);
    addPort(dataFramePort_);
    addPort(background_);
    background_.setOptional(true);
    addPort(outport_);

    addProperty(sourceType_);
    addProperty(fileName_);
    addProperty(autoReloadFile_);
    addProperty(url_);
    url_.setVisible(false);
    addProperty(reload_);

    addProperty(runJS_);
    addProperty(js_);

    auto updateVisibility = [this]() {
        fileName_.setVisible(sourceType_ == SourceType::LocalFile);
        autoReloadFile_.setVisible(sourceType_ == SourceType::LocalFile);
        url_.setVisible(sourceType_ == SourceType::WebAddress);
    };
    updateVisibility();

    auto reload = [this]() { browser_->GetMainFrame()->LoadURL(getSource()); };

    sourceType_.onChange([reload, updateVisibility]() {
        updateVisibility();
        reload();
    });
    fileName_.onChange([this, reload]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        }
        reload();
    });
    autoReloadFile_.onChange([this]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        } else {
            fileObserver_.stop();
        }
    });
    url_.onChange(reload);
    reload_.onChange(reload);

    fileObserver_.onChange([this, reload]() {
        if (sourceType_ == SourceType::LocalFile) {
            reload();
        }
    });

    // Setup CEF browser
    auto [windowInfo, browserSettings] = cefutil::getDefaultBrowserSettings();
    auto browserClient = app->getModuleByType<WebBrowserModule>()->getBrowserClient();
    // Note that browserClient_ outlives this class so make sure to remove
    // renderHandler_ in
    // destructor
    browser_ = CefBrowserHost::CreateBrowserSync(windowInfo, browserClient, getSource(),
                                                 browserSettings, nullptr, nullptr);
    browserClient->setBrowserParent(browser_, this);
    // Observe when page has loaded
    browserClient->addLoadHandler(this);
    // Do not process until frame is loaded
    isReady_.setUpdate([this]() { return allInportsAreReady() && !isBrowserLoading_; });
    // Inject events into CEF browser_
    cefInteractionHandler_.setHost(browser_->GetHost());
    cefInteractionHandler_.setRenderHandler(renderHandler_);
    addInteractionHandler(&cefInteractionHandler_);
}

std::string DataFrameWebBrowserProcessor::getSource() {
    std::string sourceString;
    if (sourceType_.get() == SourceType::LocalFile) {
        sourceString = "file://" + fileName_.get();
    } else if (sourceType_.get() == SourceType::WebAddress) {
        sourceString = url_.get();
    }
#ifndef NDEBUG
    // CEF does not allow empty urls in debug mode
    if (sourceString.empty()) {
        sourceString = "https://www.inviwo.org";
    }
#endif
    return sourceString;
}

DataFrameWebBrowserProcessor::~DataFrameWebBrowserProcessor() {
    static_cast<WebBrowserClient*>(browser_->GetHost()->GetClient().get())->removeLoadHandler(this);
    // Force close browser
    browser_->GetHost()->CloseBrowser(true);
}

void DataFrameWebBrowserProcessor::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    // Must reload page to connect property with Frame, see PropertyCefSynchronizer::OnLoadEnd
    browser_->GetMainFrame()->LoadURL(getSource());
}

void DataFrameWebBrowserProcessor::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                                        bool isLoading, bool /*canGoBack*/,
                                                        bool /*canGoForward*/) {
    if (browser_ && browser->GetIdentifier() == browser_->GetIdentifier()) {
        isBrowserLoading_ = isLoading;
        isReady_.update();
    }
}

void DataFrameWebBrowserProcessor::process() {
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
