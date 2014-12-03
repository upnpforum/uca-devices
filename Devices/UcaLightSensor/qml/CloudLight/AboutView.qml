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
    id:aboutView
    property int margin: 70
    width: 700
    height:600

    Rectangle {
        id:opacityLayer
        anchors.fill: parent
        color: "gray"
        opacity: 0.7
    }
    Rectangle {
        id: rectangle2
        width: parent.width-aboutView.margin
        height: parent.height-aboutView.margin
        color: "white"
        opacity: 1
        anchors.centerIn:parent

        DialogTopBar {
            id: dialogTopBar1
            x: 0
            y: 0
            width:parent.width
            dialog: aboutView
            title:"About"
        }

        Rectangle {
            x:0
            y:dialogTopBar1.height
            width:parent.width
            height:parent.height-dialogTopBar1.height
            id: wrapper

            Text {
                id: text1
                x: 17
                y: 15
                text: qsTr("UPnP Cloud Architecture Proof of Concept Device aplication, v2.0")
                font.pixelSize: 12
            }

            Image {
                id: image1
                width: 198
                height: 75
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16
                anchors.left: parent.left
                anchors.leftMargin: 8
                fillMode: Image.PreserveAspectFit
                source: "res/Upnp_logo.jpg"

                MouseArea{
                    anchors.fill:parent
                    hoverEnabled: true
                    cursorShape: (containsMouse ?  Qt.PointingHandCursor : Qt.ArrowCursor)
                    onClicked: {
                        Qt.openUrlExternally("http://upnp.org")
                    }
                }
            }


            Image {
                id: image2
                width: 223
                height: 75
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 20
                fillMode: Image.PreserveAspectFit
                source: "res/Comarch_logo_2009_RGB_transparent.png"

                MouseArea{
                    anchors.fill:parent
                    hoverEnabled: true
                    cursorShape: (containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor)
                    onClicked: {
                        Qt.openUrlExternally("http://upnp.comarch.com")
                    }
                }
            }


        }

    }
}
