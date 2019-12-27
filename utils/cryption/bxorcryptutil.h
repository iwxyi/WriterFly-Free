#ifndef BXORCRYPTUTIL_H
#define BXORCRYPTUTIL_H

#include <QDebug>
#include <QString>
#include <QStringList>

#define KEY_MAX_LENGTH 100

class BXORCryptUtil
{
public:
    BXORCryptUtil(QString key)
    {
        QStringList key_rows = key.split("\n", QString::SkipEmptyParts);
        for (int i = 0; i < key_rows.size(); i++)
        {
            strcpy(key_mat[i], key_rows.at(i).toLatin1().data());
        }
        key_row_count = key_rows.size();
    }

    QString encrypt(QString text)
    {
        QByteArray cipher = text.toLocal8Bit().toBase64();
        cipher = powerXorEncryptDecrypt(cipher);
        return QString(cipher);
    }

    QString decrypt(QString text)
    {
        QByteArray ba = text.toLocal8Bit();
        ba = powerXorEncryptDecrypt(ba);
        return QString::fromLocal8Bit(QByteArray::fromBase64(ba));
    }

    /**
     * 直接加密
     */
    QByteArray powerXorEncryptDecrypt(const QByteArray& str)
    {
        QByteArray ba = str;
        for (int i = 0; i < key_row_count; i++)
            ba = getXorEncryptDecrypt(ba, key_mat[i]);
        return ba;
    }

    /**
     * 多字符串加密
     */
    static QByteArray getXorEncryptDecrypt(const QByteArray& str, const char* key)
    {
        QByteArray bs = str;
        int len = static_cast<int>(strlen(key)), ofs = 0;
        for (int i = 0; i < bs.size(); i++)
        {
            bs[i] = bs[i] ^ *(key+ofs);
            ofs++;
            if (ofs >= len)
                ofs = 0;
        }
        return bs;
    }

    /**
     * 单字符串加密
     */
    static QByteArray getXorEncryptDecrypt(const QByteArray& str, const char &key)
    {
        QByteArray bs = str;
        for (int i = 0; i < bs.size(); i++)
        {
            bs[i] = bs[i] ^ key;
        }
        return bs;
    }

private:
    QStringList keys;
    char key_mat[KEY_MAX_LENGTH][KEY_MAX_LENGTH];
    int key_row_count;

};

#endif // BXORCRYPTUTIL_H
