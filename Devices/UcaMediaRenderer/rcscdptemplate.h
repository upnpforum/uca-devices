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

#ifndef RCSCDPTEMPLATE_H
#define RCSCDPTEMPLATE_H

static const char *SCDP_TEMPLATE
    = "<?xml version=\"1.0\"?>"
      "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
          "<specVersion>"
              "<major>1</major>"
              "<minor>0</minor>"
          "</specVersion>"
        "<actionList>"
          "<action>"
          "<name>ListPresets</name>"
            "<argumentList>"
              "<argument>"
                "<name>InstanceID</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
              "</argument>"
              "<argument>"
                "<name>CurrentPresetNameList</name>"
                "<direction>out</direction>"
                "<relatedStateVariable>PresetNameList</relatedStateVariable>"
              "</argument>"
            "</argumentList>"
          "</action>"
          "<action>"
          "<name>SelectPreset</name>"
            "<argumentList>"
              "<argument>"
                "<name>InstanceID</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
              "</argument>"
              "<argument>"
                "<name>PresetName</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_PresetName</relatedStateVariable>"
              "</argument>"
            "</argumentList>"
          "</action>"
          "<action>"
          "<name>GetVolume</name>"
            "<argumentList>"
              "<argument>"
                "<name>InstanceID</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
              "</argument>"
              "<argument>"
                "<name>Channel</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>"
              "</argument>"
              "<argument>"
                "<name>CurrentVolume</name>"
                "<direction>out</direction>"
                "<relatedStateVariable>Volume</relatedStateVariable>"
              "</argument>"
            "</argumentList>"
          "</action>"
          "<action>"
          "<name>SetVolume</name>"
            "<argumentList>"
              "<argument>"
                "<name>InstanceID</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
              "</argument>"
              "<argument>"
                "<name>Channel</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>"
              "</argument>"
              "<argument>"
                "<name>DesiredVolume</name>"
                "<direction>in</direction>"
                "<relatedStateVariable>Volume</relatedStateVariable>"
              "</argument>"
            "</argumentList>"
          "</action>"
        "</actionList>"
          "<serviceStateTable>"
            "<stateVariable sendEvents=\"no\">"
              "<name>PresetNameList</name>"
              "<dataType>string</dataType>"
            "</stateVariable>"
            "<stateVariable sendEvents=\"yes\">"
              "<name>LastChange</name>"
              "<dataType>string</dataType>"
            "</stateVariable>"
            "<stateVariable sendEvents=\"no\">"
              "<name>A_ARG_TYPE_PresetName</name>"
              "<dataType>string</dataType>"
              "<allowedValueList>"
                "<allowedValue>FactoryDefaults</allowedValue>"
              "</allowedValueList>"
            "</stateVariable>"
        "<stateVariable sendEvents=\"no\">"
          "<name>Volume</name>"
          "<dataType>ui2</dataType>"
            "<allowedValueRange>"
                "<minimum>0</minimum>"
                "<maximum>1</maximum>"
                "<step>1</step>"
            "</allowedValueRange>"
        "</stateVariable>"
        "<stateVariable sendEvents=\"no\">"
          "<name>A_ARG_TYPE_Channel</name>"
          "<dataType>string</dataType>"
            "<allowedValueList>"
                "<allowedValue>Master</allowedValue>"
            "</allowedValueList>"
        "</stateVariable>"
        "<stateVariable sendEvents=\"no\">"
          "<name>A_ARG_TYPE_InstanceID</name>"
          "<dataType>ui4</dataType>"
        "</stateVariable>"
       "</serviceStateTable>"
      "</scpd>";

#endif // RCSCDPTEMPLATE_H
