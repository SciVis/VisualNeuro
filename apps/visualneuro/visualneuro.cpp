/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/qtapptools.h>
#include <inviwo/qt/applicationbase/qtlocale.h>
#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulemanager.h>

#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/localetools.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/logerrorcounter.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/filelogger.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/sys/moduleloading.h>


#include "splashscreen.h"
#include "visualneuromainwindow.h"

#include <QMessageBox>
#include <QApplication>



using namespace inviwo;

int main(int argc, char** argv) {
    inviwo::LogCentral logger;
    inviwo::LogCentral::init(&logger);
    auto logCounter = std::make_shared<inviwo::LogErrorCounter>();
    logger.registerLogger(logCounter);
#ifdef __linux__
    /*
     * Suppress warning "QApplication: invalid style override passed, ignoring it." when starting
     * Inviwo on Linux. See
     * https://forum.qt.io/topic/75398/qt-5-8-0-qapplication-invalid-style-override-passed-ignoring-it/2
     */
    qputenv("QT_STYLE_OVERRIDE", "");
#endif
    // Must be set before constructing QApplication
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // Setup default context format
    //QSurfaceFormat defaultFormat;
    //defaultFormat.setMajorVersion(10); // We want latest OpenGL version
    //defaultFormat.setProfile(QSurfaceFormat::CoreProfile);
    //QSurfaceFormat::setDefaultFormat(defaultFormat);

    QApplication qtApp{argc, argv};
    InviwoApplication inviwoApp(argc, argv, "Visual Neuro");

    //inviwo::utilqt::configureInviwoDefaultNames();
    inviwo::utilqt::configureFileSystemObserver(inviwoApp);
    inviwo::utilqt::configurePostEnqueueFront(inviwoApp);
    inviwo::utilqt::setStyleSheetFile(":/stylesheets/visualneuro.qss");
    //inviwo::utilqt::configurePalette();
    inviwoApp.setUILocale(inviwo::utilqt::getCurrentStdLocale());

    inviwoApp.setProgressCallback([](std::string_view m) {
        LogCentral::getPtr()->log("VisualNeuro", LogLevel::Info, LogAudience::User, "", "", 0, m);
    });
    auto& clp = inviwoApp.getCommandLineParser();

    VisualNeuroMainWindow mainWin(&inviwoApp);
    inviwoApp.printApplicationInfo();

    // initialize and show splash screen
    inviwo::InviwoSplashScreen splashScreen(clp.getShowSplashScreen());
    inviwoApp.setProgressCallback([&splashScreen](std::string_view s) { splashScreen.showMessage(s); });

    splashScreen.show();
    splashScreen.showMessage("Loading application...");

    // Initialize application and register modules
    splashScreen.showMessage("Initializing modules...");
    // Remove GLFW module register since we will use Qt for the OpenGL context
    auto filter = [](const inviwo::ModuleContainer& m) { return m.identifier() == "glfw"; };
    inviwo::util::registerModulesFiltered(inviwoApp.getModuleManager(), filter,
                                          inviwoApp.getSystemSettings().moduleSearchPaths_.get(),
                                          clp.getModuleSearchPaths());

    qtApp.processEvents();


    // Do this after registerModules if some arguments were added
    clp.parse(inviwo::CommandLineParser::Mode::Normal);

    qtApp.processEvents();  // Update GUI
    splashScreen.showMessage("Loading workspace...");
    qtApp.processEvents();
    mainWin.showWindow();
    qtApp.processEvents();  // Make sure the gui is done loading before loading workspace

    // Need to clear the network and (will delete processors and processorwidgets)
    // before QMainWindoes is deleted, otherwise it will delete all processorWidgets
    // before Processor can delete them.
    util::OnScopeExit clearNetwork([&]() { inviwoApp.getProcessorNetwork()->clear(); });

    // Load workspace
    inviwoApp.getProcessorNetwork()->lock();
    auto brainModule = inviwoApp.getModuleByIdentifier("VisualNeuro");
    if (!brainModule) {
        QMessageBox::critical(
                    &mainWin, "Fatal Error", "Missing VisualNeuro module. Enable the module in CMake.",
                    QMessageBox::Close, QMessageBox::Close);
        mainWin.exitInviwo(false);
        return 0;
    }
    const auto workspace =
        clp.getLoadWorkspaceFromArg()
            ? clp.getWorkspacePath()
            : brainModule->getPath(ModulePath::Workspaces) / "VisualNeuro.inv";

    if (!workspace.empty()) {
        mainWin.openLastWorkspace(workspace);
    }

    inviwoApp.processFront();
    inviwoApp.getProcessorNetwork()->unlock();

    mainWin.showWindow();
    qtApp.processEvents();
    splashScreen.finish(&mainWin);

    clp.processCallbacks();  // run any command line callbacks from modules.
    inviwoApp.processEvents();

    if (clp.getQuitApplicationAfterStartup()) {
        mainWin.exitInviwo(false);
        return 0;
    } 

    inviwo::util::setThreadDescription("Visual Neuro Main");

    while (true) {
        try {
            return qtApp.exec();
        } catch (const inviwo::Exception& e) {
            inviwo::util::log(e.getContext(), e.getFullMessage(), inviwo::LogLevel::Error);
            const auto message = fmt::format(
                "{}\nApplication state might be corrupted, be warned.", e.getFullMessage(10));

            auto res = QMessageBox::critical(
                &mainWin, "Fatal Error", inviwo::utilqt::str(message),
                QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
            if (res == QMessageBox::Close) {
                mainWin.askToSaveWorkspaceChanges();
                return 1;
            }

        } catch (const std::exception& e) {
            inviwo::util::log(IVW_CONTEXT_CUSTOM("Visual Neuro"), e.what(), inviwo::LogLevel::Error);
            const auto message =
                fmt::format("{}\nApplication state might be corrupted, be warned.", e.what());
            auto res =
                QMessageBox::critical(&mainWin, "Fatal Error", inviwo::utilqt::str(message),
                                      QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
            if (res == QMessageBox::Close) {
                mainWin.askToSaveWorkspaceChanges();
                return 1;
            }
        } catch (...) {
            inviwo::util::log(IVW_CONTEXT_CUSTOM("Visual Neuro"), "Uncaught exception, terminating",
                              inviwo::LogLevel::Error);
            QMessageBox::critical(nullptr, "Fatal Error", "Uncaught exception, terminating");
            return 1;
        }
    }
}
