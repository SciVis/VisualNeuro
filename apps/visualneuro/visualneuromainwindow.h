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

#pragma once

#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/qt/applicationbase/undomanager.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

#include <tclap/CmdLine.h>

class QDropEvent;
class QDragEnterEvent;
class QTabWidget;

namespace inviwo {


class InviwoApplicationQt;
class TextLabelOverlay;
class ConsoleWidget;

class VisualNeuroMainWindow : public QMainWindow {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    static const unsigned int maxNumRecentFiles_ = 10;

    VisualNeuroMainWindow(InviwoApplication* app);
    virtual ~VisualNeuroMainWindow();

    void showWindow();

    void openLastWorkspace(const std::filesystem::path& workspace = {});
    /**
     * loads the given workspace.
     *
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspace(const std::filesystem::path& workspaceFileName);

    /**
     * loads the given workspace. In case there are unsaved changes, the user will be asked to save
     * or discard them, or cancel the loading.
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspaceAskToSave(const std::filesystem::path& workspaceFileName);
    const std::filesystem::path&  getCurrentWorkspace();

    ConsoleWidget* getConsoleWidget() const;
    InviwoApplication* getInviwoApplication() const;

    /**
     * shows a file dialog for loading a workspace. In case there are unsaved changes, the user will
     * be asked to save or discard them, or cancel the loading.
     *
     * @return true if the workspace was opened, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool openWorkspace();

    /**
     * saves the current workspace. If the workspace does not have a name yet, a file dialog will be
     * shown.
     * @return true if the workspace was saved, otherwise false.
     * @see saveWorkspaceAs
     */
    bool saveWorkspace();
    /**
     * saves the current workspace using a file dialog
     * @return true if the workspace was saved, otherwise false.
     * @see saveWorkspaceAs
     */
    bool saveWorkspaceAs();

    /*
     * Save the current workspace into a new workspace file,
     * leaves the current workspace file as current workspace
     */
    void saveWorkspaceAsCopy();
    bool askToSaveWorkspaceChanges();
    void exitInviwo(bool saveIfModified = true);
    void showAboutBox();


    /**
     * \brief query the Qt settings for recent workspaces
     */
    QStringList getRecentWorkspaceList() const;


    bool hasRestoreWorkspace() const;
    void restoreWorkspace();

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

private:

    /**
     * loads the workspace \p workspaceFileName. In case there are unsaved changes, the user will
     * be asked to save or discard them, or cancel the loading.
     *
     * @param isExample    if true, the workspace file name will not be set. Thereby preventing
     *                     the user from accidentally overwriting the original file. In addition,
     *                     the workspace is _not_ added to the recent file list.
     * @return true if the workspace was opened, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool openWorkspace(const std::filesystem::path& workspaceFileName, bool isExample);
    bool saveWorkspace(const std::filesystem::path& workspaceFileName);

    std::optional<std::filesystem::path> askForWorkspaceToOpen();

    void addActions();

    void closeEvent(QCloseEvent* event) override;

    void saveWindowState();
    void loadWindowState();

    void saveSnapshots(const std::filesystem::path& path, std::string_view fileName);
    void getScreenGrab(const std::filesystem::path& path, std::string_view fileName);

    void addToRecentWorkspaces(const std::filesystem::path& workspaceFileName);

    /**
     * \brief update Qt settings for recent workspaces with internal status
     */
    void saveRecentWorkspaceList(const QStringList& list);
    void setCurrentWorkspace(const std::filesystem::path& workspaceFileName);

    InviwoApplication* app_;
    std::shared_ptr<ConsoleWidget> consoleWidget_;

    std::vector<QAction*> workspaceActionRecent_;
    QAction* clearRecentWorkspaces_;
    QAction* visibilityModeAction_;


    // settings
    bool maximized_;

    // paths
    std::filesystem::path untitledWorkspaceName_;
    std::filesystem::path workspaceFileDir_;
    std::filesystem::path currentWorkspaceFileName_;
    std::filesystem::path workspaceOnLastSuccessfulExit_;

    UndoManager undoManager_;
    struct VisibleWidgets {
        /**
         * store all visible processor and dock widgets before hiding them
         */
        void hide(QMainWindow* win);
        /**
         * show previously hidden processor and dock widgets
         */
        void show();

        std::vector<Processor*> processors;
        std::vector<QDockWidget*> dockwidgets;
    };
    VisibleWidgets visibleWidgetState_;  //!< holds all processor and dock widgets that were visible
                                         //!< before showing the welcome widget
    WorkspaceManager::ClearHandle visibleWidgetsClearHandle_;
};

}  // namespace inviwo

