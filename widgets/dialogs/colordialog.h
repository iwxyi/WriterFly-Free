#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QColorDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollBar>
#include "interactivebuttonbase.h"
#include "waterzoombutton.h"

class ColorDialog : public QDialog
{
	Q_OBJECT
public:
    static QColor pick(QString title, QColor def = Qt::blue, QWidget *parent = nullptr);
    static QColor pick(QString title, QColor def = Qt::blue, bool *ok = nullptr, QWidget *parent = nullptr);
    static QColor pickF(QString title, QColor def = Qt::blue, QWidget *parent = nullptr);
    static QColor pickF(QString title, QColor def = Qt::blue, bool *ok = nullptr, QWidget *parent = nullptr);

protected:
    ColorDialog(QColor def_color, QWidget *parent = nullptr);
    ColorDialog(QColor def_color, bool fore=true, QWidget *parent = nullptr);
    ColorDialog(int index, QColor def_color, QWidget *parent = nullptr);
    ColorDialog(int index, QColor def_color, bool fore=true, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

    virtual bool selectSimilarColor(QColor color);
    QColor getReverseColor(QColor color);
    int getReverseChannel(int x);

private:
    void initView();
    void initSeries();
    void initColorTable();

private:
	void loadColorSeries(QString series);
    void addOneColor(QString name, QColor color);
    void setHexText(QColor color);
    void showSelectAnimation(int x);

signals:
    void signalSeriesLoaded(int x);
    void signalColorPicked(QColor color);
    void signalCanceled();

protected slots:
	void slotLoadColors(int x);
    void slotSwitchForeAndBg();
    void slotListWidgetItemChanged(int row);
    void slotListWidgetClicked(QListWidgetItem* item);
    void slotListWidgetDoubleClicked(QListWidgetItem* item);
    void slotColorPicked(QColor color, bool modifyHex = true);
    void slotHexChanged(const QString &text);

protected:
    int *ctype;
    QColor color;
    QStringList color_series{"常用色系", "色彩表", "深色系", "浅色系", "中性色系", "无彩色系"};
    QString color_table_string;
    QList<QColor> colors;
    bool useForeground;

    QComboBox *series_combo;
    QListWidget *color_list;
    QHBoxLayout *body_hlayout;
    InteractiveButtonBase *fore_btn;
    QLineEdit *hex_line;
    InteractiveButtonBase *cancel_btn;
    InteractiveButtonBase *ok_btn;

    int line_spacing;
    bool _flag_ignore_hex_changed = false;

private:
    bool _flag_ignore_hex_changed_once = false;
    bool _flag_ignore_row_changed_once = false;
};

#endif // COLORDIALOG_H
