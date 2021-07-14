# Before Configure in CMAKE:
# Add Entry
#   Name: CMAKE_PROJECT_inviwo-projects_INCLUDE
# 	Type FilePath
#   Value: C:/Users/danjo/Documents/inviwo/visualneuro/apps/visualneuro/inviwo-projects-defaults.cmake
#
# Press configure a couple of times
#
# Config
set(IVW_EXTERNAL_MODULES 
"C:/Users/danjo/Documents/inviwo/visualneuro/modules"
)
# Apps
set(IVW_EXTERNAL_PROJECTS "C:/Users/danjo/Documents/inviwo/visualneuro/apps")

set(IVW_APP_VISUALNEURO             	ON)
set(IVW_APP_INVIWO             			OFF)
set(IVW_APP_PYTHON             			OFF)
# Cannot be done just after inviwo-projects creation
# if(WIN32 AND MSVC)
#	set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT VisualNeuro)
# endif()

# Testing
set(IVW_TEST_UNIT_TESTS_RUN_ON_BUILD 	OFF)
set(IVW_TEST_BENCHMARKS                 OFF)
set(IVW_TEST_INTEGRATION_TESTS          OFF)

# Packaging
set(IVW_PACKAGE_INSTALLER            		ON)
set(IVW_PACKAGE_SELECT_APP "VisualNeuro")

# Modules
set(IVW_MODULE_ANIMATION 					OFF)
set(IVW_MODULE_ANIMATIONQT 					OFF)
set(IVW_MODULE_DATAFRAMEPYTHON 				OFF)
set(IVW_MODULE_DATAFRAMEQT 					OFF)
set(IVW_MODULE_EIGENUTILS 					OFF)
set(IVW_MODULE_PYTHON3 						OFF)
set(IVW_MODULE_PYTHON3QT 					OFF)
set(IVW_MODULE_VECTORFIELDVISUALIZATION 	OFF)
set(IVW_MODULE_VECTORFIELDVISUALIZATIONGL 	OFF)
set(IVW_MODULE_WEBBROWSER					ON)

set(IVW_MODULE_VISUALNEURO					ON)
