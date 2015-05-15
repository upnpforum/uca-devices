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

Rectangle {
    width: 800
    height: 700
    color: uiConstants.backgroundColor

    UiConstants { id: uiConstants }

    FontLoader { id: captionFontBold; source: "res/Montserrat-Bold.ttf" }
    FontLoader { id: captionFontRegular; source: "res/Montserrat-Regular.ttf" }

    Rectangle {
        id: topBar
        x: 0
        y: 0
        width: parent.width
        height: 76
        color: uiConstants.topBarColor
        z: 1

        Text {
            id: title
            x: 20
            y: 14
            color: "#ffffff"
            text: settingsView.upnp_friendlyName
            font.family: captionFontBold.name
            font.weight: Font.Bold
            font.pixelSize: 24
        }

        Text {
            id: version
            y: 20
            color: uiConstants.textAccent
            text: "v2.1"
            anchors.left: title.right
            anchors.leftMargin: 8
            font.family: captionFontRegular.name
            font.pixelSize: 18
        }

        Text {
            id: userJidd
            x: 20
            y: 44
            width: 145
            height: 13
            color: "#ffffff"
            text: settingsView.xmpp_userName
            font.family: captionFontRegular.name
            font.pixelSize: 14
            opacity: 0.8
        }

        Image {
            id: settingsBtn
            x: 622
            y: 14
            width: 22
            height: 22
            anchors.right: aboutBtn.left
            anchors.rightMargin: 6
            source: "res/settingsBtn.png"

            MouseArea {
                id: mouseArea1
                x: 0
                y: 0
                anchors.fill:parent
                onClicked:{
                    settingsView.open()
                }
            }
        }

        Image {
            id: aboutBtn
            x: 650
            y: 14
            width: 22
            height: 22
            anchors.right: parent.right
            anchors.rightMargin: 28
            source: "res/aboutBtn.png"
            MouseArea {
                id: mouseArea2
                x: 0
                y: 0
                anchors.fill: parent
                onClicked: {
                    aboutView.visible = true
                }
            }
        }
    }

    Image {
        id: background
        x: 0
        y: 50
        width: parent.width
        height: parent.height-y
        source: "res/background.png"

        Rectangle {
            id: rectangle1
            x: 0
            y: 0
            width: 700
            height: 500
            color:"transparent"
            anchors.centerIn:parent

            Slider {
                id: brightness
                x: 436
                y: 450
                width: 174
                height: 16
                value: dimmableLightModel.lightLoadLevel
                minValue: 1
                maxValue: 100
            }

            OnOffButton {
                id: onOffButton1
                x: 357
                y: 272
                dissabledImg: "res/onOffDissable.png"
                state: dimmableLightModel.lightEnabled
            }

            CloudButton {
                id: cloudButton1
                x: 356
                y: 161
                state: dimmableLightModel.ucaStackEnabled
            }

            LocalButton {
                id: localButton1
                x: 356
                y: 6
                state: dimmableLightModel.udaStackEnabled
            }

            Text {
                id: text3
                x: 596
                y: 350
                width: 100
                height: 61
                color: "white"
                text: "ON/OFF"
                font.family: captionFontRegular.name
                wrapMode: Text.WordWrap
                font.pixelSize: 16
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: text2
                x: 594
                y: 189
                width: 100
                height: 61
                color: "white"
                text: "CLOUD CONNECTIVITY"
                wrapMode: Text.WordWrap
                font.pixelSize: 16
                font.family: captionFontRegular.name
                verticalAlignment: Text.AlignVCenter
            }

            Image {
                id: light
                x: 47
                y: 43
                width: 255
                height: 423
                z: 1
                source: "res/light_off.png"
            }

            Image {
                id: lightOn
                x: 47
                y: 43
                width: 255
                height: 423
                z: 1
                opacity: onOffButton1.state ? (brightness.value-brightness.minValue)/brightness.maxValue : 0
                source: "res/light.png"
            }

            Image {
                id: gloss
                x: 0
                y: 6
                width: 350
                height: 350
                z: 2
                opacity: onOffButton1.state ? (brightness.value-brightness.minValue)/brightness.maxValue : 0
                source: "res/gloss.png"
            }

            Text {
                id: text1
                x: 594
                y: 30
                width: 100
                height: 61
                color: "white"
                text: "LOCAL CONNECTIVITY"
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                font.pixelSize: 16
                font.family: captionFontRegular.name
            }
        }
    }

    SettingsView {
        id: settingsView
        x: 0
        y: 0
        z: 10
        anchors.fill:parent
        visible:false
    }

    AboutView {
        id: aboutView
        x: 0
        y: 0
        z: 10
        anchors.fill:parent
        visible:false
    }
}


