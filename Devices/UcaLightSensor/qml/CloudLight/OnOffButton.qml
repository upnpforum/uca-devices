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
    id: button
    property string enabledImg: "res/onOffEnable.png"
    property string dissabledImg: "res/onOffDissable.png"
    property bool state: false

    signal buttonStateChanged(bool newState)

    Rectangle {
        id: rectangle8
        x: 83
        y: 14
        width: 3
        height: 91
        color: "#c2ffc0"
        z: 1
    }

    Image {
        id: image13
        x: 76
        y: 0
        source: "res/smallCircle.png"
        z: 1
    }

    Rectangle {
        id: rectangle6
        x: 12
        y: 6
        width: 65
        height: 3
        color: "#c2ffc0"
        z: 1
    }

    Image {
        id: image11
        x: -2
        y: 0
        source: "res/smallCircle.png"
        z: 1
    }

    Image {
        id: image8
        x: 76
        y: 103
        source: "res/smallCircle.png"
        z: 1
    }

    Image {
        id: onOffBtn
        x: 111
        y: 55
        source: "res/buttonFilled.png"
        z: 2
        Image {
            id: onOffState
            x: (parent.width-width)/2
            y: (parent.height-height)/2
            source: button.state ? button.enabledImg : button.dissabledImg
            z: 1
        }

        MouseArea {
            id: mouseArea3
            x: 5
            y: 0
            width: parent.width
            height: parent.height

            onClicked: {
                dimmableLightModel.lightEnabled = !button.state
                button.buttonStateChanged(button.state)
            }
        }
    }

    Rectangle {
        id: rectangle3
        x: 90
        y: 110
        width: 25
        height: 3
        color: "#c2ffc0"
        z: 1
    }
}
