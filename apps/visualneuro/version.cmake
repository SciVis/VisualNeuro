set(VISUALNEURO_MAJOR_VERSION 1)
set(VISUALNEURO_MINOR_VERSION 1)
set(VISUALNEURO_PATCH_VERSION 0)
set(VISUALNEURO_BUILD_VERSION 0) # set to zero when doing a release, bump to 1 directly after the release. 

if(${VISUALNEURO_BUILD_VERSION})
    set(VISUALNEURO_VERSION ${VISUALNEURO_MAJOR_VERSION}.${VISUALNEURO_MINOR_VERSION}.${VISUALNEURO_PATCH_VERSION}.${VISUALNEURO_BUILD_VERSION})
else() # if VISUALNEURO_BUILD_VERSION is not set or set to zero 
    set(VISUALNEURO_VERSION ${VISUALNEURO_MAJOR_VERSION}.${VISUALNEURO_MINOR_VERSION}.${VISUALNEURO_PATCH_VERSION})
endif()

#Generate headers
configure_file(visualneurocommondefines_template.h 
                 ${CMAKE_BINARY_DIR}/apps/visualneuro/include/visualneuro/visualneurocommondefines.h 
                 @ONLY IMMEDIATE)