#ifndef IMPORTMZFYPAGE_H
#define IMPORTMZFYPAGE_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QShowEvent>
#include <QFileInfo>
#include <QLineEdit>
#include "noveldirlistview.h"
#include "aniswitch.h"
#include "mylabel.h"
#include "globalvar.h"
#include "defines.h"
#include "fileutil.h"
#include "stringutil.h"

/**
 * 从码字风云导入界面
 */
class ImportMzfyPage : public QWidget
{
    Q_OBJECT
public:
    explicit ImportMzfyPage(QWidget *parent = nullptr);
    void initLayout();
    void initData();

protected:
    void showEvent(QShowEvent*);
    bool importMzfyNovel(QString novel_name, QString mzfy_dir_path, QString mzfy_chapters_path);

signals:
    void signalImportMzfyFinished(QString s);

public slots:
    void slotStartImport();

private:
    QString novel_name;
    QString file_path;
    bool import_dir_settings;

    QPushButton* path_button;
    QPushButton* submit_button;
};

#endif // IMPORTMZFYPAGE_H
