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

    property Item dialog: null
    property string title: ""

    FontLoader { id: captionFontRegular; source: "res/Montserrat-Regular.ttf" }

    id: topBar
    x: 0
    y: 0
    width: parent.width
    height: 50
    color: "#081D30"

    Text {
        id: text1
        x: 10
        y: 10
        width: 100
        height: 30
        color: "#eeeeee"
        text: topBar.title
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap

        font.family: captionFontRegular.name
        font.pixelSize: 18
        font.capitalization: Font.AllUppercase
        font.weight: Font.Bold
    }

    Image {
        id: close

        anchors.margins: 10
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        width: height

        source: "res/close.png"

        MouseArea {
            id: mouseArea1
            anchors.fill:parent
            onClicked: {
                if(topBar.dialog!=null){
                    topBar.dialog.visible = false
                }
            }
        }
    }
}

