#include "MainWindow.h"
#include "Defines.h"
#include "Terminal.h"

#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
    myComputer(nullptr)
{
    myUi.setupUi(this);

    myGroup = new QActionGroup(this);
    myGroup->addAction(myUi.actionRun);
    myGroup->addAction(myUi.actionStop);
    myGroup->addAction(myUi.actionPause);

    myGroup->setEnabled(false);
    myUi.actionStop->setChecked(true);
    myUi.actionClose_Program->setEnabled(false);

    myUi.actionStep->setEnabled(false);
    myUi.actionRestart->setEnabled(false);


    for (size_t i = 0; i < 32; i++)
        myUi.twRegisters->setItem(i, 0, new QTableWidgetItem());

    mySpeedCB = new QComboBox(this);
    mySpeedCB->addItem("Max");
    mySpeedCB->addItem("1 ms"); mySpeedCB->setItemData(1, 1);
    mySpeedCB->addItem("10 ms"); mySpeedCB->setItemData(1, 10);
    mySpeedCB->addItem("100 ms"); mySpeedCB->setItemData(1, 100);
    mySpeedCB->addItem("1 s"); mySpeedCB->setItemData(1, 1000);
    myUi.toolBar->addWidget(mySpeedCB);

    myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(timerStep()));
}

MainWindow::~MainWindow()
{
    if (myComputer)
        on_actionClose_Program_triggered(true);
}

void MainWindow::on_actionLoadDisk_triggered(bool checked)
{
    QSettings settings;
    QString lastdir = settings.value("LastOpenDirDisk", "").toString();

    QString file = QFileDialog::getOpenFileName(this, tr("Select disk image to load"), lastdir, "Disk image (*.img);;All files (*.*)");
    if (!file.isEmpty())
    {
        myDisk = file;
        settings.setValue("LastOpenDirDisk", file);
    }
}

void MainWindow::on_actionLoad_Program_triggered(bool checked)
{
    if (myComputer)
        on_actionClose_Program_triggered(true);

    QSettings settings;
    QString lastdir = settings.value("LastOpenDir", "").toString();

    QString file = QFileDialog::getOpenFileName(this, tr("Select program to load"), lastdir, "Program bin (*.bin);;All files (*.*)");
    if (!file.isEmpty())
    {
        myComputer = new ComputerThread(file, myDisk, this);
        myComputer->start();

        myUi.actionRun->setEnabled(true);
        myUi.actionPause->setEnabled(true);
        myUi.actionStep->setEnabled(true);
        myUi.actionRestart->setEnabled(true);
        myGroup->setEnabled(true);

        connect(myComputer, &ComputerThread::stepFinished, this, &MainWindow::stepFinished);
        connect(myComputer, &ComputerThread::paused, this, &MainWindow::programPaused);
        connect(myComputer, &ComputerThread::stoped, this, &MainWindow::programStoped);
        connect(myComputer, &ComputerThread::outputChar, myUi.teTerminal, &Terminal::printchar);
        connect(myUi.teTerminal, &Terminal::keypressed, myComputer, &ComputerThread::keypressed, Qt::DirectConnection);

        settings.setValue("LastOpenDir", file);

        myUi.teTerminal->clear();
    }
}

void MainWindow::on_actionClose_Program_triggered(bool checked)
{
    if (myComputer)
    {
        disconnect(myComputer, &ComputerThread::paused, this, &MainWindow::programPaused);
        disconnect(myComputer, &ComputerThread::stoped, this, &MainWindow::programStoped);
        disconnect(myComputer, &ComputerThread::stepFinished, this, &MainWindow::stepFinished);
        disconnect(myComputer, &ComputerThread::outputChar, myUi.teTerminal, &Terminal::printchar);
        disconnect(myUi.teTerminal, &Terminal::keypressed, myComputer, &ComputerThread::keypressed);
        myComputer->abort();
        myComputer->wait();
        delete myComputer;
        myComputer = nullptr;
    }

    myUi.actionClose_Program->setEnabled(false);
    myUi.actionLoad_Program->setEnabled(true);
    myGroup->setEnabled(false);
    myUi.actionStep->setEnabled(false);
    myUi.actionRestart->setEnabled(false);
}

void MainWindow::on_actionRun_triggered(bool checked)
{
    if (myComputer)
    {
        if (mySpeedCB->currentIndex() == 0)
            myComputer->startProgram();
        else
            myTimer->start(mySpeedCB->currentData().toInt());

        mySpeedCB->setEnabled(false);
    }
}

void MainWindow::on_actionStep_triggered(bool checked)
{
    if (myComputer)
        myComputer->stepProgram();
}

void MainWindow::on_actionStop_triggered(bool checked)
{
    if (myComputer)
        myComputer->abort();
    programStoped();
}

void MainWindow::on_actionPause_triggered(bool checked)
{
    if (myComputer)
        myComputer->pauseProgram();
}

void MainWindow::on_actionRestart_triggered(bool checked)
{
    if (myComputer)
        myComputer->resetProgram();

    myUi.actionRun->setEnabled(true);
    myUi.actionPause->setEnabled(true);
    myUi.actionStep->setEnabled(true);
    myUi.actionRestart->setEnabled(true);
    myGroup->setEnabled(true);
    mySpeedCB->setEnabled(true);

    myUi.teTerminal->clear();
}

void MainWindow::stepFinished(CpuState state)
{
    myUi.lwStack->clear();
    for (auto&& st : state.stack)
        myUi.lwStack->addItem(QStringLiteral("0x%1").arg(st.second, 16, 16, QLatin1Char('0')));

    for (size_t i = 0; i < state.regs.size(); i++)
    {
        auto item = myUi.twRegisters->item(i, 0);
        QString text = item->text();
        QString newtext = QStringLiteral("0x%1").arg(state.regs[i], 16, 16, QLatin1Char('0'));
        item->setText( newtext );
        if (text != newtext)
            item->setForeground(QBrush(QColor(255,0,0)));
        else
            item->setForeground(QBrush(QColor(0, 0, 0)));
    }

    myUi.twRegisters->resizeColumnsToContents();

    QString rdStr = QString::fromStdString(RegisterNames[state.nextstep.rd]);
    QString rs1Str = QString::fromStdString(RegisterNames[state.nextstep.rs1]);
    QString rs2Str = QString::fromStdString(RegisterNames[state.nextstep.rs2]);

    QString instStr = getInstructionName(
        state.nextstep.opcode,
        state.nextstep.funct3,
        state.nextstep.funct7);

    myUi.lePC->setText(QStringLiteral("0x%1 (%2)").arg(state.nextstep.pc, 16, 16, QLatin1Char('0')).arg(instStr));
    myUi.leOpcode->setText(QStringLiteral("0x%1").arg(state.nextstep.opcode, 2, 16, QLatin1Char('0')));
    myUi.leF3->setText(QStringLiteral("0x%1").arg(state.nextstep.funct3, 2, 16, QLatin1Char('0')));
    myUi.leF7->setText(QStringLiteral("0x%1").arg(state.nextstep.funct7, 2, 16, QLatin1Char('0')));
    myUi.leRd->setText(QStringLiteral("0x%1 (%2) [0x%3]").arg(state.nextstep.rd, 2, 16, QLatin1Char('0'))
        .arg(rdStr).arg(QStringLiteral("0x%1").arg(state.regs[state.nextstep.rd], 16, 16, QLatin1Char('0'))));
    myUi.leRs1->setText(QStringLiteral("0x%1 (%2) [0x%3]").arg(state.nextstep.rs1, 2, 16, QLatin1Char('0'))
        .arg(rs1Str).arg(QStringLiteral("0x%1").arg(state.regs[state.nextstep.rs1], 16, 16, QLatin1Char('0'))));
    myUi.leRs2->setText(QStringLiteral("0x%1 (%2) [0x%3]").arg(state.nextstep.rs2, 2, 16, QLatin1Char('0'))
        .arg(rs2Str).arg(QStringLiteral("0x%1").arg(state.regs[state.nextstep.rs2], 16, 16, QLatin1Char('0'))));
}

void MainWindow::programPaused()
{
    myUi.actionPause->setChecked(true);
}

void MainWindow::programStoped()
{
    myUi.actionStop->setChecked(true);
    myUi.actionRun->setEnabled(false);
    myUi.actionPause->setEnabled(false);
    myUi.actionStep->setEnabled(false);
}
