#include "inputdialog.h"

InputDialog::InputDialog(QWidget *parent, QString title, QString filepath, QString tip, QString long_tip) : QDialog(parent),
            title(title), filepath(filepath), tip(tip), long_tip(long_tip), result(nullptr)
{
    initView();
    content = readTextFile(filepath);
    initData();
}

InputDialog::InputDialog(QWidget *parent, QString title, QString content, QString *result, QString tip, QString long_tip) : QDialog(parent),
            title(title), content(content), tip(tip), long_tip(long_tip), result(result)
{
    initView();
    initData();
}

void InputDialog::initView()
{
    setWindowTitle(title);
    tip_label = new MyLabel(tip, this);
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

    if (tip.isEmpty())
        tip_label->hide();

    connect(tip_label, SIGNAL(clicked(QPoint)), this, SLOT(slotTip()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(slotSave()));
}

void InputDialog::initData()
{
    edit->setPlainText(content);
}

void InputDialog::slotTip()
{
    QMessageBox::information(this, title+"-帮助", long_tip);
}

void InputDialog::slotSave()
{
    QString text = edit->toPlainText().trimmed();

    if (!filepath.isEmpty())
        writeTextFile(filepath, text);

    this->close();
}
