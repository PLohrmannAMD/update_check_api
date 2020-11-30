//=============================================================================
/// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The main window of a sample GUI used to test the UpdateCheckAPI.
//=============================================================================
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../../Include/UpdateCheckThread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* pParent = nullptr);
    ~MainWindow();

    void SetupCheckForUpdatesOnStartup();
    void SetupCheckForUpdatesOnStartupWithNotification();
    void SetupManualCheckForUpdates();

private slots:
    void NotifyOfNewVersion(UpdateCheck::ThreadController* pThread, const UpdateCheck::Results& updateCheckResults);
    void ShowResultsDialog(UpdateCheck::ThreadController* pThread, const UpdateCheck::Results& updateCheckResults);
    void ManualCheckForUpdatesButtonClicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
