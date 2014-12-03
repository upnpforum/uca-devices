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

Item {
    id: localButton
    property string enabledImg: "res/localEnable.png"
    property string dissabledImg: "res/localDissable.png"
    property bool state: false

    signal buttonStateChanged(bool newState)

    opacity: state ? 1 : 0.6

    Rectangle {
        id: rectangle7
        x: 85
        y: 61
        width: 3
        height: 91
        color: "#c2ffc0"
        z: 1
    }

    Image {
        id: image12
        x: 78
        y: 151
        source: "res/smallCircle.png"
        z: 1
    }

    Rectangle {
        id: rectangle5
        x: 14
        y: 157
        width: 65
        height: 3
        color: "#c2ffc0"
        z: 1
    }

    Image {
        id: image10
        x: 0
        y: 151
        source: "res/smallCircle.png"
        z: 1
    }

    Image {
        id: image6
        x: 78
        y: 46
        source: "res/smallCircle.png"
        z: 1
    }

    Image {
        id: localConnBtn
        x: 114
        y: 0
        source: "res/button.png"
        z: 1
        Image {
            id: image1
            x: 27
            y: 27
            width: 55
            height: 55
            source: localButton.state ? localButton.enabledImg : localButton.dissabledImg
        }

        MouseArea {
            id: mouseArea1
            x: 4
            y: 0
            width: parent.width
            height: parent.height

            onClicked: {
                localButton.state = !localButton.state
                localButton.buttonStateChanged(localButton.state)
                dimmableLightModel.enableUda(localButton.state)
            }
        }
    }

    Rectangle {
        id: rectangle1
        x: 92
        y: 53
        width: 25
        height: 3
        color: "#c2ffc0"
        z: 1
    }
}
