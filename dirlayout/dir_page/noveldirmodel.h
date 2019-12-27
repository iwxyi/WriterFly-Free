#ifndef NOVELDIRMODEL_H
#define NOVELDIRMODEL_H

#include <QAbstractItemModel>
#include <QWidget>
#include <QTimer>
#include "stringutil.h"
#include "noveldirmdata.h"
#include "globalvar.h"
#include "noveltools.h"

/**
 * 小说章节目录的MVD模式中的Model
 */
class NovelDirModel : public QAbstractListModel, public NovelDirMData
{
	Q_OBJECT
public:
    NovelDirModel(const QList<NovelDirItem> &list, QObject *parent = nullptr)
    {
        Q_UNUSED(parent);
        cr_list = list;
        //connect(this, SIGNAL(listChanged()), this, SLOT(refrushModel()));
    }

    NovelDirModel(QObject *parent = nullptr)
    {
        Q_UNUSED(parent);
        //connect(this, SIGNAL(listChanged()), this, SLOT(refrushModel()));
    }

    NovelDirModel(QString novel_name, QObject *parent = nullptr) : NovelDirMData(novel_name)
    {
        Q_UNUSED(parent);
        //connect(this, SIGNAL(listChanged()), this, SLOT(refrushModel()));
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return cr_list.count();
    }

    int getIndexByName(QString name)
    {
        for (int i = 0; i < cr_list.size(); i++)
            if (name == cr_list[i].getName())
                return i;
        return -1;
    }

    QModelIndex getModelIndexByName(QString name)
    {
        for (int i = 0; i < cr_list.size(); i++)
            if (name == cr_list[i].getName())
                return index(i, 0);
        return index(0, 0);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        // 不需要 index 的 data
        if (role == Qt::UserRole+DRole_ROLL_COUNT)
        {
            return roll_subs.size();
        }
        else if (role == Qt::UserRole+DRole_CR_COUNT)
        {
            return cr_list.size();
        }
        else if (role == Qt::UserRole+DRole_ROLL_NAMES)
        {
            return roll_names;
        }

        // 判断 index
        if ((!index.isValid()) || (index.row() >= cr_list.size()))
            return QVariant();
        NovelDirItem& item = cr_list[index.row()]; // 因为使用了 mutable 关键字，所以能在 const 方法中强行修改……

        if (role == Qt::ToolTipRole)
        {
            if (!us->chapter_preview) // 此项功能没有开启
                return "";

            if (item.isRoll()) // 分卷不提供预览
                return "";

            QString novel_name = rt->current_novel;
            QString chapter_name = item.getName();
            if (novel_name.isEmpty() || chapter_name.isEmpty())
                return "";

            chapter_name = fnEncode(chapter_name);
            QString file_path = rt->NOVEL_PATH+novel_name+"/chapters/"+chapter_name+".txt";
            if (isFileExist(file_path))
            {
                QString text = readTextFile(file_path);
                if (text.length() > 110)
                    return text.left(100)+"……(共"+QString::number(NovelTools::getArticleWordCount(text))+"字)";
                if (NovelTools::isAllBlank(text))
                    return "[本章空内容]";
                return text;
            }
            else
            {
                return "[文件不存在]";
            }
        }
        else if (role == Qt::UserRole+DRole_CHP_NAME)
        {
            return item.getName(); // 章节名
        }
        else if (role == Qt::UserRole+DRole_CHP_NUM)
        {
            return item.getNumber(); // 序号
        }
        else if (role == Qt::UserRole+DRole_CHP_NUM_CN)
        {
            return item.getNumber_cn(); // 中文序号
        }
        else if (role == Qt::UserRole+DRole_CHP_ROLLINDEX)
        {
            return item.getRollIndex(); // 分卷序号
        }
        else if (role == Qt::UserRole+DRole_CHP_CHPTINDEX)
        {
            return item.getChptIndex(); // 章节序号
        }
        else if (role == Qt::UserRole+DRole_CHP_DETAIL)
        {
            return item.getDetail(); // 细纲
        }
        else if (role == Qt::UserRole+DRole_CHP_OPENING)
        {
            return item.isOpening(); // 是否编辑中
        }
        else if (role == Qt::UserRole+DRole_CHP_ISROLL)
        {
            return item.isRoll(); // 是否为分卷
        }
        else if (role == Qt::UserRole+DRole_CHP_ISHIDE)
        {
            return item.isHide(); // 是否隐藏
        }
        else if (role == Qt::UserRole+DRole_CHP_STEMP)
        {
            return item.getS_temp(); // 双击编辑，出错后恢复原来的文本
        }
        else if (role == Qt::UserRole+DRole_ROLL_SUBS)
        {
            int rIndex = item.getRollIndex();
            return roll_subs[rIndex]; // 这一章节的分卷章数量
        }
        else if (role == Qt::UserRole+DRole_ROLL_COUNT)
        {
            return roll_subs.size(); // 分卷的数量
        }
        else if (role == Qt::UserRole+DRole_CR_COUNT)
        {
            return cr_list.size(); // 所有加起来的数量
        }
        else if (role == Qt::UserRole+DRole_CHP_FULLNAME)
        {
             return item.getFullChapterName(); // 获取某一章节的完整名字
        }
        else if (role == Qt::UserRole+DRole_RC_ANIMATING)
        {
             return item.isAnimating(); // 章节是否正在创建的动画之中
        }
        else if (role == Qt::UserRole+DRole_ROLL_NAMES)
        {
             return roll_names; // 获取分卷的名字
        }
        else if (role == Qt::UserRole+DRole_RC_SELECTING)
        {
             return item.isSelecting(); // 列表项是否在选中状态
        }
        // ==== 目录项点击动画 ====
        else if (role == Qt::UserRole+DRole_RC_CLICKPOINT)
        {
             return item.getClickPoint(); // 鼠标点击的坐标，就是水波纹的起始方向
        }
        else if (role == Qt::UserRole+DRole_RC_SELECT_PROG100)
        {
             // return item.getNextActiveProg(); // 下一个水波纹的动画进度
             return item.getSelectProg100();
        }
        else if (role == Qt::UserRole+DRole_RC_UNSELECT_PROG100)
        {
             return item.getUnselectProg100();
        }
        // ==== 目目录项鼠标悬浮动画 ====
        else if (role == Qt::UserRole+DRole_RC_HOVERPROG)
        {
             return item.getHoverLeaveProg100(); // 鼠标悬浮的动画进度
        }
        else if (role == Qt::UserRole+DRole_RC_NEXT_HOVERPROG)
        {
             return item.getHoverProg100(); // 下一个鼠标悬浮的动画进度（出现）
        }
        else if (role == Qt::UserRole+DRole_RC_PREV_HOVERPROG)
        {
             return item.getLeaveProg100(); // 上一个鼠标悬浮的的动画进度（消失）
        }
        // ==== 其他设置 ====
        else if (role == Qt::UserRole+DRole_RC_WORD_COUNT)
        {
            return getRcWordCount(index.row()); // 获取字数
        }

        return QVariant();
    }

    // 重写 flags 和 setData 使 Model 可双击编辑
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        //if (index.row() > 0 && us->one_click) // 作品相关卷名不允许更改
            //flags |= Qt::ItemIsEditable;
        if (index.row() > 0)
            flags |= Qt::ItemIsEditable;
        return flags;
    }

    /**
     * 修改章节名字结束后
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (!index.isValid()) return false;
        NovelDirItem& item = cr_list[index.row()]; // 引用，第一次修改名字后打开章节
        if (role == Qt::EditRole) // 修改章节名
        {
            QString old_name = item.getName();
            bool isNew = item.isNew();
            if (canRegExp(old_name, "新[章卷]\\d+")) // 分卷是聚集焦点用。就算不是新建的，看着名字也当做新的
                isNew = true;
            bool rst = tryChangeName(index.row(), value.toString());
            if (/*isNew && */rst) // 重命名成功
            {
                if (isNew) // 如果是新建的
                    emit signalOpenChapter(index.row());
                else // 否则就是重命名，修改已经打开的旧名字标签页
                {
                    emit signalChangeFullName(novel_name, value.toString());
                    emit signalFocuItem(index.row());
                }
            }
            return rst;
        }
        else if (role == Qt::UserRole+DRole_CHP_STEMP)
        {
            cr_list[index.row()].setS_temp(value.toString());
        }
        else if (role == Qt::UserRole+DRole_CHP_FULLNAME)
        {
            cr_list[index.row()].setFullChapterName(value.toString());
            deb(value.toString(), "model.setFullChapterName");
        }
        return true;
    }

    NovelDirItem* getItem(int index)
    {
        return &cr_list[index];
    }

    void listDataChanged()
    {
        beginResetModel();
        endResetModel();

        emit signalListDataChange(); // 发送给 QListView 的修改信号，用以调用 恢复隐藏item 的函数
    }

    void gotoReedit()
    {
        emit signalReedit();
    }

    void changeNameSuccessed(bool isChapter, QString novelName, QString oldName, QString newName)
    {
        emit signalChangeName(isChapter, novelName, oldName, newName);
    }

    void deleteChapterSuccessed(QString novelName, QString chapterName)
    {
        emit signalDeleteChapter(novelName, chapterName);
    }

signals:
    void signalListDataChange();
    void signalReedit();
    void signalOpenCurrentChapter();
    void signalOpenChapter(int index);
    void signalFocuItem(int row);
    void signalChangeName(bool is_chapter, QString novel_name, QString old_name, QString new_name);
    void signalChangeFullName(QString novel_name, QString chapter_name);
    void signalDeleteChapter(QString novel_name, QString chapter_name);

};

#endif // NOVELDIRMODEL_H
