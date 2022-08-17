#ifndef MYWORK_H
#define MYWORK_H

#include <QThread>
#include <QDebug>

class MyWork : public QThread
{
    Q_OBJECT
public:
    explicit MyWork(QObject *parent = nullptr);

protected:
    void run();

signals:
    void chanceVisable(int index,QString url,int flag);

};

#endif // MYWORK_H
