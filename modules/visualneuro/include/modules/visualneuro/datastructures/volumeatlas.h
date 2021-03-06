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
 *
 */
class IVW_MODULE_VISUALNEURO_API VolumeAtlas {
public:
    struct Label {
        std::string name;
        std::optional<glm::vec4> color;
        bool operator==(std::string label) { return name == label; }
    };
    VolumeAtlas(std::shared_ptr<const Volume> atlas, std::shared_ptr<const DataFrame> labels)
        : atlas_(atlas) {
        std::shared_ptr<const Column> idCol;
        std::shared_ptr<const Column> labelCol;
        std::shared_ptr<const Column> colorCol;
        for (auto col : *labels) {
            if (iCaseCmp(col->getHeader(), "Index")) {
                idCol = col;
            } else if (iCaseCmp(col->getHeader(), "Region")) {
                labelCol = col;
            } else if (iCaseCmp(col->getHeader(), "Color")) {
                colorCol = col;
            }
        }
        if (!idCol) {
            idCol = *std::find_if(++labels->begin(), labels->end(), [](auto r) {
                auto type = r->getBuffer()->getDataFormat()->getNumericType();
                return type == NumericType::UnsignedInteger || type == NumericType::SignedInteger;
            });
        }
        if (!labelCol) {
            labelCol = *std::find_if(++labels->begin(), labels->end(), [](auto r) {
                return dynamic_cast<const CategoricalColumn*>(r.get());
            });
        }
        if (!idCol) {
            throw Exception(
                "Could not find Index column for atlas labels. Add Index column with id of each "
                "region");
        }
        if (!labelCol) {
            for (auto i = 0u; i < idCol->getSize(); ++i) {
                labels_[static_cast<int>(idCol->getAsDouble(i))] = Label{};
            }
        } else {
            for (auto i = 0u; i < idCol->getSize(); ++i) {
                std::string colorString = colorCol ? colorCol->getAsString(i) : "";
                std::stringstream ss(colorString);
                vec4 c{};
                for (auto elem = 0; elem < 4; elem++) {
                    ss >> c[elem];
                }
                labels_[static_cast<int>(idCol->getAsDouble(i))] =
                    Label{labelCol->getAsString(i),
                          ss.fail() ? std::nullopt : std::optional<glm::vec4>(c)};
            }
        }
    }

    int getLabelId(const ivec3 indexPos) const {
        if (glm::all(glm::lessThan(indexPos, ivec3(atlas_->getDimensions()))) &&
            glm::all(glm::lessThan(ivec3(0, 0, 0), indexPos))) {
            // get the voxel value at given position and look it up in the atlas
            return static_cast<int>(atlas_->getRepresentation<VolumeRAM>()->getAsDouble(indexPos));
        } else {
            return -1;
        }
    }
    int getLabelId(std::string label) const {
        auto elemIt = std::find_if(labels_.begin(), labels_.end(),
                                   [label](auto elem) { return elem.second == label; });
        if (elemIt == labels_.end()) {
            return -1;
        } else {
            return elemIt->first;
        }
    }
    int getLabelId(vec3 worldPos) const {
        const mat4 trans = atlas_->getCoordinateTransformer().getWorldToIndexMatrix();
        const ivec3 indexPos(ivec3(trans * vec4(worldPos, 1)));
        return getLabelId(indexPos);
    }

    double getLabelIdNormalized(int labelId) const {
        return atlas_->dataMap_.mapFromValueToNormalized(static_cast<double>(labelId));
    }

    std::optional<Label> getLabel(int id) const {
        auto elemIt = labels_.find(id);
        if (elemIt == labels_.end()) {
            return std::nullopt;
        } else {
            return elemIt->second;
        }
    }

    std::string getLabelName(int id) const {
        auto elemIt = labels_.find(id);
        if (elemIt == labels_.end()) {
            return "";
        } else {
            return elemIt->second.name;
        }
    }
    std::optional<vec4> getLabelColor(int id) const {
        auto elemIt = labels_.find(id);
        if (elemIt == labels_.end()) {
            return std::nullopt;
        } else {
            return elemIt->second.color;
        }
    }

    bool hasColors() const {
        return std::any_of(labels_.cbegin(), labels_.cend(),
                           [](const auto l) { return l.second.color != std::nullopt; });
    }

private:
    std::shared_ptr<const Volume> atlas_;
    std::map<int, Label> labels_;
};

}  // namespace inviwo
