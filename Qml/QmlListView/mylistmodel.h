#ifndef MYLISTMODEL_H
#define MYLISTMODEL_H

#include <QAbstractListModel>

struct MyData{
    QString url;
    bool visible;
};

class MyListModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum MyRoleName{
        Url = Qt::DisplayRole + 1,
        Visible
    };

    static MyListModel * getInstance();

    explicit MyListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int,QByteArray> roleNames() const override;

private:

    QMap<int,MyData> m_data;
};

#endif // MYLISTMODEL_H
