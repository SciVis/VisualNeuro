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

#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/stringconversion.h>

#include <string>
#include <optional>

namespace inviwo {

/**
 * \brief Labels for each position in a volume
 * Each label has a name and may be associated with a color.
 *
 */
class IVW_MODULE_VISUALNEURO_API VolumeAtlas {
public:
    struct Label {
        std::string name;
        std::optional<glm::vec4> color;
        bool operator==(std::string label) { return name == label; }
    };
    /*
     * Create an AtlasVolume associating indices in the Volume with labels.
     * Each label may optionally have a color, which are assumed to be in column "Color".
     * @param atlas integer volume, where each voxel value maps to a label.
     * @param labels with at least one column, but preferably three (Index/Region/Color). The index
     * column should be named "Index". If not, the first column with Integer numbers is assumed to
     * contain label indices. Column "Region" will be used as name of the label. If no "Region"
     * column is found the first CategoricalColumn will be used, and if none exist the labels will
     * have empty names. If no column with name "Color" is found the labels will not have colors.
     * @throws inviwo::Exception if index column could not be found.
     */
    VolumeAtlas(std::shared_ptr<const Volume> atlas, std::shared_ptr<const DataFrame> labels);
     /*
     * Get id of the label at voxel position.
     * indexPos can be outside the atlas volume, in which case -1 will be returned.
     * @return label index if found, -1 otherwise.
     */
    int getLabelId(const ivec3 indexPos) const;
    /*
     * Get id of the label first matching provided label.
     * @return label index if found, -1 otherwise.
     */
    int getLabelId(std::string label) const;
    /*
     * Get label id corresponding the worldPos.
     * worldPos can be outside the atlas volume, in which case -1 will be returned.
     * @return label index if found, -1 otherwise.
     */
    int getLabelId(vec3 worldPos) const;
    /*
     * Get coordinate of label id in [0 1] using the DataMapper of the Volume used for the atlas.
     * @return texture coordinate of labelId.
     */
    double getLabelIdNormalized(int labelId) const;
    /*
     * Get Label of label id.
     * @return Label if found.
     */
    std::optional<Label> getLabel(int id) const;
    /*
     * Get name of label id.
     * @return name if found, empty string otherwise.
     */
    std::string getLabelName(int id) const;
    /*
     * Get color of label id.
     * @return color if existing.
     */
    std::optional<vec4> getLabelColor(int id) const;

    /*
     * Check if any labels has colors.
     * @return true if any of the labels has a color.
     */
    bool hasColors() const;

private:
    std::shared_ptr<const Volume> atlas_;
    std::map<int, Label> labels_;
};

}  // namespace inviwo
