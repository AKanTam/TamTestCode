#include "mywork.h"

MyWork::MyWork(QObject *parent)
    : QThread{parent}
{

}

void MyWork::run()
{
    qDebug() << "当前线程对象的地址: " << QThread::currentThread();

    while(1)
    {
        for(int index = 0;index < 3;index++){
            switch(index){
            case 0:
                emit chanceVisable(index,"snow.png",false);
                break;
            case 1:
                emit chanceVisable(index,"rain.png",false);
                break;
            case 2:
                emit chanceVisable(index,"bingbao.png",false);
                break;
            default:
                break;
            }
            QThread::usleep(5000000);
        }
        for(int index = 0;index < 3;index++){
            switch(index){
            case 0:
                emit chanceVisable(index,"snow.png",true);
                break;
            case 1:
                emit chanceVisable(index,"rain.png",true);
                break;
            case 2:
                emit chanceVisable(index,"bingbao.png",true);
                break;
            default:
                break;
            }
            QThread::usleep(5000000);
        }
    }
    qDebug() << "run() 执行完毕, 子线程退出...";
}
