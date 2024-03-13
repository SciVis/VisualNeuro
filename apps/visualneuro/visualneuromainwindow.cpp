/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include "visualneuromainwindow.h"
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/licenseinfo.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/processors/exporter.h>
#include <inviwo/qt/applicationbase/qtapptools.h>

#include "consolewidget.h"
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/network/workspaceutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositeprocessorutils.h>

#include <inviwo/core/processors/canvasprocessorwidget.h>

#include <inviwo/core/rendering/datavisualizermanager.h>

#include <fmt/std.h>

#include <QScreen>
#include <QStandardPaths>
#include <QGridLayout>
#include <QActionGroup>
#include <QClipboard>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QUrl>
#include <QVariant>
#include <QToolBar>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QBuffer>
#include <QTabWidget>
#include <QScreen>
#include <QToolButton>
#include <QStackedWidget>
#include <QApplication>

#include <algorithm>

// #define QWEBENGINE_TEST
#ifdef QWEBENGINE_TEST
#include <warn/push>
#include <warn/ignore/all>
#include <QWebEngineView>
#include <modules/openglqt/canvasqt.h>
#include <warn/pop>
#endif

namespace inviwo {

VisualNeuroMainWindow::VisualNeuroMainWindow(InviwoApplication* app)
    : QMainWindow()
    , app_(app)
    , consoleWidget_{[this]() {
        // initialize console widget first to receive log messages
        auto cw = std::make_shared<ConsoleWidget>(this);
        LogCentral::getPtr()->registerLogger(cw);
        return cw;
    }()}
    , maximized_(false)
    , untitledWorkspaceName_{"untitled"}
    , undoManager_(app->getWorkspaceManager(), app->getProcessorNetwork())
    , visibleWidgetsClearHandle_{
          app->getWorkspaceManager()->onClear([&]() { visibleWidgetState_.processors.clear(); })} {

    // Ensure that modules do not add widgets to this window
    setObjectName("VisualNeuroMainWindow");
    setAcceptDrops(true);

    // make sure, tooltips are always shown (this includes port inspectors as well)
    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);

    currentWorkspaceFileName_ = "";

    auto screen = QGuiApplication::primaryScreen();
    const float maxRatio = 0.8f;
    const auto ssize = screen->availableSize();

    QSize size = utilqt::emToPx(this, QSizeF(192, 108));
    size.setWidth(std::min(size.width(), static_cast<int>(ssize.width() * maxRatio)));
    size.setHeight(std::min(size.height(), static_cast<int>(ssize.height() * maxRatio)));

    // Center Window
    QPoint pos{ssize.width() / 2 - ssize.width() / 2, ssize.height() / 2 - ssize.height() / 2};

    resize(size);
    move(pos);

    addDockWidget(Qt::BottomDockWidgetArea, consoleWidget_.get());
    consoleWidget_->setMaximumHeight(utilqt::emToPx(this, QSizeF(192, 15)).height());
    consoleWidget_->setVisible(true);
    consoleWidget_->loadState();

    // load settings and restore window state
    loadWindowState();

    QSettings settings;
    settings.beginGroup(objectName());
    QString firstWorkspace =
        utilqt::toQString(filesystem::getPath(PathType::Workspaces) / "boron.inv");
    workspaceOnLastSuccessfulExit_ =
        utilqt::toPath(settings.value("workspaceOnLastSuccessfulExit", firstWorkspace).toString());
    settings.setValue("workspaceOnLastSuccessfulExit", "");
    settings.endGroup();

    workspaceFileDir_ = filesystem::getPath(PathType::Workspaces);

    // initialize menus
    addActions();

    utilqt::configurePoolResizeWait(*app_, this);
}

VisualNeuroMainWindow::~VisualNeuroMainWindow() { app_->setPoolResizeWaitCallback(nullptr); }

void VisualNeuroMainWindow::showWindow() {
    if (maximized_)
        showMaximized();
    else
        show();
}

void VisualNeuroMainWindow::saveSnapshots(const std::filesystem::path& path,
                                          std::string_view fileName) {
    repaint();
    qApp->processEvents();
    app_->waitForPool();

    while (app_->getProcessorNetwork()->runningBackgroundJobs() > 0) {
        qApp->processEvents();
        app_->processFront();
    }

    rendercontext::activateDefault();
    util::exportAllFiles(
        *app_->getProcessorNetwork(), path, fileName,
        {FileExtension{"png", ""}, FileExtension{"csv", ""}, FileExtension{"txt", ""}},
        Overwrite::Yes);
}

void VisualNeuroMainWindow::getScreenGrab(const std::filesystem::path& path,
                                          std::string_view fileName) {
    repaint();
    qApp->processEvents();
    app_->waitForPool();
    QPixmap screenGrab = QGuiApplication::primaryScreen()->grabWindow(winId());
    screenGrab.save(utilqt::toQString(path / fileName), "png");
}

void VisualNeuroMainWindow::addActions() {
    auto menu = menuBar();

    auto fileMenuItem = menu->addMenu(tr("&File"));
    auto viewMenuItem = menu->addMenu(tr("&View"));

    // file menu entries

    {
        auto openAction = new QAction(QIcon(":/svgicons/open.svg"), tr("&Open Workspace"), this);
        openAction->setShortcut(QKeySequence::Open);
        openAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(openAction);
        connect(openAction, &QAction::triggered, this, [this]() { openWorkspace(); });
        fileMenuItem->addAction(openAction);
    }

    {
        auto saveAction = new QAction(QIcon(":/svgicons/save.svg"), tr("&Save Workspace"), this);
        saveAction->setShortcut(QKeySequence::Save);
        saveAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAction);
        connect(
            saveAction, &QAction::triggered, this,
            static_cast<bool (VisualNeuroMainWindow::*)()>(&VisualNeuroMainWindow::saveWorkspace));
        fileMenuItem->addAction(saveAction);
    }

    {
        auto saveAsAction =
            new QAction(QIcon(":/svgicons/save-as.svg"), tr("&Save Workspace As"), this);
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        saveAsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAsAction);
        connect(saveAsAction, &QAction::triggered, this, &VisualNeuroMainWindow::saveWorkspaceAs);
        fileMenuItem->addAction(saveAsAction);
    }

    {
        auto workspaceActionSaveAsCopy =
            new QAction(QIcon(":/svgicons/save-as-copy.svg"), tr("&Save Workspace As Copy"), this);
        connect(workspaceActionSaveAsCopy, &QAction::triggered, this,
                &VisualNeuroMainWindow::saveWorkspaceAsCopy);
        fileMenuItem->addAction(workspaceActionSaveAsCopy);
    }

    {
        fileMenuItem->addSeparator();
        auto recentWorkspaceMenu = fileMenuItem->addMenu(tr("&Recent Workspaces"));
        // create placeholders for recent workspaces
        workspaceActionRecent_.resize(maxNumRecentFiles_);
        for (auto& action : workspaceActionRecent_) {
            action = new QAction(this);
            action->setVisible(false);
            recentWorkspaceMenu->addAction(action);
            connect(action, &QAction::triggered, this, [this, action]() {
                if (askToSaveWorkspaceChanges()) {
                    openWorkspace(utilqt::toPath(action->data().toString()));
                }
            });
        }
        // action for clearing the recent file menu
        clearRecentWorkspaces_ = recentWorkspaceMenu->addAction("Clear Recent Workspace List");
        clearRecentWorkspaces_->setEnabled(false);
        connect(clearRecentWorkspaces_, &QAction::triggered, this, [this]() {
            for (auto elem : workspaceActionRecent_) {
                elem->setVisible(false);
            }
            // save empty list
            saveRecentWorkspaceList(QStringList());
            clearRecentWorkspaces_->setEnabled(false);
        });

        connect(recentWorkspaceMenu, &QMenu::aboutToShow, this, [this]() {
            for (auto elem : workspaceActionRecent_) {
                elem->setVisible(false);
            }

            QStringList recentFiles{getRecentWorkspaceList()};
            for (int i = 0; i < recentFiles.size(); ++i) {
                if (!recentFiles[i].isEmpty()) {
                    const bool exists = QFileInfo(recentFiles[i]).exists();
                    const auto menuEntry = QString("&%1 %2%3")
                                               .arg(i + 1)
                                               .arg(recentFiles[i])
                                               .arg(exists ? "" : " (missing)");
                    workspaceActionRecent_[i]->setVisible(true);
                    workspaceActionRecent_[i]->setText(menuEntry);
                    workspaceActionRecent_[i]->setEnabled(exists);
                    workspaceActionRecent_[i]->setData(recentFiles[i]);
                }
            }
            clearRecentWorkspaces_->setEnabled(!recentFiles.isEmpty());
        });
    }

#ifdef IVW_DEBUG
    {
        fileMenuItem->addSeparator();
        auto reloadStyle = fileMenuItem->addAction("Reload Style sheet");
        connect(reloadStyle, &QAction::triggered, [this](bool /*state*/) {
            // The following code snippet allows to reload the Qt style sheets during
            // runtime, which is handy while we change them.
            app_->setStyleSheetFile(QString::fromStdString(app_->getPath(PathType::Resources) +
                                                           "/stylesheets/visualneuro.qss"));
        });
    }
#endif

    {
        fileMenuItem->addSeparator();
        auto exitAction = new QAction(QIcon(":/svgicons/exit.svg"), tr("&Exit"), this);
        exitAction->setShortcut(QKeySequence::Close);
        connect(exitAction, &QAction::triggered, this, &VisualNeuroMainWindow::close);
        fileMenuItem->addAction(exitAction);
    }

    // View
    {
        // dock widget visibility menu entries
        consoleWidget_->toggleViewAction()->setText(tr("&Output Console"));
        viewMenuItem->addAction(consoleWidget_->toggleViewAction());
    }
}

void VisualNeuroMainWindow::addToRecentWorkspaces(const std::filesystem::path& workspaceFileName) {
    QStringList recentFiles{getRecentWorkspaceList()};

    recentFiles.removeAll(utilqt::toQString(workspaceFileName));
    recentFiles.prepend(utilqt::toQString(workspaceFileName));

    if (recentFiles.size() > maxNumRecentFiles_) {
        recentFiles.removeLast();
    }
    saveRecentWorkspaceList(recentFiles);
}

QStringList VisualNeuroMainWindow::getRecentWorkspaceList() const {
    QSettings settings;
    settings.beginGroup(objectName());
    QStringList list{settings.value("recentFileList").toStringList()};
    settings.endGroup();

    return list;
}

bool VisualNeuroMainWindow::hasRestoreWorkspace() const { return undoManager_.hasRestore(); }

void VisualNeuroMainWindow::restoreWorkspace() { undoManager_.restore(); }

void VisualNeuroMainWindow::saveRecentWorkspaceList(const QStringList& list) {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("recentFileList", list);
    settings.endGroup();
}

void VisualNeuroMainWindow::setCurrentWorkspace(const std::filesystem::path& workspaceFileName) {
    workspaceFileDir_ = std::filesystem::absolute(workspaceFileName);
    currentWorkspaceFileName_ = workspaceFileName;
}

const std::filesystem::path& VisualNeuroMainWindow::getCurrentWorkspace() {
    return currentWorkspaceFileName_;
}

bool VisualNeuroMainWindow::openWorkspace(const std::filesystem::path& workspaceFileName) {
    return openWorkspace(workspaceFileName, false);
}

bool VisualNeuroMainWindow::openWorkspaceAskToSave(const std::filesystem::path& workspaceFileName) {
    if (askToSaveWorkspaceChanges()) {
        return openWorkspace(workspaceFileName, false);
    } else {
        return false;
    }
}

void VisualNeuroMainWindow::openLastWorkspace(const std::filesystem::path& workspace) {
    QSettings settings;
    settings.beginGroup(objectName());
    const bool loadlastWorkspace = settings.value("autoloadLastWorkspace", true).toBool();

    const auto loadSuccessful = [&]() {
        if (!workspace.empty()) {
            return openWorkspace(workspace);
        } else if (loadlastWorkspace && !workspaceOnLastSuccessfulExit_.empty()) {
            return openWorkspace(workspaceOnLastSuccessfulExit_);
        }
        return false;
    }();

    if (!loadSuccessful) {
        QMessageBox msgBox(this);
        msgBox.setText(utilqt::toQString(fmt::format("Failed to open {}", workspace)));
        msgBox.setInformativeText("Visual Neuro will close");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        // set minimum size to suppress Qt warning
        // see bug report https://bugreports.qt.io/browse/QTBUG-63661
        msgBox.setMinimumSize(msgBox.minimumSizeHint());
        msgBox.exec();
        exitInviwo(false);
    }
}

std::optional<std::filesystem::path> VisualNeuroMainWindow::askForWorkspaceToOpen() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");
        openFileDialog.addSidebarPath(PathType::Workspaces);
        openFileDialog.addSidebarPath(workspaceFileDir_);
        openFileDialog.addExtension("inv", "Inviwo File");
        openFileDialog.setFileMode(FileMode::AnyFile);

        if (openFileDialog.exec()) {
            QString path = openFileDialog.selectedFiles().at(0);
            return utilqt::toPath(path);
        }
    }
    return std::nullopt;
}

bool VisualNeuroMainWindow::openWorkspace() {
    if (auto path = askForWorkspaceToOpen()) {
        return openWorkspace(*path);
    }
    return false;
}

bool VisualNeuroMainWindow::openWorkspace(const std::filesystem::path& fileName, bool isExample) {
    if (!std::filesystem::is_regular_file(fileName)) {
        LogError("Could not find workspace file: " << fileName);
        return false;
    }

    {
        NetworkLock lock(app_->getProcessorNetwork());
        app_->getWorkspaceManager()->clear();
        try {
            app_->getWorkspaceManager()->load(fileName, [&](ExceptionContext ec) {
                try {
                    throw;
                } catch (const IgnoreException& e) {
                    util::logError(e.getContext(), "Incomplete network loading {} due to {}",
                                   fileName, e.getMessage());
                }
            });

            if (isExample) {
                setCurrentWorkspace(untitledWorkspaceName_);
            } else {
                setCurrentWorkspace(fileName);
                addToRecentWorkspaces(fileName);
            }
            std::vector<Processor*> processors = app_->getProcessorNetwork()->getProcessors();

            QWidget* mainCanvasWidget = NULL;

            for (std::vector<Processor*>::iterator it = processors.begin(); it != processors.end();
                 ++it) {

                Processor* processor = *it;
                processor->invalidate(InvalidationLevel::InvalidResources);

                auto processorWidget =
                    dynamic_cast<CanvasProcessorWidget*>(processor->getProcessorWidget());
                if (processorWidget) {
                    // Assume that the main canvas is called Canvas. Otherwise just use the first
                    // one that comes up.
                    if (processor->getIdentifier().compare("Canvas") == 0 ||
                        mainCanvasWidget == NULL) {
                        mainCanvasWidget = dynamic_cast<QWidget*>(processorWidget);
                    }
                }
            }

            // Notify that the network has been reloaded so that GUI can be updated
            if (mainCanvasWidget) {
                // FIXME: Workaround to get QGLWidget to show correctly under fullscreen
                // https://bugreports.qt.io/browse/QTBUG-7556
                // Should only be
                // setCentralWidget(mainCanvasWidget);

                QWidget* const central = new QWidget;

                QWidget* const dw = mainCanvasWidget;

                QLayout* centralLayout = 0;

                QWidget* const dummyRight = new QWidget;
                dummyRight->setMinimumWidth(1);

                QWidget* const dummyBottom = new QWidget;
                dummyBottom->setMinimumHeight(1);

                QGridLayout* const gridLayout = new QGridLayout(central);
                gridLayout->addWidget(dummyRight, 0, 1);
                gridLayout->addWidget(dummyBottom, 1, 0);
                gridLayout->setRowStretch(0, 100);
                gridLayout->setColumnStretch(0, 100);
                gridLayout->addWidget(dw, 0, 0);

                centralLayout = gridLayout;

                centralLayout->setSpacing(0);
                centralLayout->setContentsMargins(0, 0, 0, 0);

                setCentralWidget(central);

                // END OF FIXME
                mainCanvasWidget->show();

                // Will make the central widget a parent to the overlay
                // mainWindow_->onNetworkLoaded();
                // By processing the events here we get rid of an error:
                // "attempt to set a screen on a child window"
                // QApplication::processEvents();
            }
        } catch (const Exception& e) {
            util::logError(e.getContext(), "Unable to load network {} due to {}", fileName,
                           e.getMessage());
            app_->getWorkspaceManager()->clear();
            setCurrentWorkspace(untitledWorkspaceName_);
        }
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);  // make sure the gui is ready
                                                                  // before we unlock.
    }
    saveWindowState();
    return true;
}

bool VisualNeuroMainWindow::saveWorkspace(const std::filesystem::path& fileName) {
    util::OnScopeExit onExit(nullptr);

    try {
        app_->getWorkspaceManager()->save(fileName, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::logError(e.getContext(), "Incomplete network save {} due to {}", fileName,
                               e.getMessage());
            }
        });
        LogInfo("Workspace saved to: " << fileName);
        return true;
    } catch (const Exception& e) {
        util::logError(e.getContext(), "Unable to save network {} due to {}", fileName,
                       e.getMessage());
    }
    return false;
}

bool VisualNeuroMainWindow::saveWorkspace() {
    if (currentWorkspaceFileName_ == untitledWorkspaceName_)
        return saveWorkspaceAs();
    else {
        return saveWorkspace(currentWorkspaceFileName_);
    }
}

bool VisualNeuroMainWindow::saveWorkspaceAs() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    bool savedWorkspace = false;
    if (saveFileDialog.exec()) {
        std::filesystem::path path = utilqt::toPath(saveFileDialog.selectedFiles().at(0));
        if (path.extension() != ".inv") path += ".inv";

        saveWorkspace(path);
        setCurrentWorkspace(path);
        addToRecentWorkspaces(path);
        savedWorkspace = true;
    }
    return savedWorkspace;
}

void VisualNeuroMainWindow::saveWorkspaceAsCopy() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        std::filesystem::path path = utilqt::toPath(saveFileDialog.selectedFiles().at(0));
        if (path.extension() != ".inv") path.append(".inv");

        saveWorkspace(path);
        addToRecentWorkspaces(path);
    }
}

void VisualNeuroMainWindow::exitInviwo(bool /*saveIfModified*/) {
    /// if (!saveIfModified) getNetworkEditor()->setModified(false);
    QMainWindow::close();
    qApp->exit();
}

void VisualNeuroMainWindow::saveWindowState() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void VisualNeuroMainWindow::loadWindowState() {
    QSettings settings;
    settings.beginGroup(objectName());
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state", saveState()).toByteArray());
    maximized_ = settings.value("maximized", false).toBool();
    settings.endGroup();
}

void VisualNeuroMainWindow::closeEvent(QCloseEvent* event) {
    if (!askToSaveWorkspaceChanges()) {
        event->ignore();
        return;
    }

    app_->getWorkspaceManager()->clear();
    // clear dangling pointers to processors with processor widgets
    visibleWidgetState_.processors.clear();

    saveWindowState();

    QSettings settings;
    settings.beginGroup(objectName());
    if (currentWorkspaceFileName_ == untitledWorkspaceName_) {
        settings.setValue("workspaceOnLastSuccessfulExit", "");
    } else {
        settings.setValue("workspaceOnLastSuccessfulExit",
                          utilqt::toQString(currentWorkspaceFileName_));
    }
    settings.endGroup();

    // loop over all children and trigger close events _before_ the main window is closed and
    // destroyed. This way, the main window can stay open if a child widget does not close, i.e.
    // calls event->ignore().
    // @see QWidget::closeEvent()
    for (auto& child : children()) {
        QApplication::sendEvent(child, event);
        if (!event->isAccepted()) {
            return;
        }
    }

    QMainWindow::closeEvent(event);
}

bool VisualNeuroMainWindow::askToSaveWorkspaceChanges() {
    bool continueOperation = true;
    /*
        if (getNetworkEditor()->isModified() && !app_->getProcessorNetwork()->isEmpty()) {
            QMessageBox msgBox(this);
            msgBox.setText("Workspace Modified");
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Yes);
            // set minimum size to suppress Qt warning
            // see bug report https://bugreports.qt.io/browse/QTBUG-63661
            msgBox.setMinimumSize(msgBox.minimumSizeHint());
            int answer = msgBox.exec();

            switch (answer) {
                case QMessageBox::Yes:
                    saveWorkspace();
                    break;

                case QMessageBox::No:
                    break;

                case QMessageBox::Cancel:
                    continueOperation = false;
                    break;
            }
        }
    */
    return continueOperation;
}

InviwoApplication* VisualNeuroMainWindow::getInviwoApplication() const { return app_; }

void VisualNeuroMainWindow::dragEnterEvent(QDragEnterEvent* event) { dragMoveEvent(event); }

void VisualNeuroMainWindow::dragMoveEvent(QDragMoveEvent* event) {
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        std::filesystem::path filename = utilqt::toPath(urlList.front().toLocalFile());
        auto ext = toLower(filename.extension().string());

        if (ext == ".inv" ||
            !app_->getDataVisualizerManager()->getDataVisualizersForFile(filename).empty()) {

            if (event->modifiers() & Qt::ControlModifier) {
                event->setDropAction(Qt::CopyAction);
            } else {
                event->setDropAction(Qt::MoveAction);
            }
            event->accept();
        } else {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void VisualNeuroMainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        // use dispatch front here to avoid blocking the drag&drop source, e.g. Windows
        // Explorer, while the drop operation is performed
        auto action = [this, keyModifiers = event->modifiers(), urlList = mimeData->urls()]() {
            RenderContext::getPtr()->activateDefaultRenderContext();

            bool first = true;

            for (auto& file : urlList) {
                std::filesystem::path filename = utilqt::toPath(file.toLocalFile());

                if (toLower(filename.extension().string()) == ".inv") {
                    openWorkspaceAskToSave(filename);
                } else {
                }
                first = false;
            }
            saveWindowState();
            undoManager_.pushStateIfDirty();
        };
        app_->dispatchFrontAndForget(action);
        event->accept();
    } else {
        event->ignore();
    }
}

}  // namespace inviwo
