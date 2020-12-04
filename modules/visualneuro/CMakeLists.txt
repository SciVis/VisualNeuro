# Download and extract the Brain atlases since they are too large to store in git
include("cmake/DownloadBrainAtlas.cmake")
DownloadBrainAtlas("${CMAKE_CURRENT_SOURCE_DIR}/data/atlases/MNI/colin27")

#--------------------------------------------------------------------
# Inviwo VisualNeuro Module
ivw_module(VisualNeuro)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/visualneuro/visualneuromodule.h
    include/modules/visualneuro/visualneuromoduledefine.h
    include/modules/visualneuro/algorithm/volume/atlasvolumemask.h
    include/modules/visualneuro/datastructures/volumeatlas.h
    include/modules/visualneuro/processors/brainmask.h
    include/modules/visualneuro/processors/brainraycaster.h
    include/modules/visualneuro/processors/camerapositioncontroller.h
    include/modules/visualneuro/processors/dataframecolumnfilter.h
    include/modules/visualneuro/processors/dataframewebbrowserprocessor.h
    include/modules/visualneuro/processors/fmritransferfunctioncontroller.h
    include/modules/visualneuro/processors/groupcontroller.h
    include/modules/visualneuro/processors/joindataframes.h
    include/modules/visualneuro/processors/parametervolumesequencecorrelation.h
    include/modules/visualneuro/processors/volume4dsequenceslicefilter.h
    include/modules/visualneuro/processors/volume4dsequencesource.h
    include/modules/visualneuro/processors/volumeatlas.h
    include/modules/visualneuro/processors/volumeatlascenterpositions.h
    include/modules/visualneuro/processors/volumeregionparametercorrelation.h
    include/modules/visualneuro/processors/volumesequencefilter.h
    include/modules/visualneuro/processors/volumesequencemean.h
    include/modules/visualneuro/processors/volumettest.h
    include/modules/visualneuro/processors/volumevariancemean.h
    include/modules/visualneuro/statistics/correlation.h
    include/modules/visualneuro/statistics/distribution.h
    include/modules/visualneuro/statistics/parametervolumeregioncorrelation.h
    include/modules/visualneuro/statistics/pearsoncorrelation.h
    include/modules/visualneuro/statistics/spearmancorrelation.h
    include/modules/visualneuro/statistics/statisticstypes.h
    include/modules/visualneuro/statistics/ttest.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/visualneuromodule.cpp
    src/algorithm/volume/atlasvolumemask.cpp
    src/datastructures/volumeatlas.cpp
    src/processors/brainmask.cpp
    src/processors/brainraycaster.cpp
    src/processors/camerapositioncontroller.cpp
    src/processors/dataframecolumnfilter.cpp
    src/processors/dataframewebbrowserprocessor.cpp
    src/processors/fmritransferfunctioncontroller.cpp
    src/processors/groupcontroller.cpp
    src/processors/joindataframes.cpp
    src/processors/parametervolumesequencecorrelation.cpp
    src/processors/volume4dsequenceslicefilter.cpp
    src/processors/volume4dsequencesource.cpp
    src/processors/volumeatlas.cpp
    src/processors/volumeatlascenterpositions.cpp
    src/processors/volumeregionparametercorrelation.cpp
    src/processors/volumesequencefilter.cpp
    src/processors/volumesequencemean.cpp
    src/processors/volumettest.cpp
    src/processors/volumevariancemean.cpp
    src/statistics/correlation.cpp
    src/statistics/distribution.cpp
    src/statistics/parametervolumeregioncorrelation.cpp
    src/statistics/pearsoncorrelation.cpp
    src/statistics/spearmancorrelation.cpp
    src/statistics/statisticstypes.cpp
    src/statistics/ttest.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    glsl/brainraycaster.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/visualneuro-unittest-main.cpp
	tests/unittests/statistics-test.cpp
    tests/unittests/volume-mask-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})

#--------------------------------------------------------------------
# Register license for Colin 27 brain atlas
ivw_register_license_file(NAME "Colin 27 Average Brain, Stereotaxic Registration Model, high-resolution version 2008" MODULE VisualNeuro
URL "http://nist.mni.mcgill.ca/?p=947"
TYPE "BSD"
FILES  ${CMAKE_CURRENT_SOURCE_DIR}/data/atlases/MNI/Colin27/LICENSE.txt
)



