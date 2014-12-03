/**
 *
 * Copyright 2013-2014 UPnP Forum All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE FREEBSD PROJECT "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE OR WARRANTIES OF 
 * NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE FREEBSD PROJECT OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official 
 * policies, either expressed or implied, by the UPnP Forum.
 *
 **/

import QtQuick 2.0

Item{
    id:settingView
    property int margin: 70
    property int padding: 20
    property string upnp_friendlyName: deviceSettings.getSetting("upnp/friendlyName")
    property string upnp_udn: deviceSettings.getSetting("upnp/udn")

    property string xmpp_ip: deviceSettings.getSetting("xmpp/ipAddress")
    property string xmpp_port: deviceSettings.getSetting("xmpp/port")
    property string xmpp_password:deviceSettings.getSetting("xmpp/password")
    property string xmpp_pubsubService:deviceSettings.getSetting("xmpp/pubsubService")
    property string xmpp_userName:deviceSettings.getSetting("xmpp/userName")

    FontLoader { id: captionFontRegular; source: "res/Montserrat-Regular.ttf" }

    width: 700
    height:600

    function open(){
        textInput_upnp_friendlyName.text = deviceSettings.getSetting("upnp/friendlyName")
        textInput_upnp_udn.text = deviceSettings.getSetting("upnp/udn")

        textInput_xmpp_ip.text = deviceSettings.getSetting("xmpp/ipAddress")
        textInput_xmpp_port.text = deviceSettings.getSetting("xmpp/port")
        textInput_xmpp_userName.text = deviceSettings.getSetting("xmpp/userName")
        textInput_xmpp_password.text = deviceSettings.getSetting("xmpp/password")
        textInput_xmpp_pubsubService.text = deviceSettings.getSetting("xmpp/pubsubService")

        settingsView.visible = true
    }

    Rectangle {
        id:opacityLayer
        anchors.fill: parent
        color: "gray"
        opacity: 0.7
    }

    Rectangle {
        id: rectangle2
        width: parent.width-settingView.margin
        height: parent.height-settingView.margin
        color: "white"
        opacity: 1
        anchors.centerIn:parent

        DialogTopBar {
            id: dialogTopBar1
            x: 0
            y: 0
            width:parent.width
            dialog: settingsView
            title:"Settings"
        }

        Rectangle {
            id: rectangle1
            x: padding
            y: dialogTopBar1.height + padding
            width: parent.width / 2 - padding * 2
            height: parent.height - y -footer.height - padding * 2
            color: "#ffffff"

            Text {
                id: text1
                x: 0
                y: 0
                text: "UPnP"

                font.family: captionFontRegular.name
                font.bold: true
                font.pixelSize: 20
            }

            Column {

                anchors.top: text1.bottom
                anchors.margins: padding
                width: parent.width

                Rectangle {
                    id: friendlyNameBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label
                        x: 0
                        y: 0
                        text: "Friendly name"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "white";
                        border.width: 1
                        border.color: "gray"

                        TextInput {
                            id: textInput_upnp_friendlyName
                            color: "black"
                            anchors.fill: parent
                            wrapMode: TextInput.WrapAnywhere
                            clip: true
                            text: settingView.upnp_friendlyName
                            anchors.margins: padding / 4
                        }
                    }
                }

                Rectangle {
                    id: udnBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label1
                        x: 0
                        y: 0
                        text: "UDN"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "#ffffff"
                        border.width: 1
                        border.color: "#808080"
                        TextInput {
                            id: textInput_upnp_udn
                            color: "#000000"
                            clip: true
                            text: settingView.upnp_udn
                            anchors.fill: parent
                            anchors.margins: padding / 4
                        }
                    }
                }
            }
        }

        Rectangle {
            id: rectangle3
            x: padding + parent.width / 2
            y: dialogTopBar1.height + padding
            width: parent.width / 2 - padding * 2
            height: parent.height - y -footer.height - padding * 2
            color: "#ffffff"

            Text {
                id: text2
                x: 0
                y: 0
                text: "XMPP"

                font.family: captionFontRegular.name
                font.pixelSize: 20
                font.bold: true
            }

            Column {

                anchors.top: text2.bottom
                anchors.margins: padding
                width: parent.width

                Rectangle {
                    id: ipBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label2
                        x: 0
                        y: 0
                        text: "IP"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "#ffffff"
                        border.width: 1
                        border.color: "#808080"

                        TextInput {
                            id: textInput_xmpp_ip
                            color: "#000000"
                            text: settingView.xmpp_ip
                            clip: true
                            anchors.fill: parent
                            anchors.margins: padding / 4
                        }
                    }
                }

                Rectangle {
                    id: portBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label3
                        x: 0
                        y: 0
                        text: "Port"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "#ffffff"
                        border.width: 1
                        border.color: "#808080"
                        TextInput {
                            id: textInput_xmpp_port
                            color: "#000000"
                            text: settingView.xmpp_port
                            clip: true
                            anchors.fill: parent
                            anchors.margins: padding / 4
                        }
                    }
                }

                Rectangle {
                    id: userNameBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label4
                        x: 0
                        y: 0
                        text: "User name"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "#ffffff"
                        border.width: 1
                        border.color: "#808080"
                        TextInput {
                            id: textInput_xmpp_userName
                            color: "#000000"
                            text: settingView.xmpp_userName
                            clip: true
                            anchors.fill: parent
                            anchors.margins: padding / 4
                        }
                    }
                }

                Rectangle {
                    id: passwordBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label5
                        x: 0
                        y: 0
                        text: "Password"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "#ffffff"
                        border.width: 1
                        border.color: "#808080"
                        TextInput {
                            id: textInput_xmpp_password
                            color: "#000000"
                            text: settingView.xmpp_password
                            echoMode: TextInput.PasswordEchoOnEdit
                            clip: true
                            anchors.fill: parent
                            anchors.margins: padding / 4
                        }
                    }
                }

                Rectangle {
                    id: pubsubServiceBox
                    width: parent.width
                    height: 32
                    Text {
                        id: label6
                        x: 0
                        y: 0
                        text: "PubSubService"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        width: parent.width * 0.6
                        height: parent.height - padding / 2

                        anchors.right: parent.right
                        anchors.margins: padding

                        color: "#ffffff"
                        border.width: 1
                        border.color: "#808080"
                        TextInput {
                            color: "#000000"
                            id: textInput_xmpp_pubsubService
                            text: settingView.xmpp_pubsubService
                            clip: true
                            anchors.fill: parent
                            anchors.margins: padding / 4
                        }
                    }
                }
            }
        }

        Rectangle {
            id: footer
            x: 0
            y: parent.height-height
            width: parent.width
            height: 50
            color: "#ffffff"

            Rectangle {
                id: cancelBtn
                x: 517
                y: 9
                width: 105
                height: 33
                color: "#ffffff"
                border.width: 1
                border.color: "#808080"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        settingView.visible = false
                    }

                    Text {
                        id: text3
                        text: qsTr("Cancel")
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        anchors.fill: parent
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                id: saveBtn
                x: 406
                y: 9
                width: 105
                height: 33
                color: "#ffffff"
                border.width: 1
                border.color: "#808080"
                MouseArea{
                    anchors.fill: parent
                    onClicked:{
                        deviceSettings.setSetting("upnp/friendlyName",textInput_upnp_friendlyName.text)
                        deviceSettings.setSetting("upnp/udn",textInput_upnp_udn.text)

                        deviceSettings.setSetting("xmpp/ipAddress",textInput_xmpp_ip.text)
                        deviceSettings.setSetting("xmpp/port",textInput_xmpp_port.text)
                        deviceSettings.setSetting("xmpp/userName",textInput_xmpp_userName.text)
                        deviceSettings.setSetting("xmpp/password",textInput_xmpp_password.text)
                        deviceSettings.setSetting("xmpp/pubsubService",textInput_xmpp_pubsubService.text)

                        settingView.upnp_friendlyName = textInput_upnp_friendlyName.text
                        settingsView.upnp_udn = textInput_upnp_udn.text

                        settingsView.xmpp_ip = textInput_xmpp_ip.text
                        settingsView.xmpp_port = textInput_xmpp_port.text
                        settingsView.xmpp_userName = textInput_xmpp_userName.text
                        settingsView.xmpp_password = textInput_xmpp_password.text
                        settingsView.xmpp_pubsubService = textInput_xmpp_pubsubService.text

                        settingView.visible = false

                        dimmableLightModel.restart()
                    }

                    Text {
                        id: text4
                        text: qsTr("Save")
                        font.pixelSize: 12
                        verticalAlignment: Text.AlignVCenter
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
