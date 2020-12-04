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

#pragma once

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>


namespace inviwo {

/** \docpage{org.inviwo.DataFrameColumnFilter, Data Frame Column Filter}
 * ![](org.inviwo.DataFrameColumnFilter.png?classIdentifier=org.inviwo.DataFrameColumnFilter)
 * Remove columns from input data frame. There is virtually no performance penalty as the output will 
 * use the same columns as the input.
 *
 * ### Inports
 *   * __Data frame__ Table to remove columns from.
 *
 * ### Outports
 *   * __Filtered data frame__ Table with columns removed.
 *
 * ### Properties
 *   * __filter__ List of columns to keep/remove.
 *   * __Secondary filter__ Select column to remove.
 */
class IVW_MODULE_VISUALNEURO_API DataFrameColumnFilter : public Processor {
public:
    DataFrameColumnFilter();
    virtual ~DataFrameColumnFilter() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<DataFrame> input_;
    DataOutport<DataFrame> output_;

    CompositeProperty filter_;
    DataFrameColumnProperty secondaryFilter_;
};

}  // namespace inviwo
