import QtQuick 2.14
import QtQuick.Window 2.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.14
import Qt.labs.platform 1.1
import QtGraphicalEffects 1.14

Window {
    visible: true;
    width: 1180
    height: 770
    minimumWidth: 1050
    minimumHeight: 680
    color: "#232324"
    title: "图片浏览器"

    property string picturesLocation : "";
    property var imageNameFilters : ["所有图片格式 (*.png; *.jpg; *.bmp; *.gif; *.jpeg)"];
    property var pictureList : []
    property var pictureIndex : 0
    property var scaleMax : 800                                                                                          // 最大800%
    property var scaleMin : 10                                                                                             // 最小10%
    property var titleColor : "#E8E8E8"
    property var contentColor : "#D7D7D7"
    property var ctrlSliderList : [
        ["放大", scaleMin, scaleMax , photoImage.scale * 100 , "%"],
        ["旋转", -180, 180 , photoImage.rotation, "°"],
    ]

    FileDialog {
        id: fileDialog
        title: "请打开图片(可以多选)"
        fileMode: FileDialog.OpenFiles
        folder: picturesLocation
        nameFilters: imageNameFilters
        onAccepted: {
            pictureList = files
            openNewImage(0)
        }
        onFolderChanged: picturesLocation = folder
   }

   ColumnLayout {
       anchors.fill: parent
       spacing: 2                                                      //
       RowLayout {
           Layout.fillHeight: true
           Layout.fillWidth: true
           spacing: 1                                                 //
           Flickable {                                                                                                                 // 大图浏览区，用来存放放置当前大图的一个Flickable容器
               id: flick
               Layout.fillHeight: true
               Layout.fillWidth: true
               MouseArea {
                   anchors.fill: parent
                   onWheel: {
                       if (wheel.modifiers & Qt.ControlModifier) {                                                            // ctrl + 滑轮 旋转图片
                           photoImage.rotation += wheel.angleDelta.y / 120 * 5;
                           if (photoImage.rotation > 180)
                               photoImage.rotation = 180
                           else if (photoImage.rotation < -180)
                               photoImage.rotation = -180
                           if (Math.abs(photoImage.rotation) < 4)                                                                             // 如果绝对值小于4°，则摆正图片
                               photoImage.rotation = 0;
                        } else {
                            photoImage.scale += photoImage.scale * wheel.angleDelta.y / 120 / 10;       // 滑轮 缩放图片
                            if (photoImage.scale > scaleMax / 100)
                                photoImage.scale = scaleMax / 100
                            else if (photoImage.scale < scaleMin / 100)
                                photoImage.scale = scaleMin / 100
                        }
                    }
                }
                Image {                                                                                                             // 大图浏览区，用来显示当前大图的一个Image
                    id: photoImage
                    fillMode: Image.Pad
                    source: (typeof pictureList[pictureIndex] === 'undefined') ? "" : pictureList[pictureIndex]
                    smooth: true
                    mipmap: true
                    antialiasing: true
                    Component.onCompleted: {
                        x = parent.width / 2 - width / 2
                        y = parent.height / 2 - height / 2
                        pictureList.length = 0
                    }

                   PinchArea {
                       anchors.fill: parent
                       pinch.target: parent
                       pinch.minimumRotation: -180                                                             // 设置拿捏旋转图片最大最小比例
                       pinch.maximumRotation: 180
                       pinch.minimumScale: 0.1                                                                     // 设置拿捏缩放图片最小最大比例
                       pinch.maximumScale: 10
                       pinch.dragAxis: Pinch.XAndYAxis
                   }

                   MouseArea {                                                                                             // 设置左键拖动效果
                       anchors.fill: parent
                       drag.target: parent
                       drag.axis: Drag.XAndYAxis
                       drag.minimumX: 20 - photoImage.width
                       drag.maximumX: flick.width - 20
                       drag.minimumY: 20 - photoImage.height
                       drag.maximumY: flick.height - 20
                   }
               }
           }

          Rectangle {
              Layout.fillHeight: true
              Layout.fillWidth: false
              Layout.preferredWidth : 220
              color: "#313131"
              DynamicGroupBox {                                                                                     // 改！！！文件选项组合框
                   id: fileGroup
                   title: "文件选项"
                   width: parent.width
                   ColumnLayout {
                       anchors.centerIn: parent
                       spacing: 12
                       Repeater {
                           model : ListModel {
                               id: fileModel
                               ListElement { name: "打开文件"; }
                               ListElement { name: "上一张"; }
                               ListElement { name: "下一张"; }
                           }
                          DynamicBtn {                                                                                    //
                              text: fileModel.get(index).name
                              backColor: "#3A3A3A"
                              fontColor: contentColor
                              fontPixelSize: 14
                              onPressed: fileGroupPressed(index)
                          }
                     }
                 }
                 Component.onCompleted: initGroupBox(this);
             }

            DynamicGroupBox {                                                                                    // 图片控制组合框                   //
                id: ctrlGroup
                title: "图片控制"
                width: parent.width
                anchors.top: fileGroup.bottom
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    Repeater {
                        model : 2
                        RowLayout {
                            width: parent.width
                            Text {
                                color: contentColor
                                Layout.fillWidth: false
                                Layout.preferredWidth : 50
                                text: ctrlSliderList[index][0]
                                horizontalAlignment: Text.AlignRight
                                font.pixelSize: 14
                           }
                          DynamicSlider {
                              id: ctrlSlider
                              Layout.fillWidth: true
                              Layout.preferredWidth : 130
                              from: ctrlSliderList[index][1]
                              value: ctrlSliderList[index][3]
                              to: ctrlSliderList[index][2]
                              stepSize: 1
                              onMoved: setCtrlValue(index, value);
                          }
                          Text {
                              color: "#D4D4D4"
                              Layout.fillWidth: false
                              Layout.preferredWidth : 40
                              text: parseInt(ctrlSliderList[index][3].toString()) + ctrlSliderList[index][4]
                           }
                       }
                   }
               }
               Component.onCompleted: initGroupBox(this);
           }

          DynamicGroupBox {                                                                                     // 基本信息组合框           //
              id: imageInfoGroup
              title: "基本信息"
              width: parent.width
              height: 120
              anchors.top: ctrlGroup.bottom
              ColumnLayout {
                  width: parent.width
                  spacing: 16
                  Text {
                      color: contentColor
                      text: "尺寸: " + photoImage.sourceSize.width + "X" + photoImage.sourceSize.height
                      font.pixelSize: 14
                  }
                 Text {
                     color: contentColor
                     text: "路径: " + ((typeof pictureList[pictureIndex] === 'undefined') ?
                     "等待打开文件..." : pictureList[pictureIndex].replace("file:///",""))
                     Layout.preferredWidth: parent.width - 20
                     Layout.preferredHeight: 60
                     wrapMode: Text.Wrap
                     font.pixelSize: 14
                  }
              }
              Component.onCompleted: initGroupBox(this);
          }

           DynamicGroupBox {                                                                             // 关于组合框               //
               id: authorInfoGroup
               title: "关于"
               width: parent.width
               height: 110
               anchors.top: imageInfoGroup.bottom
               ColumnLayout {
                   width: parent.width
                   spacing: 16
                   Text {
                       color: contentColor
                       text: "设计: ***"
                       Layout.preferredWidth: parent.width - 20
                       wrapMode: Text.Wrap
                       font.pixelSize: 14
                   }
                  Text {
                      color: contentColor
                      text: "网址: <font color=\"#D4D4D4\"><a href=\"http://www.supconit.com/\">中控信息/</a></font>"
                      font.pixelSize: 14
                      onLinkActivated: Qt.openUrlExternally(link)
                   }
               }
              Component.onCompleted: initGroupBox(this);
               }
           }
       }

       Rectangle {                                                                                                                  // 多个图片浏览区
           id: images
           Behavior on Layout.preferredHeight { NumberAnimation { duration: 250 } }
           Layout.fillHeight: false
           Layout.fillWidth: true
           Layout.preferredHeight: 130
           LinearGradient {
               anchors.fill: parent
               source: parent
               start: Qt.point(0, 0)
               end: Qt.point(0, parent.height)
               gradient: Gradient {
                   GradientStop { position: 0.0; color: "#484848" }
                   GradientStop { position: 0.01; color: "#373737" }
                   GradientStop { position: 1.0; color: "#2D2D2D" }
                }
            }
           Button {
               id: imageCtrlBtn
               text: images.Layout.preferredHeight <= 30 ? "展开("+pictureList.length+")" :
               "收起("+pictureList.length+")"
               anchors.right: parent.right
               anchors.rightMargin: 3
               z: 100
               background: Rectangle {
                   color: "transparent"
                }
                contentItem: Label {                                                          // 设置文本
                    id: btnForeground
                    text: parent.text
                    font.family: "Microsoft Yahei"
                    font.pixelSize: 14
                    color: imageCtrlBtn.hovered ? "#D7D7D7" : "#AEAEAE"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onPressed: {
                     if (text.indexOf("收起") >= 0) {
                         images.Layout.preferredHeight = 30
                     } else {
                         images.Layout.preferredHeight = 130
                     }
                 }
             }
             ScrollView {
                 id: imageScroll
                 anchors.fill: parent
                 anchors.leftMargin: 10
                 anchors.rightMargin: 10
                 wheelEnabled: true
                 WheelHandler {                                                                                                                                                      //鼠标滑轮切换
                     onWheel: openNewImageAndUpdateScroll(event.angleDelta.y > 0 ? pictureIndex - 1 : pictureIndex + 1)
                  }
                 ScrollBar.horizontal.policy: ScrollBar.horizontal.size >= 1.0 ?
                 ScrollBar.AlwaysOff : ScrollBar.AlwaysOn
                 ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                 ScrollBar.horizontal.contentItem: Rectangle {
                     implicitHeight: 7
                     implicitWidth: 100
                     radius: height / 2
                     color: "#7D7C7C"
                     visible: images.Layout.preferredHeight <= 30 ? false : true
                 }
                Row {
                    anchors.fill: parent
                    anchors.topMargin: 30
                    spacing: 20
                    Repeater {
                        model: pictureList.length
                        Button {
                            implicitWidth: 85
                            implicitHeight: 85
                            onPressed: openNewImage(index)
                            background: Rectangle {
                                color: "#202020"
                                border.color: pictureIndex == index ? "#2770DF" :
                                hovered ? "#6C6A6A" : "transparent"
                                radius: 5
                                border.width: 3
                            }
                            Image {
                                anchors.fill:parent
                                anchors.margins: 6
                                antialiasing: true
                                fillMode: Image.PreserveAspectFit
                                source: pictureList[index]
                               }
                           }
                       }
                   }
               }
           }
       }

       function initGroupBox(group) {                                                                //
           group.titleLeftBkColor = "#313131"
           group.titleRightBkColor = "#474951"
           group.titleColor = titleColor
           group.contentBkColor = "#2A2A2A"
           group.borderColor = "#454545"
           group.titleFontPixel = 14
           group.radiusVal = 0
           group.borderWidth = 1
        }

       function fileGroupPressed(index) {                                                           //
           switch (index) {
               case 0 : fileDialog.open(); break;
               case 1 : openNewImageAndUpdateScroll(pictureIndex - 1); break;
               case 2 : openNewImageAndUpdateScroll(pictureIndex + 1); break;
            }
        }

        function setCtrlValue(index, value) {                                                     //
            switch (index) {
                case 0 : photoImage.scale = value / 100; break;
                case 1 : photoImage.rotation = value; break;
            }
        }

       function openNewImage(index) {                                                         //
           if (index < 0 || index >= pictureList.length) {
           }                                                                                                                //？
           pictureIndex = index
           photoImage.x = flick.width / 2 - photoImage.width / 2
           photoImage.y = flick.height / 2 - photoImage.height / 2
           photoImage.scale = 1.0
           photoImage.rotation = 0
       }

       function openNewImageAndUpdateScroll(index) {                           //
           if (index < 0 || index >= pictureList.length) {
               return false
           }
           pictureIndex = index
           photoImage.x = flick.width / 2 - photoImage.width / 2
           photoImage.y = flick.height / 2 - photoImage.height / 2
           photoImage.scale = 1.0
           photoImage.rotation = 0
           var scrollLen = 1.0 - imageScroll.ScrollBar.horizontal.size;
           if (scrollLen > 0) {
               scrollLen = scrollLen * pictureIndex / (pictureList.length - 1)
               imageScroll.ScrollBar.horizontal.position = scrollLen
            }
            return true
       }
   }
