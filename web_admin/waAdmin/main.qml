import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Controls.Material.impl 2.15
import QtQuick.Layouts 1.12

ApplicationWindow {

    visible: true
    title: qsTr("arcirk WebSocket Server")

    StackView{

        id: stackView
        anchors.fill: parent

        initialItem: PageStart{
            id: pageStart
            objectName: "pageStart"

        }


    }
}
