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


#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/cefimageconverter.h>
#include <modules/webbrowser/renderhandlergl.h>

#include <inviwo/dataframe/datastructures/dataframe.h>

#include <modules/webbrowser/processors/webbrowserprocessor.h>

namespace inviwo {

/** \docpage{org.inviwo.DataFrameWebBrowserProcessor, Chromium Processor}
 * ![](org.inviwo.DataFrameWebBrowser.png?classIdentifier=org.inviwo.DataFrameWebBrowser)
 * Display webpage, including transparency, on top of optional background and enable synchronization
 * of properties.
 * Takes DataFrames as input and sends content to webpage as soon as it changes.
 * calls onInviwoDataChanged(data, inportIdentifier)
 *
 * Synchronization from Invwo to web page requires its html element id, i.e.
 * \code{.html}
 * <input type="text" id="stringProperty">.
 * \endcode
 * Synchronization from web page to Inviwo requires that you add javascript
 * code. Added properties can be linked. Their display name might change but it will not affect
 * their identifier. Example of code to add to HTML-page:
 * \code{.js}
 * <script language="JavaScript">
 * function onTextInput(val) {
 *     window.cefQuery({
 *        request: '<Properties><Property type="org.inviwo.stringProperty"
 *             identifier="PropertySyncExample.stringProperty"><value content="' + val + '"
 *              </Property></Properties>',
 *        onSuccess: function(response) {
 *              document.getElementById("stringProperty").focus();},
 *        onFailure: function(error_code, error_message) {}
 *     });
 * }
 * </script>
 * \endcode
 *
 * ### Inports
 *   * __background__ Background to render web page ontop of.
 *
 * ### Outports
 *   * __webpage__ GUI elements rendered by web browser.
 *
 * ### Properties
 *   * __URL__ Link to webpage, online or file path.
 *   * __Reload__ Fetch page again.
 *   * __Property__ Type of property to add.
 *   * __Html id__ Identifier of html element to synchronize. Not allowed to contain dots, spaces
 * etc.
 *   * __Add property__ Create a property of selected type and identifier. Start to synchronize
 * against loaded webpage.
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_VISUALNEURO_API DataFrameWebBrowserProcessor : public WebBrowserProcessor {
public:
    DataFrameWebBrowserProcessor(InviwoApplication* app);
    virtual ~DataFrameWebBrowserProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    DataFrameMultiInport dataFramePort_;
private:
    bool reloaded_ = true;
    IMPLEMENT_REFCOUNTING(DataFrameWebBrowserProcessor);

};
#include <warn/pop>
}  // namespace inviwo

