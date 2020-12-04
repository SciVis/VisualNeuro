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

#include <modules/visualneuro/processors/syncpatientdata.h>
#include <modules/visualneuro/processors/volumesequencefilter.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo SyncPatientData::processorInfo_{
    "org.inviwo.SyncPatientData",  // Class identifier
    "Sync Patient Data",           // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};
const ProcessorInfo SyncPatientData::getProcessorInfo() const { return processorInfo_; }

SyncPatientData::SyncPatientData()
    : Processor()
    , mapping_("mapping")
    , patientData_("patientData")
    , folder_("folder", "Volume folder")
    , sync_("sync", "Sync patient data")
    , volumeFileIdKeyword_("volumeFileIdKeyword", "Volume file id keyword", "Subject") {

    addPort(mapping_);
    addPort(patientData_);

    addProperty(folder_);
    addProperty(sync_);
    addProperty(volumeFileIdKeyword_);

    sync_.onChange([this] { buttonPressed_ = true; });
}

void SyncPatientData::addFilesToMap(std::map<std::string, std::pair<std::string, std::string>>& m,
                                    std::map<std::string, std::string> mapping,
                                    std::string directory) {

    // Loop through the files in that folder, extract the patient id from the
    // filename, translate it and add all id's + filepaths to an map.
    //for (auto& p : std::filesystem::directory_iterator(directory)) {
    for (auto& p : filesystem::getDirectoryContents(directory, filesystem::ListMode::Files)) {
        auto fileAndPath = p;
        //std::string fileAndPath = p.path().string();
        std::string file = fileAndPath;

        auto startPositionToErase = file.find(directory);
        file.erase(startPositionToErase, directory.length());

        // For some reason a "\" is added to the path, which is not very useful...
        if (file.find("\\") != std::string::npos) {
            file.replace(file.find("\\"), 1, "");
        }

        // Ignores alla files that doesn't have the .nii file extension.
        if (file.find(".nii") == std::string::npos) continue;

        std::string id = VolumeSequenceFilter::extractPatientId(file, volumeFileIdKeyword_.get());

        std::string translatedId = mapping[id];

        m.insert(std::make_pair(translatedId, make_pair(fileAndPath, file)));
    }
}

void SyncPatientData::process() {

    if (buttonPressed_) {
        buttonPressed_ = false;

        if (folder_.get() == "") {
            LogError("No fMRI volume folder was set! (set it in processor properties)");
            return;
        }

        auto mapping = mapping_.getData();
        auto mappingSize = mapping->getNumberOfRows();

        // Create two maps between the two types of patient id's.
        // They both contain the same data but the id that is used for key in one map
        // is the value in the other map etc.
        std::map<std::string, std::string> patientIdsStringKey;
        std::map<std::string, std::string> patientIdsNumbKey;
        for (int i = 0; i < mappingSize; i++) {

            auto id_A = mapping->getColumn(1)->getAsString(i);
            auto id_B = mapping->getColumn(2)->getAsString(i);

            patientIdsNumbKey[id_B] = id_A;
            patientIdsStringKey[id_A] = id_B;
        }

        // Get the fMRI folder location
        std::string location_fMRI = folder_.get();
        std::map<std::string, std::pair<std::string, std::string>> files;

        addFilesToMap(files, patientIdsNumbKey, location_fMRI);

        // Go through .csv-file:
        // - all rows that cannot be found in the unordered_map are not copied to the new .csv-file.
        auto patientData = patientData_.getData();
        auto patientDataSize = patientData->getNumberOfRows();
        std::stringstream newCsvData;
        std::stringstream patientsWithoutScans;
        std::set<std::string> patientIdsWithData;

        int matches = 0, nonMatches = 0;
        for (int i = 0; i < patientDataSize; i++) {

            auto patientId = patientData->getColumn(1)->getAsString(i);

            // If the patient have a fMRI scan, add it to the new .csv file
            if (patientIdsStringKey.find(patientId) != patientIdsStringKey.end()) {
                for (int j = 1; j < patientData->getNumberOfColumns(); j++) {
                    newCsvData << patientData->getColumn(j)->getAsString(i) << ",";
                }
                newCsvData << "\n";
                matches++;
            } else {
                // For debugging purposes, save the patients without a matching fMRI scan.
                patientsWithoutScans << patientData->getColumn(1)->getAsString(i) << "\n";
                nonMatches++;
            }

            // Create a stringstream with all patients with other data than the scan.
            patientIdsWithData.insert(patientData->getColumn(1)->getAsString(i));
        }

        // Create a folder for all fMRI scans that doesn't have matching data in the .csv-file.
        const std::string unmatchedLocation = location_fMRI.append("/non_matching_scans/");
        filesystem::createDirectoryRecursively(unmatchedLocation);
        //std::filesystem::create_directory(unmatchedLocation);
        // Add potential files that has already been moved to the files-map, since it will be
        // used to list all mismatched patients in a logfile later on.
        addFilesToMap(files, patientIdsNumbKey, unmatchedLocation);

        int movedVolumes = 0;
        // Loop through the fMRI-files.
        for (auto file : files) {
            // If the file is not found among the patients with data
            if (patientIdsWithData.find(file.first) == patientIdsWithData.end()) {

                std::string newLocation = unmatchedLocation;
                newLocation.append(file.second.second);
                // Move the file to another folder.
                rename(file.second.first.c_str(), newLocation.c_str());
                movedVolumes++;
            }
        }

        // Get fMRI volumes location.
        std::string saveLocation = folder_.get();

        // Move up one directory.
        auto pos = saveLocation.find_last_of("/");
        saveLocation.erase(pos, saveLocation.length());
        std::string missmatchedCsv = saveLocation.append("/Non-matching patient cases.csv");

        // Create logfile and fill it with values.
        std::ofstream logFile;
        const std::string spacing = ",,,,,,";
        logFile.open(missmatchedCsv);
        logFile << "fMRI-scans that lack data in the .csv-file,," << spacing;
        logFile << "Patients in the .csv-file that cannot be coupled with a fMRI-scan\n";
        logFile << "ID,,Filename" << spacing << "ID\n";

        for (auto file : files) {

            if (patientIdsWithData.find(file.first) == patientIdsWithData.end()) {
                logFile << file.first << ",," << file.second.second;
            } else {
                continue;
            }

            if (patientsWithoutScans) {
                std::string temp;
                patientsWithoutScans >> temp;
                logFile << spacing << temp;
            }
            logFile << "\n";
        }
        while (patientsWithoutScans) {
            std::string temp;
            patientsWithoutScans >> temp;
            logFile << ",," << spacing << temp << "\n";
        }
        logFile.close();

        std::string newFileName;
        if (nonMatches > 0) {
            // Create a new .csv-file with only patients that has fMRI-scans.
            std::ofstream newCsvFile;

            // Get fMRI volumes location.
            saveLocation = folder_.get();

            // Move up one directory.
            pos = saveLocation.find_last_of("/");
            saveLocation.erase(pos, saveLocation.length());
            newFileName = saveLocation.append("/Patients with matching fMRI.csv");

            // Create a new file to put only matching patient cases into.
            newCsvFile.open(newFileName);
            for (auto header : patientData->getHeaders()) {
                newCsvFile << header.first << ",";
            }
            newCsvFile << "\n";
            while (newCsvData) {
                std::string temp;
                std::getline(newCsvData, temp);
                newCsvFile << temp << "\n";
            }
            newCsvFile.close();
        }

		// Print information about what has been done.
        LogInfo("Patients with fMRI-scans: " << matches);
        LogInfo("Patients without fMRI-scans: " << nonMatches);
        LogInfo("fMRI volumes without other patient data: " << movedVolumes);

        if (movedVolumes > 0)
            LogInfo("fMRI volumes with no matching data in given .csv-file was moved to:\n"
                    << unmatchedLocation);
        if (nonMatches) {
            LogInfo("New .csv-file containting only patients with matching fMRI volumes created at:\n"
                    << newFileName);
            LogInfo("A .csv-file with all mismatched patients has been created at:\n"
                    << missmatchedCsv);
        }
    }
}

}  // namespace inviwo
