#include "customtitlebarwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QFile>

CustomTitleBarWindow::CustomTitleBarWindow(QWidget *parent)
    : QWidget(parent)
{
    // FramelessWindowHint属性设置窗口去除边框;
    // WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口;
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    //setAttribute(Qt::WA_TranslucentBackground); // 设置窗口背景透明

    // setAttribute(Qt::WA_DeleteOnClose); // 关闭窗口时释放资源【主窗口不要设置】
    initTitleBar(); // 初始化标题栏
}

CustomTitleBarWindow::~CustomTitleBarWindow()
{

}

void CustomTitleBarWindow::initTitleBar()
{
    m_titlebar = new CustomTitleBar(this);
    m_titlebar->move(0, 0);

    connect(m_titlebar, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonMinClicked()));
    connect(m_titlebar, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonRestoreClicked()));
    connect(m_titlebar, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonMaxClicked()));
    connect(m_titlebar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));

//    m_titleBar->m_pButtonClose->setStyleSheet(QString("QPushButton {background-image: url(./image/close.png); border: none; background-repeat: no-repeat; min-height: 28px; min-width: 100px; max-width: 100px; max-height: 28px; } QPushButton#m_pButtonClose:hover {background-image: url(:/images/add_shop_data_hover.png); border: none; background-repeat: no-repeat; min-height: 24px; min-width: 81px; max-width: 81px; max-height: 24px; }"));
}

void CustomTitleBarWindow::paintEvent(QPaintEvent* event)
{
    //设置背景色;
    QPainter painter(this);
    QPainterPath path_back;
    path_back.setFillRule(Qt::WindingFill);
    path_back.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(path_back, QBrush(QColor(238, 223, 204)));

    return QWidget::paintEvent(event);
}

void CustomTitleBarWindow::loadStyleSheet(const QString &sheet_name)
{
    QFile file(":/Resources/" + sheet_name + ".css");
    file.open(QFile::ReadOnly);
    if (file.isOpen())
    {
        QString style_sheet = this->styleSheet();
        style_sheet += QLatin1String(file.readAll());
        this->setStyleSheet(style_sheet);
    }
}

void CustomTitleBarWindow::onButtonMinClicked()
{
    showMinimized();
}

void CustomTitleBarWindow::onButtonRestoreClicked()
{
    QPoint window_pos;
    QSize window_size;
    m_titlebar->getRestoreInfo(window_pos, window_size);
    this->setGeometry(QRect(window_pos, window_size));
}

void CustomTitleBarWindow::onButtonMaxClicked()
{
    m_titlebar->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
    QRect desktopRect = QApplication::desktop()->availableGeometry();
    QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
    setGeometry(FactRect);
}

void CustomTitleBarWindow::onButtonCloseClicked()
{
    close();
}
