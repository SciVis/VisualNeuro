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

#include <modules/visualneuro/processors/volume4dsequenceslicefilter.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Volume4DSequenceSliceFilter::processorInfo_{
    "org.inviwo.Volume4DSequenceSliceFilter",  // Class identifier
    "Volume 4D Sequence Slice Filter",         // Display name
    "Data Selector",                           // Category
    CodeState::Experimental,                   // Code state
    Tags::CPU,                                 // Tags
};
const ProcessorInfo Volume4DSequenceSliceFilter::getProcessorInfo() const { return processorInfo_; }

Volume4DSequenceSliceFilter::Volume4DSequenceSliceFilter()
    : Processor(), inport_("inport"), outport_("volumesOutport"), index_("index", "Index", 1, 1, 1, 1) {

    addPort(inport_);
    addPort(outport_);

    addProperty(index_);

    inport_.onChange([this]() {
        if (inport_.hasData() && !inport_.getData()->empty()) {
            auto max = inport_.getData()->front()->size();
            if (max != index_.getMaxValue()) {
                index_.setMaxValue(max);
                index_.set(1);
                index_.setCurrentStateAsDefault();
            }
        }
    });
}

void Volume4DSequenceSliceFilter::process() {

    if (auto data = inport_.getData()) {
        if (data->size() == 0) {
            outport_.detachData();
            return;
        }
        size_t index = std::min(data->front()->size() - 1, static_cast<size_t>(index_.get() - 1));
        auto volumes = std::make_shared<VolumeSequence>();
        for (auto sequence : *inport_.getData()) {
            volumes->push_back(sequence->at(index));
        }
        outport_.setData(volumes);
    }

}

}  // namespace inviwo
