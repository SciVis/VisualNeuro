#--------------------------------------------------------------------
# Brain Cohort Analyzer
project(VisualNeuroApp)

include(version.cmake)

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions   
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES 
	consolewidget.h 
    visualneuromainwindow.h 
    splashscreen.h
)
# Add resource file
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_resources(QRC_FILE "visualneuro.qrc")
else()
    qt5_add_resources(QRC_FILE "visualneuro.qrc")
endif()
source_group("Resource Files" FILES visualneuro.qrc ${QRC_FILE})

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
if(QT_VERSION_MAJOR EQUAL 6)
    qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
else()
    qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
endif()
source_group("MOC Files" FILES ${MOCED_FILES})


ivw_retrieve_all_modules(enabled_modules)

list(APPEND enabled_modules [InviwoVisualNeuroModule])
# Only include one OpenGL context creation module (remove InviwoGLFWModule)
# Remove modules not used by this app
list(REMOVE_ITEM enabled_modules [InviwoAnimationModule InviwoAnimationQtModule InviwoGLFWModule InviwoEigenUtilsModule InviwoPython3Module  
                                  InviwoPython3QtModule InviwoPython3GLModule InviwoDataFramePythonModule InviwoVectorFieldVisualizationModule InviwoVectorFieldVisualizationGLModule])

set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} "visualneuro.rc")
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
    inviwo::qtapplicationbase
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)
target_include_directories(${PROJECT_NAME} PUBLIC
     ${CMAKE_BINARY_DIR}/apps/visualneuro/include
)
ivw_configure_application_module_dependencies(${PROJECT_NAME} ${enabled_modules})
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
        MACOSX_BUNDLE_INFO_STRING "Visual Neuro ${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "favicon.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.VisualNeuro"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_BUNDLE_NAME "Visual Neuro"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${VISUALNEURO_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "C) 2012-${YEAR} Daniel Jönsson"
    )
    # Following http://www.cmake.org/Wiki/BundleUtilitiesExample
    install(
        TARGETS ${PROJECT_NAME}
        RUNTIME   DESTINATION VisualNeuro.app/Contents/MacOS/
        BUNDLE    DESTINATION .
        ARCHIVE   DESTINATION VisualNeuro.app/Contents/MacOS/
        LIBRARY   DESTINATION VisualNeuro.app/Contents/MacOS/
        COMPONENT Application
    )
    set(plugin_dest_dir VisualNeuro.app/Contents/MacOS)
    set(qtconf_dest_dir VisualNeuro.app/Contents/Resources)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/VisualNeuro.app/Contents/MacOS/VisualNeuro")
    set(DIRS "\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/")

    # get link errors with out this line.
    install(CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")" COMPONENT qt_app)
    install(CODE "set(BU_CHMOD_BUNDLE_ITEMS ON)
                include(BundleUtilities)
                file(GLOB_RECURSE LIBS1 \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}\")
                file(GLOB_RECURSE LIBS2 \"\${CMAKE_INSTALL_PREFIX}/VisualNeuro.app/Contents/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
                fixup_bundle(\"${APPS}\" \"\${LIBS1};\${LIBS2}\" \"${DIRS}\")"
        COMPONENT Application
    )
    install(
        DIRECTORY   ${IVW_ROOT_DIR}/data/
        DESTINATION VisualNeuro.app/Contents/Resources/data
        COMPONENT   Application
    )
    #set_target_properties(${PROJECT_NAME} PROPERTIES
    #            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/VisualNeuro-Info.plist
    #        )
    set(CPACK_BINARY_DRAGNDROP ON)
endif()

# Debugging only:
#if (WIN32)
#    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS 
#        " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
#endif()

ivw_deploy_qt(${PROJECT_NAME})

set_property(CACHE IVW_PACKAGE_SELECT_APP APPEND PROPERTY STRINGS "VisualNeuro")

if(IVW_PACKAGE_SELECT_APP STREQUAL "VisualNeuro")
    include(packaging/packaging.cmake) # Package creation
endif()
