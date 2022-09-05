#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //获取cpu核心数
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    int taskNum = 6*systemInfo.dwNumberOfProcessors;
    QThreadPool::globalInstance()->setMaxThreadCount(taskNum);


    testList.clear();
    for(int i = 0;i<1200;i++){
        QString tempStr = QString::number(i);
        testList.push_back(tempStr);
    }

/*
    QSqlDatabase mydb = QSqlDatabase::addDatabase("QMYSQL","soloThreadTest");
    mydb.setHostName("172.20.61.29");
    mydb.setUserName("sysadmin");
    mydb.setPassword("adminsys");
    mydb.setDatabaseName("jzwsd");
    QSqlQuery query(mydb);
    QString sqls ="";
    qDebug() <<"[单线程测试任务]"<<"MYSQL任务开始运行";
    qint64 startTime = QDateTime::currentSecsSinceEpoch();
    reTest:
    if(mydb.open() == true){
        for (QVector<QString>::iterator i = testList.begin(); i != testList.end(); i++){
            sqls = "INSERT INTO cur_test ( QString ) VALUES ( '"+ *i +"' )";
            query.exec(sqls);
        }
        qint64 taskTime = QDateTime::currentSecsSinceEpoch() - startTime;
        qDebug() <<"[单线程测试任务] "<<"完成任务,任务耗时"<<taskTime<<"s";
    }else{
        goto reTest;
    }*/


    qDebug() <<"[多线程测试任务]"<<"线程池数量设置为："<<taskNum;
    QVector<QString>::iterator itPoint = testList.begin();
    //qDebug()<<"测试列表一共有"<<testList.size()<<"个位号";

    int tagNum = testList.size()/taskNum;
    int lastTagNum = testList.size() % taskNum;

    for (int i = 0; i < taskNum; i++){
        QVector<QString>::iterator _itBegin = itPoint + (tagNum* i);
        if (i == (taskNum - 1)){
            MyWork* task = new MyWork(_itBegin,tagNum + lastTagNum);
            QThreadPool::globalInstance()->start(task);
            //qDebug()<<"创建一个任务，从"<<tagNum* i<<"开始,一共有"<<tagNum + lastTagNum<<"个位号需要推送";
        }else{
            MyWork* task = new MyWork(_itBegin,tagNum);
            QThreadPool::globalInstance()->start(task);
            //qDebug()<<"创建一个任务，从"<<tagNum* i<<"开始,一共有"<<tagNum<<"个位号需要推送";
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
