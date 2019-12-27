#include "shortcutinfoeditor.h"

ShortcutInfoEditor::ShortcutInfoEditor(ShortcutEntry &se, QWidget *parent)
    : MyDialog(parent), se(se), focus_place(-1)
{
    initView();
    initData();
}

ShortcutInfoEditor::ShortcutInfoEditor(ShortcutEntry &se, int focus, QWidget *parent)
    : ShortcutInfoEditor(se, parent)
{
    focus_place = focus;
}

void ShortcutInfoEditor::initView()
{
    cmd_label = new QLabel("命令", this);
    key_label = new QLabel("按键", this);
    env_label = new QLabel("位置", this);
    when_label = new QLabel("条件", this);
    cmd_edit = new QPlainTextEdit(this);
    key_edit = new QLineEdit(this);
    when_edit = new QPlainTextEdit("暂不可用", this);
    env_cb = new QComboBox(this);
    modify_btn = new InteractiveButtonBase("确定", this);

    cmd_edit->setFixedHeight(key_edit->height()*2);
    when_edit->setFixedHeight(key_edit->height()*2);

    QHBoxLayout* cmd_hlayout = new QHBoxLayout;
    cmd_hlayout->addWidget(cmd_label);
    cmd_hlayout->addWidget(cmd_edit);
    QHBoxLayout* key_hlayout = new QHBoxLayout;
    key_hlayout->addWidget(key_label);
    key_hlayout->addWidget(key_edit);
    QHBoxLayout* env_hlayout = new QHBoxLayout;
    env_hlayout->addWidget(env_label);
    env_hlayout->addWidget(env_cb);
    QHBoxLayout* when_hlayout = new QHBoxLayout;
    when_hlayout->addWidget(when_label);
    when_hlayout->addWidget(when_edit);

    addTitleSpacing();
    main_layout->addLayout(cmd_hlayout);
    main_layout->addLayout(key_hlayout);
    main_layout->addLayout(env_hlayout);
    main_layout->addLayout(when_hlayout);
    main_layout->addWidget(modify_btn);

    connect(modify_btn, SIGNAL(clicked()), this, SLOT(slotModifyClicked()));
}

void ShortcutInfoEditor::initData()
{
    // QStringList envs{"全局", "编辑器", "目录", "大纲", "所有编辑器"};
    // env_cb->addItems(envs);
    env_cb->addItem("全局", (int)SCK_GLOBAL);
    env_cb->addItem("编辑器", (int)SCK_EDITOR);
    env_cb->addItem("目录", (int)SCK_DIR);
    env_cb->addItem("大纲", (int)SCK_OUTLINE);
    env_cb->addItem("所有编辑器", (int)SCK_EDITORS);
    env_cb->setCurrentIndex(env_cb->findData(se.env));

    cmd_edit->setPlainText(se.cmds.join("\n"));
    key_edit->setText(se.keys);
}

void ShortcutInfoEditor::setAddModel()
{
    modify_btn->setText("添加");
}

void ShortcutInfoEditor::showEvent(QShowEvent *e)
{
    MyDialog::showEvent(e);
    if (focus_place != -1)
    {
        setFocusPlace(focus_place);
    }
}

void ShortcutInfoEditor::setFocusPlace(int x)
{
    if (x == 0)
    {
        cmd_edit->setFocus();
        cmd_edit->selectAll();
    }
    else if (x == 1)
    {
        key_edit->setFocus();
        key_edit->selectAll();
    }
    else if (x == 2)
    {
        env_cb->setFocus();
        QTimer::singleShot(100, [=]{
            env_cb->showPopup();
        });
    }
    else if (x == 3)
    {
        when_edit->setFocus();
        when_edit->selectAll();
    }
}

void ShortcutInfoEditor::slotModifyClicked()
{
    // 检查能否使用
    if (cmd_edit->toPlainText().isEmpty()) {
        QMessageBox::critical(this, "错误", "命令不能为空");
        return;
    }
    if (key_edit->text().isEmpty()) {
        QMessageBox::critical(this, "错误", "快捷键不能为空");
        return;
    }

    // 保存到Bean中
    se.cmds = cmd_edit->toPlainText().split("\n");
    se.keys = key_edit->text();
    se.env = (ShortcutEnvironment)(env_cb->currentData().toInt());

    this->accept();
}
