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

#ifndef IVW_SYNCPATIENTDATA_H
#define IVW_SYNCPATIENTDATA_H

#include <modules/visualneuro/visualneuromoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

/** \docpage{org.inviwo.SyncPatientData, Sync Patient Data}
 * ![](org.inviwo.SyncPatientData.png?classIdentifier=org.inviwo.SyncPatientData)
 *
 *	Synchronizes patient data. Moves volumes that does not have any corresponding data in the
 *	file with patient data to a separate folder and creates a new .csv-file containing only data
 *	that has a corresponding volume. The processor also creates a .csv logfile with all the
 *	volumes that were moved and all the lines that were not included in the new synchronized
 *	.csv-file.
 *
 * ### Inports
 *   * __mapping__ Dataframe containing a mapping between 2 types of patient id's
 *	 * __patientData__ Dataframe containing patient data
 *	 * __folder__ Folder containting patient volumes
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __sync__ When pressed, moves volumes with no corresponding patient data to a separate folder
 *		and creates a new .csv-file containing only the patient data that has a
 *corresponding volume
 *   * __volumeFileIdKeyword__ Tells the processor after which keyword in the volume filenames that
 *		the patient id comes
 */

/**
 * \class SyncPatientData
 * \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
 * DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_VISUALNEURO_API SyncPatientData : public Processor {
public:
    SyncPatientData();
    virtual ~SyncPatientData() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<DataFrame> mapping_;
    DataInport<DataFrame> patientData_;

    DirectoryProperty folder_;
    ButtonProperty sync_;

    StringProperty volumeFileIdKeyword_;

    bool buttonPressed_ = false;

    // Function for adding filenames and patient id's to a map.
    void addFilesToMap(std::map<std::string, std::pair<std::string, std::string>> &m,
                       std::map<std::string, std::string> mapping, std::string directory);
};

}  // namespace inviwo

#endif  // IVW_SYNCPATIENTDATA_H
