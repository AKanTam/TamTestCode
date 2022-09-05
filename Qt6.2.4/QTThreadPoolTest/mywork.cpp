#include "mywork.h"


void MyWork::run()
{
    {
        QSqlDatabase mydb = QSqlDatabase::addDatabase("QMYSQL",QString::number(quintptr(QThread::currentThreadId())));
        mydb.setHostName("172.20.61.29");
        mydb.setUserName("sysadmin");
        mydb.setPassword("adminsys");
        mydb.setDatabaseName("jzwsd");
        reConnect:
        if (mydb.open() != false){
            QSqlQuery query(mydb);
            QString sqls ="";
            itPoint = itBegin;
            //qDebug() <<"["<<QString::number(quintptr(QThread::currentThreadId()))<<"]"<<"MYSQL任务开始运行";
            qint64 startTime = QDateTime::currentMSecsSinceEpoch();
            for(int i = 0; i<itNum;++i){
                sqls = "INSERT INTO cur_test ( QString ) VALUES ( '"+ *itPoint +"' )";
                //qDebug()<<sqls;
                query.exec(sqls);
                ++itPoint;
            }
            qint64 taskTime = QDateTime::currentMSecsSinceEpoch() - startTime;
            qDebug() <<"[线程号:"<<QString::number(quintptr(QThread::currentThreadId()))<<"]"<<"完成任务,任务耗时"<<taskTime<<"ms";
        }else{
            //qDebug() <<"["<<QString::number(quintptr(QThread::currentThreadId()))<<"]"<<"MYSQL连接创建失败  "<<mydb.lastError().text();
            goto reConnect;
        }
    }
    QSqlDatabase::removeDatabase(QString::number(quintptr(QThread::currentThreadId())));
}
