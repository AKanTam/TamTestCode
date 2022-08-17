#include "mylistmodel.h"
#include "mywork.h"

MyListModel *MyListModel::getInstance()
{
   static MyListModel * obj = new MyListModel;
   return obj;
}

MyListModel::MyListModel(QObject *parent)
    : QAbstractListModel(parent)
{

    MyData tempData;

    tempData.url = "snow.png";
    tempData.visible = true;
    m_data.insert(0,tempData);
    tempData.url = "rain.png";
    m_data.insert(1,tempData);
    tempData.url = "bingbao.png";
    m_data.insert(2,tempData);

    MyWork* mywork = new MyWork;

    connect(mywork, &MyWork::chanceVisable, this, [=](int index,QString url,bool flag)
    {
        beginResetModel();
        MyData tempData;
        tempData.url = url;
        tempData.visible = flag;
        m_data.insert(index,tempData);
        endResetModel();

        qDebug()<< m_data.value(index).url<<".visible: "<<m_data.value(index).visible ;
    });

    mywork->start();
}


int MyListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_data.count();

    // FIXME: Implement me!
}

QVariant MyListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role){
    case MyRoleName::Url:
        return m_data[index.row()].url;
        break;
    case MyRoleName::Visible:
        return m_data[index.row()].visible;
    default:
        break;
    }
    // FIXME: Implement me!
    return QVariant();
}

QHash<int, QByteArray> MyListModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles.insert(MyRoleName::Url,"Wurl");//字符串是Qml端用
    roles.insert(MyRoleName::Visible,"Wvisible");//枚举是C++来判断

    return roles;
}
