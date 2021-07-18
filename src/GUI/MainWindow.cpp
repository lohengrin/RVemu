#include "MainWindow.h"

#include <QFileDialog>

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
}

MainWindow::~MainWindow()
{
    if (myComputer)
        on_actionClose_Program_triggered(true);
}


void MainWindow::on_actionLoad_Program_triggered(bool checked)
{
    if (myComputer)
        on_actionClose_Program_triggered(true);

    QString file = QFileDialog::getOpenFileName(this, tr("Select program to load"), QString(), "Program bin (*.bin);;All files (*.*)");
    if (!file.isEmpty())
    {
        myComputer = new ComputerThread(file, this);
        myComputer->start();

        myUi.actionRun->setEnabled(true);
        myUi.actionPause->setEnabled(true);
        myUi.actionStep->setEnabled(true);
        myUi.actionRestart->setEnabled(true);
        myGroup->setEnabled(true);

        connect(myComputer, &ComputerThread::stepFinished, this, &MainWindow::stepFinished);
        connect(myComputer, &ComputerThread::paused, this, &MainWindow::programPaused);
        connect(myComputer, &ComputerThread::stoped, this, &MainWindow::programStoped);
    }
}

void MainWindow::on_actionClose_Program_triggered(bool checked)
{
    if (myComputer)
    {
        disconnect(myComputer, &ComputerThread::paused, this, &MainWindow::programPaused);
        disconnect(myComputer, &ComputerThread::stoped, this, &MainWindow::programStoped);
        disconnect(myComputer, &ComputerThread::stepFinished, this, &MainWindow::stepFinished);
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
        myComputer->startProgram();
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
}

void MainWindow::stepFinished(CpuState state)
{
    myUi.lwStack->clear();
    for (auto&& st : state.stack)
        myUi.lwStack->addItem(QStringLiteral("0x%1").arg(st.second, 16, 16, QLatin1Char('0')));

    for (size_t i = 0; i < state.regs.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(QStringLiteral("0x%1").arg(state.regs[i], 16, 16, QLatin1Char('0')));
        myUi.twRegisters->setItem(i,0,item);
    }

    myUi.twRegisters->resizeColumnsToContents();

    myUi.lePC->setText(QStringLiteral("0x%1").arg(state.nextstep.pc, 16, 16, QLatin1Char('0')));
    myUi.leOpcode->setText(QStringLiteral("0x%1").arg(state.nextstep.opcode, 2, 16, QLatin1Char('0')));
    myUi.leF3->setText(QStringLiteral("0x%1").arg(state.nextstep.funct3, 2, 16, QLatin1Char('0')));
    myUi.leF7->setText(QStringLiteral("0x%1").arg(state.nextstep.funct7, 2, 16, QLatin1Char('0')));
    myUi.leRd->setText(QStringLiteral("0x%1").arg(state.nextstep.rd, 2, 16, QLatin1Char('0')));
    myUi.leRs1->setText(QStringLiteral("0x%1").arg(state.nextstep.rs1, 2, 16, QLatin1Char('0')));
    myUi.leRs2->setText(QStringLiteral("0x%1").arg(state.nextstep.rs2, 2, 16, QLatin1Char('0')));
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
