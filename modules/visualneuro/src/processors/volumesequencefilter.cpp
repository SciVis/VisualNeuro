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

#include <modules/visualneuro/processors/volumesequencefilter.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSequenceFilter::processorInfo_{
    "org.inviwo.VolumeSequenceFilter",  // Class identifier
    "Volume Sequence Filter",           // Display name
    "Data Selector",                    // Category
    CodeState::Experimental,            // Code state
    Tags::CPU,                          // Tags
};
const ProcessorInfo VolumeSequenceFilter::getProcessorInfo() const { return processorInfo_; }

VolumeSequenceFilter::VolumeSequenceFilter()
    : Processor()
    , inport_("inport")
    , dataFrame_("dataFrame_")
    , brushingAndLinking_("brushingAndLinking")
    , volumesOutport_("volumesOutport")
    , dataOutport_("dataOutport")
    , filenameIdColumn_("filenameIdColumn", "Volume Filename Column", dataFrame_, false, 1) {

    addPort(inport_);
    addPort(dataFrame_);
    addPort(brushingAndLinking_);
    brushingAndLinking_.setOptional(false);
    addPort(volumesOutport_);
    addPort(dataOutport_);

    addProperty(filenameIdColumn_);
}

void VolumeSequenceFilter::process() {
    auto volumesChanged = !brushingAndLinking_.isChanged();
    if (inport_.isChanged() || filenameIdColumn_.isModified() || (brushingAndLinking_.getFilteredIndices().size() != numberOfBrushed_)) {

        std::unordered_set<size_t> brushedIndices = brushingAndLinking_.getFilteredIndices();
        numberOfBrushed_ = brushedIndices.size();

        // Create a map between brushing id's and the volume filenames.
        std::unordered_map<std::string, int> volumeIdMapping;

        auto dataFrame = dataFrame_.getData();
        auto dfSize = dataFrame->getNumberOfRows();
        auto fileCol = dataFrame->getColumn(filenameIdColumn_.get());
        if (!fileCol) {
            if (dataFrame->getNumberOfColumns() > 0) {
                fileCol = dataFrame->getColumn(0);
            } else {
                throw Exception("No columns in the input data frame.", IVW_CONTEXT);
            }
            
        }
        for (auto brushingId = 0u; brushingId < dfSize; brushingId++) {
            auto filename = dataFrame->getColumn(filenameIdColumn_.get())->getAsString(brushingId);
            volumeIdMapping[filename] = brushingId;
        }

        // Check if two different filenames get mapped to the same brushingId.
        for (auto elem : volumeIdMapping)
            for (auto compare : volumeIdMapping) {
                if (elem.first != compare.first)
                    if (elem.second == compare.second)
                        LogWarn(elem.second << " has same id as " << compare.second << ": "
                                            << elem.first << " and " << compare.first);
                if (elem.second != compare.second)
                    if (elem.first == compare.first)
                        LogWarn(elem.second << " has same id as " << compare.second << " => "
                                            << elem.first << " and " << compare.first);
            }

        auto volumes = inport_.getData();
        VolumeSequence outputVolumes;

        if (dfSize != volumes->size()) {
            std::stringstream mess;
            mess << "The number of volumes does not match the number of entries in the data frame (.csv). Found "
            << volumes->size() << " volumes but " << dfSize << " rows.";
            throw Exception(mess.str(), IVW_CONTEXT);
        }

        for (const auto volume : *volumes) {

            std::string defaultFilepath = "No file path found";
            std::string filepath = volume->getMetaData<StringMetaData>("filename", defaultFilepath);

            // Add the filename as metadata to each volume
            if (volumesChanged || filepath != defaultFilepath) {
                if (volumesChanged || !volume->hasMetaData<StringMetaData>("volumeId")) {
                    std::string filename = filesystem::getFileNameWithExtension(filepath);
                    volume->setMetaData<StringMetaData>("volumeId", filename);
                }
            } else {
                LogWarn("No filename metadata could be found for the volume!");
            }

            std::string errorMessage = "No volume id";
            std::string volumeId = volume->getMetaData<StringMetaData>("volumeId", errorMessage);

            auto mapIterator = volumeIdMapping.find(volumeId);
            if (mapIterator == volumeIdMapping.end()) {
                LogWarn("Volume: " << volumeId << " cannot be mapped to a brushing id!");
                continue;
            }
            auto brushingId = mapIterator->second;
            auto brushedIterator = brushedIndices.find(brushingId);

            // Add volumes that are not in the brushed indices to the output
            if (brushedIterator == brushedIndices.end()) {
                outputVolumes.push_back(volume);
            }
        }

        // Add the filtered volumes to the volumesOutport
        std::shared_ptr<VolumeSequence> volumePointer =
            std::make_shared<VolumeSequence>(std::move(outputVolumes));

        if (volumePointer->empty()) {
            // Send volume with zeroes if all volumes has been brushed away.
            // Processors below will not be evaluated otherwise.
            volumePointer->push_back(std::make_shared<Volume>(
                std::make_shared<VolumeRAMPrecision<float>>(volumes->front()->getDimensions())));
            volumePointer->back()->dataMap_.dataRange = dvec2(0, 1);
            volumePointer->back()->dataMap_.valueRange = dvec2(0, 1);
        }
        volumesOutport_.setData(volumePointer);

        // Create dataframe with the id's of all the volumes selected through brushing so that a
        // list with brushed volumes can be exported.
        auto dataframe = std::make_shared<DataFrame>();
        std::vector<std::string> col;

        // Add all non-brushed volumes to the column vector.
        for (auto id : volumeIdMapping) {
            auto brushedId = brushedIndices.find(id.second);
            if (brushedId != brushedIndices.end()) col.push_back(id.first);
        }

        // Add the created column to the dataframe and send to outport.
        auto stringCol =
            dataframe->addCategoricalColumn(filenameIdColumn_.getSelectedDisplayName(), 0);
        for (auto element : col) stringCol->add(element);
        dataframe->updateIndexBuffer();
        dataOutport_.setData(dataframe);

        // If no change has been done in the brushing, take no action.
    } else if (brushingAndLinking_.getFilteredIndices().size() == numberOfBrushed_) {
        return;
        // If no volumes are brushed, send all volumes to the outports.
    } else {
        volumesOutport_.setData(inport_.getData());
        dataOutport_.setData(dataFrame_.getData());
    }
}

}  // namespace inviwo
