#include "homonymcoverwindow.h"

HomonymCoverWindow::HomonymCoverWindow(QWidget *parent) : QDialog(parent)
{
    initView();
    initData();
}

void HomonymCoverWindow::initView()
{
    setWindowTitle("同音词覆盖黑名单");
    tip_label = new MyLabel("编辑黑名单（暂时仅限中文）。\n点击此处查看例子。", this);
    edit = new QPlainTextEdit(this);
    save_button = new QPushButton("保存", this);
    QVBoxLayout* main_vlayout = new QVBoxLayout(this);

    main_vlayout->addWidget(tip_label);
    main_vlayout->addWidget(edit);
    main_vlayout->addWidget(save_button);
    main_vlayout->setAlignment(Qt::AlignCenter);
    this->setLayout(main_vlayout);

    this->setMinimumHeight(300);
    this->setMinimumWidth(200);

    connect(tip_label, SIGNAL(clicked(QPoint)), this, SLOT(slotTip()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(slotSave()));
}

void HomonymCoverWindow::initData()
{
    QString text= us->homonym_cover_data.blackToString();
    edit->setPlainText(text);
    edit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}

void HomonymCoverWindow::slotTip()
{
    QString tip = "旧的文字、新输入的文字，只需一个满足黑名单即可\n\n一行一项，以下为一些例子（不包括括号【】）：\n\n";
    tip += "【写作】  只要带“写作”二字就不进行覆盖\n";
    tip += "【^写作$】  恰好是“写作”时二字不覆盖\n";
    tip += "【^写作】  以“写作”开头时不覆盖\n";
    tip += "【.*写作.*】  “写作”在中间时不覆盖\n";
    tip += "\n更多格式请自行学习：正则表达式";

    QMessageBox::information(this, "帮助-同音词覆盖黑名单", tip);
}

void HomonymCoverWindow::slotSave()
{
    QString text = edit->toPlainText().trimmed();
    us->homonym_cover_data.setBlack(text);

    this->close();
}
