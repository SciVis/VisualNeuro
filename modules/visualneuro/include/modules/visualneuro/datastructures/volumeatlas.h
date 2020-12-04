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

namespace inviwo {

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
struct IVW_MODULE_VISUALNEURO_API VolumeAtlas {
    VolumeAtlas(std::shared_ptr<const Volume> atlas, std::shared_ptr<const DataFrame> labels)
        : atlas_(atlas), labels_(labels) {
        std::shared_ptr<const Column> idCol;
        std::shared_ptr<const Column> labelCol;
        for (auto col : *labels) {
            if (col->getHeader() == "Index") {
                idCol = col;
            } else if (col->getHeader() == "Region") {
                labelCol = col;
            }
        }
        if (!idCol) {
            auto headers = labels->getHeaders();
            idCol = *std::find_if(++labels->begin(), labels->end(), [](auto r) {
                auto type = r->getBuffer()->getDataFormat()->getNumericType();
                return type == NumericType::UnsignedInteger || type == NumericType::SignedInteger;
            });
        }
        if (!labelCol) {
            auto headers = labels->getHeaders();
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
                idToName_[static_cast<int>(idCol->getAsDouble(i))] = "";
            }
        } else {
            for (auto i = 0u; i < idCol->getSize(); ++i) {
                idToName_[static_cast<int>(idCol->getAsDouble(i))] = labelCol->getAsString(i);
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
        auto elemIt = std::find_if(idToName_.begin(), idToName_.end(),
                                   [label](auto elem) { return elem.second == label; });
        if (elemIt == idToName_.end()) {
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

    std::string getLabel(int id) const {
        auto elemIt = idToName_.find(id);
        if (elemIt == idToName_.end()) {
            return "";
        } else {
            return elemIt->second;
        }
    }

private:
    std::shared_ptr<const Volume> atlas_;
    std::shared_ptr<const DataFrame> labels_;
    std::map<int, std::string> idToName_;
};  

}  // namespace inviwo
