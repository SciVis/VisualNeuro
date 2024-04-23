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

#include <modules/visualneuro/datastructures/volumeatlas.h>

namespace inviwo {


VolumeAtlas::VolumeAtlas(std::shared_ptr<const Volume> atlas,
                                std::shared_ptr<const DataFrame> labels)
    : atlas_(atlas) {
    std::shared_ptr<const Column> idCol;
    std::shared_ptr<const Column> labelCol;
    std::shared_ptr<const Column> colorCol;
    for (auto col : *labels) {
        if (iCaseCmp(col->getHeader(), "Index") || iCaseCmp(col->getHeader(), "Label ID")) {
            idCol = col;
        } else if (iCaseCmp(col->getHeader(), "Region") ||
                   iCaseCmp(col->getHeader(), "Label Name")) {
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
            std::optional<glm::vec4> color{};
            std::string colorString = colorCol ? colorCol->getAsString(i) : "";
            try {
                color = color::hex2rgba(colorString);
            } catch (Exception&) {
                std::stringstream ss(colorString);
                glm::vec4 c{};
                for (auto elem = 0; elem < 4; elem++) {
                     ss >> c[elem];
                }
                if (ss.fail()) {
                    color = std::nullopt;
                } else {
                    color = c;
                }
            }

            labels_[static_cast<int>(idCol->getAsDouble(i))] = Label{
                labelCol->getAsString(i), color};
        }
    }
}

int VolumeAtlas::getLabelId(const ivec3 indexPos) const {
    if (glm::all(glm::lessThan(indexPos, ivec3(atlas_->getDimensions()))) &&
        glm::all(glm::lessThan(ivec3(0, 0, 0), indexPos))) {
        // get the voxel value at given position and look it up in the atlas
        return static_cast<int>(atlas_->getRepresentation<VolumeRAM>()->getAsDouble(indexPos));
    } else {
        return -1;
    }
}

int VolumeAtlas::getLabelId(std::string label) const {
    auto elemIt = std::find_if(labels_.begin(), labels_.end(),
                               [label](auto elem) { return elem.second == label; });
    if (elemIt == labels_.end()) {
        return -1;
    } else {
        return elemIt->first;
    }
}

int VolumeAtlas::getLabelId(vec3 worldPos) const {
    const mat4 trans = atlas_->getCoordinateTransformer().getWorldToIndexMatrix();
    const ivec3 indexPos(ivec3(trans * vec4(worldPos, 1)));
    return getLabelId(indexPos);
}

double VolumeAtlas::getLabelIdNormalized(int labelId) const {
    return atlas_->dataMap.mapFromValueToNormalized(static_cast<double>(labelId));
}

std::optional<VolumeAtlas::Label> VolumeAtlas::getLabel(int id) const {
    auto elemIt = labels_.find(id);
    if (elemIt == labels_.end()) {
        return std::nullopt;
    } else {
        return elemIt->second;
    }
}


std::string VolumeAtlas::getLabelName(int id) const {
    auto elemIt = labels_.find(id);
    if (elemIt == labels_.end()) {
        return "";
    } else {
        return elemIt->second.name;
    }
}

std::optional<vec4> VolumeAtlas::getLabelColor(int id) const {
    auto elemIt = labels_.find(id);
    if (elemIt == labels_.end()) {
        return std::nullopt;
    } else {
        return elemIt->second.color;
    }
}

bool VolumeAtlas::hasColors() const {
    return std::any_of(labels_.cbegin(), labels_.cend(),
                       [](const auto l) { return l.second.color != std::nullopt; });
}

vec3 VolumeAtlas::getCenterPoint(int label) const { return vec3(); }

VolumeAtlas::const_iterator VolumeAtlas::begin() const { return labels_.begin(); }

VolumeAtlas::const_iterator VolumeAtlas::end() const { return labels_.end(); }

}  // namespace inviwo
