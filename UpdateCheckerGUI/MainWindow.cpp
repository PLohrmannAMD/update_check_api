//=============================================================================
/// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The main window of a sample GUI used to test the UpdateCheckAPI.
//=============================================================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <cassert>
#include <string>

#include "../../Include/UpdateCheckApi.h"
#include "../../Include/UpdateCheckResultsDialog.h"
#include "../../Include/UpdateCheckThread.h"

const uint32_t MAJOR = 1;
const uint32_t MINOR = 0;
const uint32_t PATCH = 0;
const uint32_t BUILD = 0;

const QString UPDATECHECKURL = ".";
const QString UPDATECHECKASSET = "UpdateCheckerGUI-updates.json";

//-----------------------------------------------------------------------------
/// Main Window
/// Displays widgets to demonstrate different ways to interact with the
/// UpdateCheckAPI using Qt Widgets.
//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* pParent) :
    QMainWindow(pParent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Force the box to be checked at startup.
    // Normally this would come from application settings.
    ui->checkForUpdatesOnStartupCheckBox->setChecked(true);
    ui->checkForUpdatesOnStartupCheckBoxWithNotification->setChecked(true);

    SetupCheckForUpdatesOnStartup();
    SetupCheckForUpdatesOnStartupWithNotification();
    SetupManualCheckForUpdates();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//-----------------------------------------------------------------------------
/// In a simple check for updates on startup functionality, which has minimal
/// impact on a tool's UI, a boolean setting will indicate whether or not the
/// check for updates should occur, and if so, a background thread is spawned to
/// perform the check, and upon completion, a dialog box is displayed to show
/// the results to the user.
///
/// This benefit of this approach is that the dialog will clearly show the user
/// that a new update is available. However, since there is currently no way to
/// ignore a specific update or to set the notification to "Remind me later",
/// this approach could also be an annoyance to the end-user because the dialog
/// would appear every time they start the application.
///
/// The second approach "Check for updates on startup (with notification button)
/// might be a preferred solution for some applications.
//-----------------------------------------------------------------------------
void MainWindow::SetupCheckForUpdatesOnStartup()
{
    // Conditionally check for updates based on the checkbox.
    if (ui->checkForUpdatesOnStartupCheckBox->isChecked())
    {
#ifdef _DEBUG
        UpdateCheck::ThreadController* pBackgroundThread = new UpdateCheck::ThreadController(this, 0, 0, 0, 0);
#else
        UpdateCheck::ThreadController* pBackgroundThread = new UpdateCheck::ThreadController(this, MAJOR, MINOR, PATCH, BUILD);
#endif

        // Display the results when the check for updates has completed.
        // There is not a way in the UI to cancel this thread, so no reason to connect to its CheckForUpdatesCancelled callback.
        bool isCompleteConnected = connect(pBackgroundThread, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &MainWindow::ShowResultsDialog);
        assert(isCompleteConnected);

        pBackgroundThread->StartCheckForUpdates(UPDATECHECKURL, UPDATECHECKASSET);
    }
}

//-----------------------------------------------------------------------------
// An effective and simple integration of the check for updates on startup
// functionality can be done by simply reserving space on the main UI for a
// notification button which is disabled or not displayed by default. A boolean
// setting will indicate whether or not the check for updates should occur, and
// if so, a background thread is spawned to perform the check. Upon completion,
// the notification button can be enabled or made visible, to alert the user
// that newer version is available, without actually interrupting the workflow.
// When the notification button is clicked on, then a dialog box is displayed
// to show more information about the newly available version to the user.
//
// This benefit of this approach is that the user's workflow is not interrupted
// to inform them of the new version, and even if the user purposefully wants
// to wait to download the update, the notification is not intrusive and can
// easily be ignored.
//-----------------------------------------------------------------------------
void MainWindow::SetupCheckForUpdatesOnStartupWithNotification()
{
    ui->newVersionAvailablePushButton->setEnabled(false);

    // Conditionally check for updates based on the checkbox.
    if (ui->checkForUpdatesOnStartupCheckBoxWithNotification->isChecked())
    {
#ifdef _DEBUG
        UpdateCheck::ThreadController* pBackgroundThread = new UpdateCheck::ThreadController(this, 0, 0, 0, 0);
#else
        UpdateCheck::ThreadController* pBackgroundThread = new UpdateCheck::ThreadController(this, MAJOR, MINOR, PATCH, BUILD);
#endif

        // Get notified when the check for updates has completed.
        // There is not a way in the UI to cancel this thread, so no reason to connect to its CheckForUpdatesCancelled callback.
        bool isCompleteConnected = connect(pBackgroundThread, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &MainWindow::NotifyOfNewVersion);
        assert(isCompleteConnected);
        if (!isCompleteConnected)
        {
            delete pBackgroundThread;
            pBackgroundThread = nullptr;
        }
        else
        {
            pBackgroundThread->StartCheckForUpdates(UPDATECHECKURL, UPDATECHECKASSET);
        }
    }
}

//-----------------------------------------------------------------------------
/// This demonstrates how to setup the check for updates to happen manually or
/// on-demand.
///
/// With the manual approach, the user must click a button in the UI to signal
/// the check for updates to occur.
//-----------------------------------------------------------------------------
void MainWindow::SetupManualCheckForUpdates()
{
    bool isButtonConnected = connect(ui->checkForUpdatesPushButton, &QPushButton::clicked, this, &MainWindow::ManualCheckForUpdatesButtonClicked);
    assert(isButtonConnected);
    if (!isButtonConnected)
    {
        // Disable the button since it could not be connected.
        ui->checkForUpdatesPushButton->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
/// This gets signaled as part of the manual check for updates when the user
/// clicks on the button to initiate the check. A background thread is spawned
/// to perform the check, and when the check is complete, another method is
/// signaled to display the results if a new version is available.
///
/// Since this is a manual process, the application may want to display a
/// temporary dialog box to let the user know that the check for updates is in
/// progress, and to allow the user to cancel the action if it is taking longer
/// than expected. This function does NOT demonstrate how to utilize that
/// temporary / in-progress dialog box, nor how to cancel the in-progress check.
//-----------------------------------------------------------------------------
void MainWindow::ManualCheckForUpdatesButtonClicked()
{
#ifdef _DEBUG
    UpdateCheck::ThreadController* pBackgroundThread = new UpdateCheck::ThreadController(this, 0, 0, 0, 0);
#else
    UpdateCheck::ThreadController* pBackgroundThread = new UpdateCheck::ThreadController(this, major, minor, patch, build);
#endif

    bool isCheckCompleteConnected = connect(pBackgroundThread, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &MainWindow::ShowResultsDialog);
    assert(isCheckCompleteConnected);
    if (!isCheckCompleteConnected)
    {
        delete pBackgroundThread;
        pBackgroundThread = nullptr;
    }
    else
    {
        pBackgroundThread->StartCheckForUpdates(UPDATECHECKURL, UPDATECHECKASSET);
    }
}

//-----------------------------------------------------------------------------
/// Slot to Notify the user that a new version is available.
/// \param pThread The background thread controller that signaled this notification.
/// \param updateCheckResults The detailed results of the newly available update.
//-----------------------------------------------------------------------------
void MainWindow::NotifyOfNewVersion(UpdateCheck::ThreadController* pThread, const UpdateCheck::Results& updateCheckResults)
{
    if (updateCheckResults.wasCheckSuccessful && updateCheckResults.updateInfo.m_isUpdateAvailable)
    {
        QString buttonText = "New version available: ";
        buttonText.append(updateCheckResults.updateInfo.m_releases[0].m_title.c_str());
        buttonText.append("\nClick here for more information.");

        ui->newVersionAvailablePushButton->setText(buttonText);

        // Set the button enabled and visible.
        ui->newVersionAvailablePushButton->setEnabled(true);
        ui->newVersionAvailablePushButton->setVisible(true);

        // This dialog will get deleted when the MainWindow is deleted.
        // Since the notification button can get clicked at any time, the dialog is kept alive to be opened as-needed.
        // The dialog cannot be created on-demand when the button is clicked because the updateCheckResults are not
        // readily available.
        UpdateCheckResultsDialog* pResultsDialog = new UpdateCheckResultsDialog(this);
        pResultsDialog->SetResults(updateCheckResults);

        // Connect the button so that the when it is clicked, the dialog is shown.
        connect(ui->newVersionAvailablePushButton, &QPushButton::clicked, pResultsDialog, &QDialog::show);
    }

    // Delete the thread so that it no longer exists in the background.
    if (pThread != nullptr)
    {
        delete pThread;
    }
}

//-----------------------------------------------------------------------------
/// Slot to Notify the user that a new version is available.
/// \param pThread The background thread controller that signaled this notification.
/// \param updateCheckResults The detailed results of the newly available update.
//-----------------------------------------------------------------------------
void MainWindow::ShowResultsDialog(UpdateCheck::ThreadController* pThread, const UpdateCheck::Results& updateCheckResults)
{
    if (updateCheckResults.wasCheckSuccessful && updateCheckResults.updateInfo.m_isUpdateAvailable)
    {
        // This dialog will get deleted when it is closed.
        UpdateCheckResultsDialog* pResultsDialog = new UpdateCheckResultsDialog(this);
        pResultsDialog->setAttribute(Qt::WA_DeleteOnClose);
        pResultsDialog->SetResults(updateCheckResults);

        pResultsDialog->show();
    }

    // Delete the thread so that it no longer exists in the background.
    if (pThread != nullptr)
    {
        delete pThread;
    }
}
