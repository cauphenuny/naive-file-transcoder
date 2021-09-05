#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "libini.h"

// 去除str前的"和后的";
char *delchar(char *str, char *temp)
{
    int len = strlen(str);
    char *p1 = str;
    if ((0 == strncmp("\"", str, 1)) && (str[len - 1] == ';')
        && (str[len - 2] == '"')) {
        p1[len - 2] = '\0';
        return strcpy(temp, p1 + 1);
    } else {
        return str;
    }
    return str;
}

// 去除字符空格
char *del_space(char *str, char *buf)
{
    unsigned int uLen = strlen(str);

    if (0 == uLen) {
        return (char*)"\0";
    }

    memset(buf, 0, uLen + 1);

    unsigned int i = 0, j = 0;
    for (i = 0; i < uLen + 1; i++) {
        if (str[i] != ' ') {
            buf[j++] = str[i];
        }
    }
    buf[j] = '\0';

    return buf;
}

// 创建临时内存
int libini_memory(char **pp)
{
    if (pp != NULL) {
        char *tmp = (char *)malloc(BUFFER_SIZE);
        if (tmp != NULL) {
            *pp = tmp; // pp的地址指向tmp
            return 1;
        }
    }
    return -1;
}

// 释放临时内存
void libini_free(char *p)
{
    if (p != NULL) {
        free(p);
    }
}

int getinikeystring(const char *title, const char *key, const char *filename, char *buf)
{
    *buf = 0;
    FILE *fp;
    int flag = 0;
    char sTitle[BUFFER_SIZE], *wTmp;
    char sLine[BUFFER_SIZE], sSection[BUFFER_SIZE] = "[]";
    sprintf(sTitle, "[%s]", title);
    char buf1[BUFFER_SIZE];

    if (NULL == (fp = fopen(filename, "r"))) {
        perror("fopen");
        return -1;
    }
    if (0 == strncmp(sTitle, sSection, strlen(sTitle))) {
        flag = 1;
    }
    while (NULL != fgets(sLine, BUFFER_SIZE, fp)) {
        if (0 == strncmp("//", sLine, 2))
            continue;
        if (sLine[0] == '#' || 0 == strncmp("#", sLine, 1))
            continue;
        if (sLine[0] == ';' || 0 == strncmp(";", sLine, 1))
            continue;
        wTmp = strchr(sLine, '=');
        if ((NULL != wTmp) && (1 == flag)) {
            sLine[strlen(sLine) - 1] = '\0';

            //if (0 == strncmp(key, sLine, strlen(key)) && !(strncasecmp(strtok(sLine, "="), key, strlen(strtok(sLine, "="))))) {
            if (0 == strncmp(key, sLine, strlen(key)) &&
                !(strncasecmp
                  (del_space(strtok(sLine, "="), buf1), key,
                   strlen(del_space(strtok(sLine, "="), buf1))))) {   // 判断key长度的等号左边字符是否相等, 判断key长度的等号左边字符(去除空格)是否相等
                fclose(fp);
                while (*(wTmp + 1) == ' ') {
                    wTmp++;
                }
                delchar(strcpy(buf, wTmp + 1), buf);
                return 0;
            }
        } else {
            if (sLine[0] == '[') {
                strncpy(sSection, sLine, strlen(sLine));
                if (0 == strncmp(sTitle, sSection, strlen(sTitle))) {
                    flag = 1;
                } else {
                    flag = 0;
                }
            }
        }
    }
    fclose(fp);
    return -1;
}

int putinikeystring(const char *title, const char *key, const char *val, const char *filename)
{
    char buf[BUFFER_SIZE];
    FILE *fpr;
    FILE *fpw;
    int flag = 0;
    char sFull[BUFFER_SIZE], sTitle[BUFFER_SIZE];
    char sSection[BUFFER_SIZE] = "[]";
    char sLine[BUFFER_SIZE], sLine_backup[BUFFER_SIZE], *wTmp;
    sprintf(sTitle, "[%s]", title);
    sprintf(sFull, "%s = %s\n", key, val);
    sprintf(sLine, "%s.tmp", filename);
    if (NULL == (fpr = fopen(filename, "r"))) {
    	fpr = fopen(filename, "w");
    	fclose(fpr);
    	fpr = fopen(filename, "r");
	} else {
        fclose(fpr);
        rename(filename, sLine);
        fpr = fopen(sLine, "r");
    }
    if (NULL == (fpw = fopen(filename, "w")))
        return -2;
    if (0 == strncmp(sTitle, sSection, strlen(sTitle))) {
        flag = 1;
    }
    while (NULL != fgets(sLine, BUFFER_SIZE, fpr)) {
        if (2 != flag) {
            wTmp = strchr(sLine, '=');
            if ((NULL != wTmp) && (1 == flag)) {
                strcpy(sLine_backup, sLine);
                if (0 == strncmp(key, sLine, strlen(key))
                    &&
                    !(strncasecmp
                      (del_space(strtok(sLine_backup, "="), buf), key,
                       strlen(del_space(strtok(sLine_backup, "="), buf))))) {
                    flag = 2;
                    sprintf(wTmp + 1, " %s\n", val);
                }
            } else {
                if (sLine[0] == '[') {
                    strncpy(sSection, sLine, strlen(sLine));
                    if (0 == strncmp(sTitle, sSection, strlen(sTitle))) {
                        flag = 1;
                    } else {
                        if (1 == flag) {
                            fputs(sFull, fpw);
                            flag = 2;
                        }
                    }
                }
            }
        }
        fputs(sLine, fpw);
    }
    if (!flag) {
        if (strcmp(sSection, "[]") == 0)
    	    sprintf(sTitle, "[%s]\n", title);
        else
    	    sprintf(sTitle, "\n[%s]\n", title);
    	fputs(sTitle, fpw);
    	fputs(sFull, fpw);
	}
    if (flag == 1) {
        fputs(sFull, fpw);
    }
    fclose(fpr);
    fclose(fpw);
    sprintf(sLine, "%s.tmp", filename);
    return remove(sLine);
}

int getinikeyint(const char *title, const char *key, const char *filename)
{
    char buf[BUFFER_SIZE];
    getinikeystring(title, key, filename, buf);
    return atoi(buf);
}

long long getinikeylong(const char *title, const char *key, const char *filename)
{
    char buf[BUFFER_SIZE];
    getinikeystring(title, key, filename, buf);
    return atoll(buf);
}

double getinikeyfloat(const char *title, const char *key, const char *filename)
{
    char buf[BUFFER_SIZE];
    getinikeystring(title, key, filename, buf);
    return atof(buf);
}
