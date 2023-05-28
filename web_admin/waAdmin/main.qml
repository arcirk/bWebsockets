import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

ApplicationWindow {

    visible: true
    width: 800
    height: 640
    title: qsTr("WebSocket Server (arcirk)")

    GridLayout{
        //Layout.fillWidth: true
        columns: 2
        //anchors.fill: parent
        //spacing: 5

        Text{
            id: txtWs
            padding: 10
            text: "WebSocket Server started";
            font.bold: true
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignTop
            height: txtWs.implicitHeight
            width: parent.width
        }

        Text{
            padding: 10
            text: "Файлы для загрузки:"
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignTop
        }

        Text{
            padding: 10
            text: "price_checker.bak"
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignTop
        }
    }

//    StackView{

//        id: stackView
//        anchors.fill: parent

//        initialItem: PageStart{
//            id: pageStart
//            objectName: "pageStart"

//        }


//    }
}
