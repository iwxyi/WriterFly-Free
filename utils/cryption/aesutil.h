#ifndef AESUTIL_H
#define AESUTIL_H

#include <QString>
//#include <QDebug>
#include "aes.h"

class AESUtil
{
public:
    AESUtil(QString key) : aes(Bits128, (unsigned char*)(key.toLatin1().data()))
    {
    }

    AESUtil(unsigned char* key) : aes(Bits128, key)
    {
    }

    AESUtil() {}

    void print_uchar(void *data, int len)
    {
        for(int j=0; j<len; j++)
            printf("%X ", ((unsigned char *)data)[j]);
        printf("\n");
    }

    QString encryption(QString text)
    {
        int len = strlen((char*)(text.toLatin1().data()));
        int malloc_len = len * sizeof(char) + 1;
//qDebug() << "开始加密 len=" << len;
        // 转换成 char* 类型
        char* string = (char*)malloc(malloc_len);
        memset((char*)string, 0, malloc_len);
        strcpy(string, (char*)(text.toLatin1().data()));

        // 输出的字符串
        char*  out_string = (char*)malloc(malloc_len);
        memset((char*)out_string, 0, malloc_len);

        unsigned char input[17]; // 这些是16就够了，留下最后一位0用来调试输出
        unsigned char output [17];

        for (int i = 0; i < len; i+=16)
        {
            memset((void*)input, 0, 17);
            strncpy((char *)input,(char *)(string+i), len-i<16?len-i:16);
            aes.Cipher(input, output);
            strncpy(out_string+i, (char*)output, 16);

//            printf(" >> %d:%s\n", i, input);
//            print_uchar(input, 16);
//            print_uchar(output, 16);
        }
//        printf("out_string:%s\n", out_string);
        QString res(QLatin1String((char*)out_string));

        free(string);
        free(out_string);
//        qDebug() << "加密结束" << res;
        return res;
    }

    QString decryption(QString text)
    {
        int len = strlen((char*)(text.toLatin1().data()));
        int malloc_len = len * sizeof(char) + 1;
//        printf("char:%s\n", (char*)(text.toLatin1().data()));
//        qDebug() << "开始解密 len=" << len;

        // 转换成 char* 类型
        char* string = (char*)malloc(malloc_len);
        memset((char*)string, 0, malloc_len);
        strcpy(string, (char*)(text.toLatin1().data()));

        // 输出的字符串
        char* out_string = (char*)malloc(malloc_len);
        memset((char*)out_string, 0, malloc_len);

        unsigned char input[17];
        unsigned char output [17];

        for (int i = 0; i < len; i+=16)
        {
            memset((void*)input, 0, 17);
            strncpy((char *)input,(char *)(string+i), len-i<16?len-i:16);
            aes.InvCipher(input, output);
            strncpy(out_string+i, (char*)output, 16);

//            printf(" >> %d:%s\n", i, output);
//            print_uchar(input, 16);
//            print_uchar(output, 16);
        }
//        printf("out_string2:%s\n", out_string);
        QString res(out_string);

        free(string);
        free(out_string);
        return res;
    }

private:
    AES aes;
};

#endif // AESUTIL_H
