#pragma once

#include "ComputerThread.h"

#include "ui_MainWindow.h"

#include <QMainWindow>
#include <QActionGroup>
#include <QComboBox>
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~MainWindow();

protected slots:
    void on_actionLoadDisk_triggered(bool checked);
    void on_actionLoad_Program_triggered(bool checked);
    void on_actionClose_Program_triggered(bool checked);
    void on_actionRun_triggered(bool checked);
    void on_actionStep_triggered(bool checked);
    void on_actionStop_triggered(bool checked);
    void on_actionPause_triggered(bool checked);
    void on_actionRestart_triggered(bool checked);

    void stepFinished(CpuState state);
    void programPaused();
    void programStoped();

    void timerStep() { on_actionStep_triggered(true); }

protected:
    Ui::MainWindow myUi;
    ComputerThread* myComputer;
    QActionGroup *myGroup;
    QComboBox* mySpeedCB;
    QTimer* myTimer;

    QString myDisk;
};