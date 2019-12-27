#ifndef FILEDOWNLOADMANAGER_H
#define FILEDOWNLOADMANAGER_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <stdio.h>
#include <QDebug>
#include "globalvar.h"

/**
 * 网络下载文件工具类
 */
class FileDownloadManager: public QObject
{
    Q_OBJECT
public:
    FileDownloadManager();
    void setDownloadParam(QString url, QString filename);
    void start();

private:
    void doDownload(const QUrl &url);
    QString getSaveFileName(const QUrl &url);
    bool saveToDisk(const QString &filename, QIODevice *data);

public slots:
    void slotDownloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);

signals:
    void signalDownloadFinished();

private:
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;
    QString download_url;
    QString save_file_name;
};

#endif // FILEDOWNLOADMANAGER_H
