#ifndef MYWORK_H
#define MYWORK_H

#include <QRunnable>
#include <QObject>
#include <QThread>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSql>
#include <QDateTime>

class MyWork : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit MyWork(QVector<QString>::iterator _begin = nullptr,int _Num = 0)
    {
        // 任务执行完毕,该对象自动销毁
        itBegin = _begin;
        itNum = _Num;
        setAutoDelete(true);
    }
    ~MyWork(){

    };

    void run() override;
private:
    QVector<QString>::iterator itBegin;
    QVector<QString>::iterator itPoint;
    int itNum;
};

extern QVector<QString> testList;


#endif // MYWORK_H
