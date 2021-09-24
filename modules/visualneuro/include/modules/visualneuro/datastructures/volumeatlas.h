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


#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/stringconversion.h>

#include <inviwo/dataframe/datastructures/dataframe.h>

#include <string>
#include <optional>

namespace inviwo {

/**
 * \brief Labels for each position in a volume
 *
 */
class IVW_MODULE_VISUALNEURO_API VolumeAtlas {
public:
    struct Label {
        std::string name;
        std::optional<glm::vec4> color;
        bool operator==(std::string label) { return name == label; }
    };

    VolumeAtlas(std::shared_ptr<const Volume> atlas, std::shared_ptr<const DataFrame> labels);


    int getLabelId(const ivec3 indexPos) const;
    int getLabelId(std::string label) const;
    int getLabelId(vec3 worldPos) const;

    double getLabelIdNormalized(int labelId) const;

    std::optional<Label> getLabel(int id) const;

    std::string getLabelName(int id) const;
    std::optional<vec4> getLabelColor(int id) const;

    bool hasColors() const;

private:
    std::shared_ptr<const Volume> atlas_;
    std::map<int, Label> labels_;
};

}  // namespace inviwo
