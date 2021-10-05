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

#include <modules/visualneuro/processors/dataframecolumnfilter.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameColumnFilter::processorInfo_{
    "org.inviwo.DataFrameColumnFilter",  // Class identifier
    "Data Frame Column Filter",          // Display name
    "Data frame",                        // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
};
const ProcessorInfo DataFrameColumnFilter::getProcessorInfo() const { return processorInfo_; }

DataFrameColumnFilter::DataFrameColumnFilter()
    : Processor()
    , input_("input")
    , output_("output")
    , filter_("filter", "Filter")
    , secondaryFilter_("secondaryFilter", "Remove", input_, true) {

    addPort(input_);
    addPort(output_);
    addProperties(filter_, secondaryFilter_);
    filter_.setSerializationMode(PropertySerializationMode::All);
    input_.onChange([&]() {
        std::unordered_set<Property*> previousProperties(filter_.getProperties().begin(),
                                                         filter_.getProperties().end());
        if (!input_.hasData()) return;
        auto data = input_.getData();
        if (!data->getNumberOfColumns()) return;

        for (size_t i = 0; i < data->getNumberOfColumns(); i++) {
            auto c = data->getColumn(i);
            std::string displayName = c->getHeader();
            std::string identifier = util::stripIdentifier(displayName);
            auto it = util::find_if(previousProperties, [identifier](auto elem) {
                return elem->getIdentifier() == identifier;
            });
            if (it == previousProperties.end()) {
                auto newProp = std::make_unique<BoolProperty>(identifier, displayName, true);
                filter_.addProperty(newProp.release(), true);
            } else {
                previousProperties.erase(*it);
            }
        }

        // Remove properties for axes that doesn't exist in the current dataframe
        for (auto prop : previousProperties) {
            filter_.removeProperty(prop);
        }
        filter_.setCurrentStateAsDefault();
    });
}

void DataFrameColumnFilter::process() {
    auto input = input_.getData();
    auto filtered = std::make_shared<DataFrame>();
    filtered->dropColumn(0);
    for (auto col = input->begin(); col != input->end(); col++) {
        const auto id = (*col)->getHeader();
        auto propIt = std::find_if(filter_.cbegin(), filter_.cend(),
                                   [id](const auto& elem) { return elem->getDisplayName() == id; });
        if (secondaryFilter_.getSelectedDisplayName() != id && propIt != filter_.end() &&
            static_cast<const BoolProperty*>(*propIt)->get()) {
            filtered->addColumn(*col);
        }
    }

    output_.setData(filtered);
}

}  // namespace inviwo
