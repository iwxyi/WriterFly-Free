#ifndef MOTIONCONTENTNUMBER_H
#define MOTIONCONTENTNUMBER_H

#include <QObject>
#include <QLineEdit>
#include <QHBoxLayout>
#include "motionboard.h"

class MotionContentNumber : public MotionBoard
{
	Q_OBJECT
public:
    MotionContentNumber(QTextEdit* edit);

protected:
    void initAvaliableMotion() override;
    bool isChildWidget(QWidget* widget) override;

private:
    QString getNumber(const QString& key, int& number, const QString& text);

public slots:
    void slotInputted(const QString &text);

protected:
    QLineEdit *input_edit;
};

#endif // MOTIONCONTENTNUMBER_H
