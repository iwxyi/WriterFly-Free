#ifndef SQUAREBUTTON_H
#define SQUAREBUTTON_H

#include <QObject>
#include "circlebutton.h"

class SquareButton : public CircleButton
{
    Q_OBJECT
public:
    SquareButton(QString icon, QWidget* parent);

};

#endif // SQUAREBUTTON_H
