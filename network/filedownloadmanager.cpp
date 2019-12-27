#include "filedownloadmanager.h"

FileDownloadManager::FileDownloadManager()
{
    ensureDirExist(rt->APP_PATH + "download");
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(slotDownloadFinished(QNetworkReply*)));
}

void FileDownloadManager::setDownloadParam(QString url, QString filename)
{
    download_url = url;
    save_file_name = filename;
}

void FileDownloadManager::start()
{
    QUrl url = QUrl::fromEncoded(QString("http://writerfly.cn/download/latest_version.txt").toLocal8Bit());
    doDownload(url);
}

void FileDownloadManager::doDownload(const QUrl &url)
{
    qDebug() << "download:" << url;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

#ifndef QT_NO_SSL
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
#endif

    currentDownloads.append(reply); // 添加到数组中
}

QString FileDownloadManager::getSaveFileName(const QUrl &url)
{
    // 默认为设置的文件名，否则则为下载的文件名
    QString basename = QFileInfo(url.path()).fileName();
    QString filename = save_file_name.isEmpty() ? save_file_name : basename;
    QString path = rt->DOWNLOAD_PATH + "test.txt";//filename;

    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    return path;
}

bool FileDownloadManager::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing: %s\n", qPrintable(filename), qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

void FileDownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void FileDownloadManager::slotDownloadFinished(QNetworkReply *reply)
{
    deb("下载完成");
    QUrl url = reply->url();
    if (reply->error()) {
        fprintf(stderr, "Download of %s failed: %s\n", url.toEncoded().constData(), qPrintable(reply->errorString()));
    } else {
        QString filename = getSaveFileName(url);
        if (saveToDisk(filename, reply))
        {
            printf("Download of %s succeeded (saved to %s)\n", url.toEncoded().constData(), qPrintable(filename));
            emit signalDownloadFinished();
        }
    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();


}
