#--------------------------------------------------------------------
# Brain Cohort Analyzer
# Do not name the CMake project VisualNeuro because it can be confused with the module
project(VisualNeuroApp)

include(version.cmake)


find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES 
    consolewidget.h 
    visualneuromainwindow.h 
    splashscreen.h
)
# Add resource file
qt_add_resources(QRC_FILE "resources/visualneuro.qrc")

source_group("Resource Files" FILES "resources/visualneuro.qrc" ${QRC_FILE})

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES  
    ${MOC_FILES})
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
	consolewidget.cpp
    visualneuro.cpp
    visualneuromainwindow.cpp
    splashscreen.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})


set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} "resources/visualneuro.rc")
elseif(APPLE)
    set_source_files_properties(resources/favicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RES_FILES ${RES_FILES} resources/favicon.icns)
endif()
source_group("Resource Files" FILES ${RES_FILES})

# Create application
add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 
    ${SOURCE_FILES} 
    ${HEADER_FILES} 
    ${QRC_FILE} 
    ${MOCED_FILES} 
    ${RES_FILES})
# Do not name the CMake project VisualNeuro because it can be confused with the module
# However, we still want the the executable to be called VisualNeuro
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "VisualNeuro")
target_link_libraries(${PROJECT_NAME} PUBLIC
    inviwo::core
    inviwo::module-system
    inviwo::qtapplicationbase
    inviwo::module::qtwidgets
    Qt6::Core
    Qt6::Widgets
)
target_include_directories(${PROJECT_NAME} PUBLIC
     ${CMAKE_BINARY_DIR}/apps/visualneuro/include
)
ivw_define_standard_definitions(${PROJECT_NAME} ${PROJECT_NAME})
ivw_define_standard_properties(${PROJECT_NAME})


#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_targets(${PROJECT_NAME})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Configure Info.plist
    # https://cmake.org/cmake/help/v3.15/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
    string(TIMESTAMP YEAR "%Y")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_STRING "${IVW_APP_INSTALL_NAME} ${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "favicon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.VisualNeuro"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_BUNDLE_NAME "${IVW_APP_INSTALL_NAME}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "C) 2012-${YEAR} Daniel Jönsson"
    )

    install(
        DIRECTORY   ${IVW_ROOT_DIR}/data/
        DESTINATION ${IVW_APP_INSTALL_NAME}.app/Contents/Resources/data
        COMPONENT   Application
    )
    
    set(CPACK_BINARY_DRAGNDROP ON)
endif()

# Debugging only:
#if (WIN32)
#    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS 
#        " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
#endif()

ivw_deploy_qt(${PROJECT_NAME})

set_property(CACHE IVW_PACKAGE_SELECT_APP APPEND PROPERTY STRINGS "VisualNeuroApp")

if(IVW_PACKAGE_SELECT_APP STREQUAL "VisualNeuroApp")
    include(packaging/packaging.cmake) # Package creation
endif()
