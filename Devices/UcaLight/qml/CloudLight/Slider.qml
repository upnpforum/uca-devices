/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

Item {
    id:sliderWrapper
    property color color: "#545454"
    property double minValue: 0.0
    property double maxValue: 1.0
    property double value: 0.5

    signal sliderValueChanged(double newValue)

    FontLoader { id: captionFontRegular; source: "res/Montserrat-Regular.ttf" }

    Text {
        anchors.left: container.right
        anchors.verticalCenter: container.verticalCenter
        anchors.margins: 16

        color: "white"
        opacity: 0.8
        text: "100"
        font.family: captionFontRegular.name
        font.pixelSize: 16
    }

    Text {
        anchors.right: container.left
        anchors.verticalCenter: container.verticalCenter
        anchors.margins: 16

        color: "white"
        opacity: 0.8
        text: "0"
        font.family: captionFontRegular.name
        font.pixelSize: 16
    }

    Rectangle {
        id: container

        color: "transparent"
        anchors.centerIn: parent
        height: 18
        width: 120

        Image {
            source: "res/sliderBarOff.png"
        }

        Rectangle {
            id: highlight
            color: "transparent"
            x: 0
            y: 0
            height: container.height
            width: slider.x + slider.width / 2
            clip:true

            Image {
                source: "res/sliderBarOn.png"
            }
        }

        MouseArea {
            anchors.fill:parent
            onClicked: {
                var v = (mouse.x-(slider.width/2))/(container.width-slider.width)* (sliderWrapper.maxValue-sliderWrapper.minValue)+sliderWrapper.minValue
                v = v < sliderWrapper.minValue ? sliderWrapper.minValue : v;
                v = v > sliderWrapper.maxValue ? sliderWrapper.maxValue : v;
                dimmableLightModel.lightLoadLevel = v
                sliderWrapper.sliderValueChanged(sliderWrapper.value)
                console.log(v);
            }
        }

        Rectangle {
            id: slider
            x: sliderWrapper.value / (sliderWrapper.maxValue-sliderWrapper.minValue) * (parent.width - width)
            y: (parent.height-height)/2; width: 20; height: 20
            color: "transparent"

            Image {
                source: "res/sliderHandle.png"
            }

            MouseArea {
                anchors.fill: parent
                drag.target: parent; drag.axis: Drag.XAxis
                drag.minimumX: 0; drag.maximumX: container.width - parent.width

                onPressedChanged: {
                    if(!pressed){
                    var v = slider.x/(container.width-slider.width)* (sliderWrapper.maxValue-sliderWrapper.minValue)+sliderWrapper.minValue
                    v = v < sliderWrapper.minValue ? sliderWrapper.minValue : v;
                    v = v > sliderWrapper.maxValue ? sliderWrapper.maxValue : v;
                    dimmableLightModel.lightLoadLevel = v
                    sliderWrapper.sliderValueChanged(sliderWrapper.value)
                    console.log(v);
                    }
                }
            }
        }

    }
}
