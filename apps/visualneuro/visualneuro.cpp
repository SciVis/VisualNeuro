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

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/moduleregistration.h>

#include "splashscreen.h"
#include "visualneuromainwindow.h"

#include <warn/push>
#include <warn/ignore/all>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <warn/pop>



using namespace inviwo;

int main(int argc, char** argv) {
    LogCentral::init();
    inviwo::util::OnScopeExit deleteLogcentral([]() { inviwo::LogCentral::deleteInstance(); });
    auto logger = std::make_shared<inviwo::ConsoleLogger>();
    LogCentral::getPtr()->registerLogger(logger);
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
    QSurfaceFormat surfaceFormat;
    surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
    surfaceFormat.setMajorVersion(10);  // We want latest OpenGL version

    InviwoApplicationQt inviwoApp(argc, argv, "VisualNeuro");
    inviwoApp.setStyleSheetFile(":/stylesheets/inviwo.qss");
    
    inviwoApp.setProgressCallback([](std::string m) {
        LogCentral::getPtr()->log("VisualNeuro", LogLevel::Info, LogAudience::User, "", "", 0, m);
    });
    auto& clp = inviwoApp.getCommandLineParser();

    VisualNeuroMainWindow mainWin(&inviwoApp);
    inviwoApp.printApplicationInfo();

    // initialize and show splash screen
    inviwo::InviwoSplashScreen splashScreen(clp.getShowSplashScreen());
    inviwoApp.setProgressCallback([&splashScreen](std::string s) { splashScreen.showMessage(s); });

    splashScreen.show();
    splashScreen.showMessage("Loading application...");

    // Initialize application and register modules
    splashScreen.showMessage("Initializing modules...");
    inviwoApp.registerModules(inviwo::getModuleList());

    inviwoApp.processEvents();


    // Do this after registerModules if some arguments were added
    clp.parse(inviwo::CommandLineParser::Mode::Normal);

    inviwoApp.processEvents();  // Update GUI
    splashScreen.showMessage("Loading workspace...");
    inviwoApp.processEvents();


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
        inviwoApp.closeInviwoApplication();
        inviwoApp.quit();
        return 0;
    }
    const std::string workspace =
        clp.getLoadWorkspaceFromArg()
            ? clp.getWorkspacePath()
            : brainModule->getPath(ModulePath::Workspaces) + "/visualneuro.inv";

    if (!workspace.empty()) {
        mainWin.openLastWorkspace(workspace);
    }

    inviwoApp.processFront();
    inviwoApp.getProcessorNetwork()->unlock();

    mainWin.showWindow();
    inviwoApp.processEvents();
    splashScreen.finish(&mainWin);

    clp.processCallbacks();  // run any command line callbacks from modules.
    inviwoApp.processEvents();

    if (clp.getQuitApplicationAfterStartup()) {
        inviwoApp.closeInviwoApplication();
        inviwoApp.quit();
        return 0;
    } 
    while (true) {
        try {
            return inviwoApp.exec();
        } catch (const inviwo::Exception& e) {
            {
                std::stringstream ss;
                ss << e.getMessage() << "\n";
                if (!e.getStack().empty()) {
                    ss << "\nStack Trace:\n";
                    e.getStack(ss);
                }
                inviwo::util::log(e.getContext(), ss.str(), inviwo::LogLevel::Error);
            }
            {
                std::stringstream ss;
                e.getFullMessage(ss, 10);
                ss << "\nApplication state might be corrupted, be warned.";
                auto res = QMessageBox::critical(
                    &mainWin, "Fatal Error", QString::fromStdString(ss.str()),
                    QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
                if (res == QMessageBox::Close) {
                    mainWin.askToSaveWorkspaceChanges();
                    return 1;
                }
            }

        } catch (const std::exception& e) {
            LogErrorCustom("Visual Neuro", e.what());
            std::stringstream ss;
            ss << e.what();
            ss << "\nApplication state might be corrupted, be warned.";
            auto res =
                QMessageBox::critical(&mainWin, "Fatal Error", QString::fromStdString(ss.str()),
                                      QMessageBox::Ignore | QMessageBox::Close, QMessageBox::Close);
            if (res == QMessageBox::Close) {
                mainWin.askToSaveWorkspaceChanges();
                return 1;
            }
        } catch (...) {
            LogErrorCustom("Visual Neuro", "Uncaught exception, terminating");
            QMessageBox::critical(nullptr, "Fatal Error", "Uncaught exception, terminating");
            return 1;
        }
    }
}
