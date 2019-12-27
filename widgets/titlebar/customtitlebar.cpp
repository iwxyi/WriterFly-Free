#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QMouseEvent>
#include "customtitlebar.h"

#define BUTTON_HEIGHT 30 // 按钮高度
#define BUTTON_WIDTH 30  // 按钮宽度
#define TITLE_HEIGHT 30  // 标题栏高度

CustomTitleBar::CustomTitleBar(QWidget *parent)
    : QWidget(parent), m_colorR(255), m_colorG(250), m_colorB(250), m_isPressed(false),
      m_buttonType(MIN_MAX_BUTTON), m_windowBorderWidth(0), m_isTransparent(false)
{
    initControl();
    initConnections();
    loadStyleSheet("CustomTitleBar"); // 加载本地样式 CustomTitleBar.css 文件
}

CustomTitleBar::~CustomTitleBar()
{

}

void CustomTitleBar::initControl()
{
    m_pIcon = new QLabel;
    m_pTitleContent = new QLabel;

    QPixmap* pixmap_min, *pixmap_restore, *pixmap_max, *pixmap_close;
    pixmap_min = new QPixmap(32, 32);
    pixmap_min->load("./image/min.png");
    pixmap_restore = new QPixmap(32, 32);
    pixmap_restore->load("./image/max.png");
    pixmap_max = new QPixmap(32, 32);
    pixmap_max->load("./image/max.png");
    pixmap_close = new QPixmap(32, 32);
    pixmap_close->load("./image/close.png");
    QIcon *icon_min = new QIcon(*pixmap_min), *icon_restore = new QIcon(*pixmap_restore),
            *icon_max = new QIcon(*pixmap_max), *icon_close = new QIcon(*pixmap_close);

    m_pButtonMin = new QPushButton(*icon_min, "");
    m_pButtonRestore = new QPushButton(*icon_restore, "");
    m_pButtonMax = new QPushButton(*icon_max, "");
    m_pButtonClose = new QPushButton(*icon_close, "");

    m_pButtonMin->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_pButtonRestore->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_pButtonMax->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_pButtonClose->setFixedSize(QSize(BUTTON_WIDTH, BUTTON_HEIGHT));

    m_pTitleContent->setObjectName("TitleContent");
    m_pButtonMin->setObjectName("ButtonMin");
    m_pButtonRestore->setObjectName("ButtonRestore");
    m_pButtonMax->setObjectName("ButtonMax");
    m_pButtonClose->setObjectName("ButtonClose");

    m_pButtonMin->setToolTip(QStringLiteral("最小化"));
    m_pButtonRestore->setToolTip(QStringLiteral("还原"));
    m_pButtonMax->setToolTip(QStringLiteral("最大化"));
    m_pButtonClose->setToolTip(QStringLiteral("关闭"));

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addWidget(m_pIcon);
    hlayout->addWidget(m_pTitleContent);

    hlayout->addWidget(m_pButtonMin);
    hlayout->addWidget(m_pButtonRestore);
    hlayout->addWidget(m_pButtonMax);
    hlayout->addWidget(m_pButtonClose);

    hlayout->setContentsMargins(5, 0, 0, 0);
    hlayout->setSpacing(0);

    m_pTitleContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setFixedHeight(TITLE_HEIGHT);
    this->setWindowFlags(Qt::FramelessWindowHint);

    QString styleSheet = QString("QPushButton {background-color: transparent; border: none; } QPushButton:hover {background-color: rgb(220, 220, 220); }");
    m_pButtonMin->setStyleSheet(styleSheet);
    m_pButtonRestore->setStyleSheet(styleSheet);
    m_pButtonMax->setStyleSheet(styleSheet);
    m_pButtonClose->setStyleSheet(styleSheet);
}

void CustomTitleBar::initConnections()
{
    connect(m_pButtonMin, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
    connect(m_pButtonRestore, SIGNAL(clicked()), this, SLOT(onButtonRestoreClicked()));
    connect(m_pButtonMax, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
    connect(m_pButtonClose, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
}

void CustomTitleBar::setBackgroundColor(int r, int g, int b, bool isTransparent)
{
    m_colorR = r;
    m_colorG = g;
    m_colorB = b;
    m_isTransparent = isTransparent;
    update(); // 重新绘制（调用paintEvent事件）
}

void CustomTitleBar::setTitleIcon(QString filePath, QSize IconSize)
{
    QPixmap titleIcon(filePath);
    m_pIcon->setPixmap(titleIcon.scaled(IconSize));
}

void CustomTitleBar::setTitleContent(QString titleContent, int titleFontSize)
{
    // 设置标题字体大小;
    QFont font = m_pTitleContent->font();
    font.setPointSize(titleFontSize);
    m_pTitleContent->setFont(font);
    // 设置标题内容;
    m_pTitleContent->setText(titleContent);
    m_titleContent = titleContent;
}

void CustomTitleBar::setTitleWidth(int width)
{
    this->setFixedWidth(width);
}

void CustomTitleBar::setButtonType(ButtonType buttonType)
{
    m_buttonType = buttonType;

    switch (buttonType)
    {
    case MIN_BUTTON :
        m_pButtonRestore->setVisible(false);
        m_pButtonMax->setVisible(false);
        break;
    case MIN_MAX_BUTTON :
        m_pButtonRestore->setVisible(false);
        break;
    case ONLY_CLOSE_BUTTON :
        m_pButtonMin->setVisible(false);
        m_pButtonRestore->setVisible(false);
        m_pButtonMax->setVisible(false);
        break;
    default :
        break;
    }
}

void CustomTitleBar::setTitleRoll()
{
    connect(&m_titleRollTimer, SIGNAL(timeout()), this, SLOT(onRollTitle()));
    m_titleRollTimer.start(200);
}

void CustomTitleBar::setWindowBorderWidth(int borderWidth)
{
    m_windowBorderWidth = borderWidth;
}

void CustomTitleBar::saveRestoreInfo(const QPoint point, const QSize size)
{
    m_restorePos = point;
    m_restoreSize = size;
}

void CustomTitleBar::getRestoreInfo(QPoint &point, QSize &size)
{
    point = m_restorePos;
    size = m_restoreSize;
}

void CustomTitleBar::paintEvent(QPaintEvent *event)
{
    // 是否设置标题透明;
    if (!m_isTransparent)
    {
        //设置背景色;
        QPainter painter(this);
        QPainterPath pathBack;
        pathBack.setFillRule(Qt::WindingFill);
        pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(pathBack, QBrush(QColor(m_colorR, m_colorG, m_colorB)));
    }

    // 当窗口最大化或者还原后，窗口长度变了，标题栏的长度应当一起改变;
    // 这里减去m_windowBorderWidth ，是因为窗口可能设置了不同宽度的边框;
    // 如果窗口有边框则需要设置m_windowBorderWidth的值，否则m_windowBorderWidth默认为0;
    if (this->width() != (this->parentWidget()->width() - m_windowBorderWidth))
    {
        this->setFixedWidth(this->parentWidget()->width() - m_windowBorderWidth);
    }
    QWidget::paintEvent(event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 只有存在最大化、还原按钮时双击才有效;
    if (m_buttonType == MIN_MAX_BUTTON)
    {
        // 通过最大化按钮的状态判断当前窗口是处于最大化还是原始大小状态;
        // 或者通过单独设置变量来表示当前窗口状态;
        if (m_pButtonMax->isVisible())
        {
            onButtonMaxClicked();
        }
        else
        {
            onButtonRestoreClicked();
        }
    }

    return QWidget::mouseDoubleClickEvent(event);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (m_buttonType == MIN_MAX_BUTTON && !m_pButtonMax->isVisible())
    {
        // 在窗口最大化时禁止拖动窗口;
    }
    else
    {
        m_isPressed = true;
        m_startMovePos = event->globalPos();
    }

    return QWidget::mousePressEvent(event);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPressed)
    {
        QPoint movePoint = event->globalPos() - m_startMovePos;
        QPoint widgetPos = this->parentWidget()->pos();
        this->parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
        m_startMovePos = event->globalPos();
    }
    return QWidget::mouseMoveEvent(event);
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_isPressed = false;
    return QWidget::mouseReleaseEvent(event);
}

void CustomTitleBar::loadStyleSheet(const QString &sheetName)
{
    QFile file("./stylesheet/" + sheetName + ".css");
    file.open(QFile::ReadOnly);
    if (file.isOpen())
    {
        QString styleSheet = this->styleSheet();
        styleSheet += QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }
}

void CustomTitleBar::onButtonMinClicked()
{
    emit signalButtonMinClicked();
}

void CustomTitleBar::onButtonRestoreClicked()
{
    m_pButtonRestore->setVisible(false);
    m_pButtonMax->setVisible(true);
    emit signalButtonRestoreClicked();
}

void CustomTitleBar::onButtonMaxClicked()
{
    m_pButtonMax->setVisible(false);
    m_pButtonRestore->setVisible(true);
    emit signalButtonMaxClicked();
}

void CustomTitleBar::onButtonCloseClicked()
{
    emit signalButtonCloseClicked();
}

void CustomTitleBar::onRollTitle()
{
    static int nPos = 0;
    QString titleContent = m_titleContent;
    // 当截取的位置比字符串长时，从头开始;
    if (nPos > titleContent.length())
        nPos = 0;

    m_pTitleContent->setText(titleContent.mid(nPos));
    nPos++;
}
