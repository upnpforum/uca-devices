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

#ifndef CMSCDPTEMPLATE_H
#define CMSCDPTEMPLATE_H

static const char *CM_SCDP_TEMPLATE
    = "<?xml version=\"1.0\"?>"
      "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
          "<specVersion>"
              "<major>1</major>"
              "<minor>0</minor>"
          "</specVersion>"
        "<actionList>"
            "<action>"
                "<name>GetProtocolInfo</name>"
                "<argumentList>"
                "<argument>"
                    "<name>Sink</name>"
                    "<direction>out</direction>"
                    "<relatedStateVariable>SinkProtocolInfo</relatedStateVariable>"
                "</argument>"
                "<argument>"
                    "<name>Source</name>"
                    "<direction>out</direction>"
                   "<relatedStateVariable>SourceProtocolInfo</relatedStateVariable>"
                    "</argument>"
                "</argumentList>"
            "</action>"
            "<action>"
                "<name>GetCurrentConnectionIDs</name>"
                "<argumentList>"
                  "<argument>"
                    "<name>ConnectionIDs</name>"
                    "<direction>out</direction>"
                    "<relatedStateVariable>CurrentConnectionIDs</relatedStateVariable>"
                  "</argument>"
                "</argumentList>"
            "</action>"
            "<action>"
                "<name>GetCurrentConnectionInfo</name>"
                "<argumentList>"
                    "<argument>"
                        "<name>ConnectionID</name>"
                        "<direction>in</direction>"
                       "<relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>"
                    "</argument>"
        "<argument>"
            "<name>AVTransportID</name>"
            "<direction>out</direction>"
            "<relatedStateVariable>A_ARG_TYPE_AVTransportID</relatedStateVariable>"
        "</argument>"
        "<argument>"
            "<name>Direction</name>"
            "<direction>out</direction>"
            "<relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>"
        "</argument>"
        "<argument>"
            "<name>PeerConnectionID</name>"
            "<direction>out</direction>"
           "<relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>"
        "</argument>"
        "<argument>"
            "<name>PeerConnectionManager</name>"
            "<direction>out</direction>"
            "<relatedStateVariable>A_ARG_TYPE_ConnectionManager</relatedStateVariable>"
        "</argument>"
        "<argument>"
            "<name>ProtocolInfo</name>"
            "<direction>out</direction>"
            "<relatedStateVariable>A_ARG_TYPE_ProtocolInfo</relatedStateVariable>"
        "</argument>"
                    "<argument>"
                        "<name>RcsID</name>"
                        "<direction>out</direction>"
                        "<relatedStateVariable>A_ARG_TYPE_RcsID</relatedStateVariable>"
                    "</argument>"


                    "<argument>"
                        "<name>Status</name>"
                        "<direction>out</direction>"
                        "<relatedStateVariable>A_ARG_TYPE_ConnectionStatus</relatedStateVariable>"
                    "</argument>"
                "</argumentList>"
            "</action>"
        "</actionList>"
          "<serviceStateTable>"
              "<stateVariable sendEvents=\"yes\">"
                  "<name>SourceProtocolInfo</name>"
                  "<dataType>string</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"yes\">"
                  "<name>SinkProtocolInfo</name>"
                  "<dataType>string</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"yes\">"
                  "<name>CurrentConnectionIDs</name>"
                  "<dataType>string</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_ConnectionStatus</name>"
                  "<dataType>string</dataType>"
                  "<allowedValueList>"
                      "<allowedValue>OK</allowedValue>"
                      "<allowedValue>ContentFormatMismatch</allowedValue>"
                      "<allowedValue>InsufficientBandwidth</allowedValue>"
                      "<allowedValue>UnreliableChannel</allowedValue>"
                      "<allowedValue>Unknown</allowedValue>"
                  "</allowedValueList>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_ConnectionManager</name>"
                  "<dataType>string</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_Direction</name>"
                  "<dataType>string</dataType>"
                  "<allowedValueList>"
                      "<allowedValue>Input</allowedValue>"
                      "<allowedValue>Output</allowedValue>"
                  "</allowedValueList>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_ProtocolInfo</name>"
                  "<dataType>string</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_ConnectionID</name>"
                  "<dataType>i4</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_AVTransportID</name>"
                  "<dataType>i4</dataType>"
              "</stateVariable>"
              "<stateVariable sendEvents=\"no\">"
                  "<name>A_ARG_TYPE_RcsID</name>"
                  "<dataType>i4</dataType>"
              "</stateVariable>"
          "</serviceStateTable>"
      "</scpd>";

#endif // CMSCDPTEMPLATE_H
