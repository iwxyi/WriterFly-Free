#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

enum ButtonType {
	MIN_BUTTON = 0,
	MIN_MAX_BUTTON,
	ONLY_CLOSE_BUTTON
};

/**
 * 自定义窗口标题栏控件[模板]
 */
class CustomTitleBar : public QWidget
{
	Q_OBJECT
public:
	CustomTitleBar(QWidget* parent);
	~CustomTitleBar();
	
	void setBackgroundColor(int r, int g, int b, bool isTransparent = false); // 设置标题栏背景色及是否设置标题栏背景色透明
	void setTitleIcon(QString filePath, QSize IconSize = QSize(25, 25));      // 设置标题栏图标
	void setTitleContent(QString titleContent, int titleFontSize = 9);        // 设置标题内容
	void setTitleWidth(int width);                                            // 设置标题栏宽度
	void setButtonType(ButtonType buttonType);                                // 设置标题栏上按钮类型
	void setTitleRoll();                                                      // 设置标题栏中的标题是否会滚动
	void setWindowBorderWidth(int borderWidth);                               // 设置窗口边框宽度

        void saveRestoreInfo(const QPoint point, const QSize size); // 保存/获取 最大化前窗口的位置和大小
	void getRestoreInfo(QPoint& point, QSize& size);

private:
	void paintEvent(QPaintEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

	void initControl();                            // 初始化控件
	void initConnections();                        // 信号槽的绑定
	void loadStyleSheet(const QString& sheetName); // 加载样式文件

signals:
	// 按钮触发的信号
	void signalButtonMinClicked();
    void signalButtonRestoreClicked();
	void signalButtonMaxClicked();
	void signalButtonCloseClicked();

private slots:
	// 按钮触发的槽
	void onButtonMinClicked();
	void onButtonRestoreClicked();
	void onButtonMaxClicked();
	void onButtonCloseClicked();
	void onRollTitle();

private:
	QLabel* m_pIcon;                  // 标题栏图标
	QLabel* m_pTitleContent;          // 标题栏内容
	QPushButton* m_pButtonMin;        // 最小化按钮
	QPushButton* m_pButtonRestore;    // 最大化还原按钮
	QPushButton* m_pButtonMax;        // 最大化按钮
	QPushButton* m_pButtonClose;      // 关闭按钮

	int m_colorR, m_colorG, m_colorB; // 标题栏背景色

    QPoint m_restorePos;               // 最大化，最小化变量
    QSize m_restoreSize;

	bool m_isPressed;                 // 移动窗口的变量
	QPoint m_startMovePos;

	QTimer m_titleRollTimer;          // 标题栏跑马灯效果时钟
	QString m_titleContent;           // 标题栏内容
	ButtonType m_buttonType;          // 按钮类型
	int m_windowBorderWidth;          // 窗口边框宽度
	bool m_isTransparent;             // 标题栏是否透明
};

#endif // CUSTOMTITLEBAR_H
