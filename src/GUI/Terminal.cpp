#include "Terminal.h"

#include <QKeyEvent>

Terminal::Terminal(QWidget* parent) : QTextEdit(parent)
{
}

Terminal::~Terminal()
{

}

void Terminal::keyPressEvent(QKeyEvent* e)
{
	emit keypressed((char)e->key());
}

void Terminal::printchar(char c)
{
	moveCursor(QTextCursor::End);
	insertPlainText(QString(c));
	moveCursor(QTextCursor::End);
}