/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/visualneuro/processors/volume4dsequencesource.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Volume4DSequenceSource::processorInfo_{
    "org.inviwo.Volume4DSequenceSource",  // Class identifier
    "Volume 4D Sequence Source",          // Display name
    "Data Input",                         // Category
    CodeState::Stable,                    // Code state
    Tags::CPU,                            // Tags
};
const ProcessorInfo Volume4DSequenceSource::getProcessorInfo() const { return processorInfo_; }

Volume4DSequenceSource::Volume4DSequenceSource(InviwoApplication* app)
    : PoolProcessor()
    , rf_{app->getDataReaderFactory()}
    , outport_("data")
    , inputType_("inputType", "Input type",
                 {{"singlefile", "Single File", InputType::SingleFile},
                  {"folder", "Folder", InputType::Folder}},
                 0)
    , file_("filename", "Volume file")
    , folder_("folder", "Volume folder")
    , filter_("filter_", "Filter", "*.*")
    , reload_("reload", "Reload data")
    , mirrorRanges_("mirrorRanges", "Mirror Data and Value Ranges", false)
    , basis_("Basis", "Basis and offset")
    , information_("Information", "Data information") {
    file_.setContentType("volume");
    folder_.setContentType("volume");

    // make sure that we always process even if not connected
    isSink_.setUpdate([]() { return true; });

    addFileNameFilters();

    addPort(outport_);

    addProperty(inputType_);
    addProperty(folder_);
    addProperty(filter_);
    addProperty(file_);
    addProperty(reload_);
    addProperty(mirrorRanges_);
    addProperty(information_);
    addProperty(basis_);

    // It does not make sense to change these for an entire sequence
    basis_.setReadOnly(true);
    // Want to be able to change value range for entire sequence
    for (auto prop : information_) {
        prop->setReadOnly(prop != &information_.valueRange_);
    }

    auto updateVisible = [&]() {
        file_.setVisible(inputType_.get() == InputType::SingleFile);
        folder_.setVisible(inputType_.get() == InputType::Folder);
        filter_.setVisible(inputType_.get() == InputType::Folder);
    };

    inputType_.onChange(updateVisible);
    updateVisible();
}

std::shared_ptr<Volume4DSequence> Volume4DSequenceSource::loadFile(std::string_view path,
                                                                   const FileExtension& sext,
                                                                   DataReaderFactory* rf,
                                                                   pool::Progress& progress) {
    auto volumes = std::make_shared<Volume4DSequence>();

    progress(0.f);
    if (auto reader = rf->getReaderForTypeAndExtension<VolumeSequence>(sext, path)) {
        try {
            volumes = std::make_shared<Volume4DSequence>(1, reader->readData(path.data(), this));
        } catch (DataReaderException const& e) {
            LogProcessorError(e.getMessage());
        }
    } else {
        LogProcessorError("Could not find a data reader for file: " << path);
    }
    progress(1.f);
    return volumes;
}

std::shared_ptr<Volume4DSequence> Volume4DSequenceSource::loadFolder(std::string_view folder,
                                                                     DataReaderFactory* rf,
                                                                     pool::Stop stop,
                                                                     pool::Progress& progress) {
    progress(0.f);
    auto volumes = std::make_shared<Volume4DSequence>();
    auto files = filesystem::getDirectoryContents(folder.data());
    for (auto&& [ind, f] : util::enumerate(files)) {
        auto file = std::string(folder) + "/" + f;
        if (filesystem::wildcardStringMatch(filter_, file)) {
            try {
                if (auto reader1 = rf->getReaderForTypeAndExtension<Volume>(file)) {
                    auto volume = reader1->readData(file, this);
                    volume->setMetaData<StringMetaData>("filename", file);
                    volumes->push_back(std::make_shared<VolumeSequence>(1, volume));

                } else if (auto reader2 = rf->getReaderForTypeAndExtension<VolumeSequence>(file)) {
                    auto volumeSeq = reader2->readData(file, this);

                    for (auto volume : *volumeSeq) {
                        volume->setMetaData<StringMetaData>("filename", file);
                    }
                    volumes->push_back(volumeSeq);
                } else {
                    LogProcessorError("Could not find a data reader for file: " << file);
                }
            } catch (DataReaderException const& e) {
                LogProcessorError(e.getMessage());
            }
        }
        progress(ind + 1, files.size());
        if (stop) {
            return nullptr;
        }
    }
    return volumes;
}

void Volume4DSequenceSource::addFileNameFilters() {
    file_.clearNameFilters();
    file_.addNameFilter(FileExtension::all());
    file_.addNameFilters(rf_->getExtensionsForType<VolumeSequence>());
}

void Volume4DSequenceSource::process() {
    if (file_.isModified() || reload_.isModified() || folder_.isModified() ||
        filter_.isModified() || mirrorRanges_.isModified()) {

        const auto load = [this, path = getPath(), inputType = inputType_.get(),
                           sext = file_.getSelectedExtension(),
                           rf = rf_](pool::Stop stop,
                                     pool::Progress progress) -> std::shared_ptr<Volume4DSequence> {
            if (getPath().empty()) {
                return std::make_shared<Volume4DSequence>();
            }
            switch (inputType) {
                case InputType::Folder:
                    return loadFolder(path, rf, stop, progress);
                    break;
                case InputType::SingleFile:
                default:
                    return loadFile(path, sext, rf, progress);
                    break;
            }
        };
        dispatchOne(load, [this](std::shared_ptr<Volume4DSequence> result) {
            if (result && !result->empty()) {
                auto valueRange = dvec2(std::numeric_limits<double>::max(),
                                        std::numeric_limits<double>::lowest());
                auto dataRange = dvec2(std::numeric_limits<double>::max(),
                                       std::numeric_limits<double>::lowest());

                // store filename in metadata
                for (const auto& volume4d : *result) {
                    for (const auto& volume : *volume4d) {
                        if (!volume->hasMetaData<StringMetaData>("filename"))
                            volume->setMetaData<StringMetaData>("filename", file_.get());

                        valueRange.x = std::min(valueRange.x, volume->dataMap_.valueRange.x);
                        valueRange.y = std::max(valueRange.y, volume->dataMap_.valueRange.y);

                        dataRange.x = std::min(dataRange.x, volume->dataMap_.dataRange.x);
                        dataRange.y = std::max(dataRange.y, volume->dataMap_.dataRange.y);
                    }
                    if (!volume4d->empty()) {
                        // set basis of first volume
                        if ((*volume4d)[0]) {
                            basis_.updateForNewEntity(*(*volume4d)[0], deserialized_);
                            information_.updateForNewVolume(
                                *(*volume4d)[0], deserialized_ ? util::OverwriteState::Yes
                                                               : util::OverwriteState::No);
                        }
                    }
                }
                if (mirrorRanges_.get()) {
                    dataRange.x = -std::max(std::abs(dataRange.x), std::abs(dataRange.y));
                    dataRange.y = std::abs(dataRange.x);
                    valueRange.x = -std::max(std::abs(valueRange.x), std::abs(valueRange.y));
                    valueRange.y = std::abs(valueRange.x);
                }
                if (deserialized_) {
                    Property::setStateAsDefault(information_.valueRange_, valueRange);
                    Property::setStateAsDefault(information_.dataRange_, dataRange);
                } else {
                    information_.valueRange_ = valueRange;
                    information_.dataRange_ = dataRange;
                    information_.valueRange_.setCurrentStateAsDefault();
                    information_.dataRange_.setCurrentStateAsDefault();
                }
            }
            deserialized_ = false;
            if (result && !result->empty()) {
                // Update value range
                for (auto& volumeSequence : *result) {
                    for (auto& volume : *volumeSequence) {
                        volume->dataMap_.dataRange = dvec2(
                            volume->dataMap_.mapFromValueToData(information_.valueRange_.get().x),
                            volume->dataMap_.mapFromValueToData(information_.valueRange_.get().y));
                        volume->dataMap_.valueRange = information_.valueRange_.get();
                    }
                }
                outport_.setData(result);
                newResults();
            }
        });
    }
}

std::string Volume4DSequenceSource::getPath() const {
    switch (inputType_.get()) {
        case InputType::Folder:
            return folder_.get();
            break;
        case InputType::SingleFile:
        default:
            return file_.get();
            break;
    }
}

void Volume4DSequenceSource::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    addFileNameFilters();
    // It does not make sense to change these for an entire sequence
    basis_.setReadOnly(true);
    deserialized_ = true;
}

}  // namespace inviwo
