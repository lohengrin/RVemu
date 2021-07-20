#pragma once

#include <QTextEdit>

class Terminal : public QTextEdit
{
	Q_OBJECT
public:
	Terminal(QWidget* parent = nullptr);
	~Terminal();

public slots:
	void printchar(char c);

signals:
	void keypressed(char c);

protected:
	virtual void 	keyPressEvent(QKeyEvent* e) override;
};