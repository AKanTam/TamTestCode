import QtQuick 2.5
import QtQuick.Window 2.15
import QtQuick.Controls 2.0

import MyModel 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    ListView{
        Rectangle{
            anchors.fill: parent
            color: "red"
            z: -1
        }
        id: list
        currentIndex: 0
        clip: true
        width: 200
        height: 300
        anchors.centerIn: parent.center
        model: MyListModel
        snapMode: ListView.SnapOneItem
        orientation: ListView.Horizontal
        spacing: 50
        delegate: Image {
            id: txt
            width: 200
            height: 200
            visible: Wvisible
            source: Wurl
        }
        onModelChanged: {
            console.log ("1")
        }
    }
    Rectangle{
        width: 200
        height: 300
        anchors.left: list.right
        visible: true
        z: 1
        //color: "transparent"
        color: "blue"
        MouseArea{
            anchors.fill: parent
            onClicked: {
                list.visible = !list.visible
                //list.itemAtIndex(0).visible = !list.itemAtIndex(0).visible
            }
        }
    }
}
