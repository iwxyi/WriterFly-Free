#ifndef MYSPINBOX_H
#define MYSPINBOX_H

#include <QSpinBox>
#include <QColor>
#include <QTimer>

class MySpinBox : public QSpinBox
{
    Q_OBJECT
public:
    MySpinBox(QWidget* parent = nullptr);
    MySpinBox(int min, int max, QWidget* parent = nullptr);

    void initStyle(QColor main_color = Qt::gray, QColor font_color = Qt::black);

protected:
    void focusInEvent(QFocusEvent* e);

public slots:
    void slotValueChanged(int val);
};

#endif // MYSPINBOX_H
