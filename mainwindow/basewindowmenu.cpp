#include "basewindow.h"
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

void BaseWindow::loadMenuFromFile(const QString &filepath)
{
    QString menu_json_str = readTextFile(filepath);

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(menu_json_str.toUtf8(), &json_error); // 转化为JSON文档，UTF-8避免中文乱码
    if (!document.isNull() /*&& (json_error.error == QJsonParseError::NoError)*/) // 解析未发生错误
    {
        if (document.isArray()) // 是菜单分组 数组
        {
            QJsonArray menu_array = document.array();

            for (int i = 0; i < menu_array.size(); i++) // 遍历每个数组元素
            {
                QJsonValue value = menu_array.at(i);
                MenuOrAction moa = getMenuFromJson(value);
                if (moa.isMenu()) menuBar()->addMenu(moa.menu);
                if (moa.isAction())
                {
                    menuBar()->addAction(moa.action);
                }
            }
            QAction* geo_action = new QAction("?", this); // 必须要带一个text，否则actionGeometry不会有变化
            menuBar()->addAction(geo_action);
            menu_bar_right = menuBar()->actionGeometry(geo_action).left();
            menuBar()->removeAction(geo_action);
            geo_action->deleteLater();
        }
    }
    else
    {
        qDebug() << json_error.error << json_error.errorString() << json_error.offset;
    }
}

BaseWindow::MenuOrAction BaseWindow::getMenuFromJson(QJsonValue value)
{
    if (value.isObject()) // return action
    {
        QJsonObject object = value.toObject();
        if (object.contains("menus")) // 添加 Menu
        {
            QMenu* menu = new QMenu(this);
            if (object.contains("name"))
            {
                QJsonValue value = object.value("name");
                if (value.isString())
                    menu->setTitle(value.toString());
            }
            if (object.contains("icon"))
            {
                QJsonValue value = object.value("icon");
                if (value.isString())
                    menu->setIcon(QIcon(value.toString()));
            }
            if (object.contains("menus"))
            {
                QJsonValue value = object.value("menus");
                if (value.isArray())
                    addActionsToMenu(value, menu);
            }
            if (object.contains("enabled"))
            {
                QJsonValue value = object.value("enabled");
                if (value.isBool())
                    menu->setEnabled(value.toBool());
            }
            if (object.contains("condition"))
            {

            }
            if (object.contains("qss"))
            {
                QJsonValue value = object.value("qss");
                if (value.isString())
                    menu->setStyleSheet(value.toString());
            }
            return MenuOrAction(menu);
        }
        else if (object.contains("seperator"))
        {
            return MenuOrAction(1);
        }
        else // 添加 Action
        {
            QAction* action = new QAction(this);

            if (object.contains("name"))
            {
                QJsonValue value = object.value("name");
                if (value.isString())
                    action->setText(value.toString());
            }
            if (object.contains("tooltip"))
            {
                QJsonValue value = object.value("tooltip");
                if (value.isString())
                    action->setToolTip(value.toString());
            }
            if (object.contains("icon"))
            {
                QJsonValue value = object.value("icon");
                if (value.isString())
                    action->setIcon(QIcon(value.toString()));
            }
            if (object.contains("command"))
            {
                QJsonValue value = object.value("command");
                if (value.isString())
                {
                    connect(action, &QAction::triggered, [=]{
                        slotEmitCommand(value.toString());
                    });
                }
            }
            if (object.contains("checked"))
            {
                QJsonValue value = object.value("checked");
                action->setCheckable(true);
                if (value.isBool())
                    action->setChecked(value.toBool());
            }
            if (object.contains("enabled"))
            {
                QJsonValue value = object.value("enabled");
                if (value.isBool())
                    action->setEnabled(value.toBool());
            }
            if (object.contains("condition"))
            {
                QJsonValue value = object.value("condition");
                if (value.isString())
                    processActionCondition(action, value.toString());
            }
            if (object.contains("object_name"))
            {
                QJsonValue value = object.value("object_name");
                if (value.isString())
                    action->setObjectName(value.toString());
            }
            return MenuOrAction(action);
        }
    }
    else
        return MenuOrAction();
}

BaseWindow::MenuOrAction BaseWindow::addActionsToMenu(QJsonValue value, QMenu *parent_menu)
{
    if (value.isArray()) // add actions
    {
        if (parent_menu == nullptr) return MenuOrAction();
        QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); i++)
        {
            QJsonValue value = array.at(i);
            MenuOrAction moa = getMenuFromJson(value);
            if (moa.isMenu())
                parent_menu->addMenu(moa.menu);
            if (moa.isAction())
                parent_menu->addAction(moa.action);
            if ((moa.isSeparator()))
                parent_menu->addSeparator();
        }
        return MenuOrAction(parent_menu);
    }
    else
        return MenuOrAction();
}

void BaseWindow::processActionCondition(QAction* action, QString condition)
{

}
