include(../qxmpp.pri)

TEMPLATE = subdirs

LIBS += $$QXMPP_INTERNAL_LIBS -L/usr/include/Qca-qt5/QtCrypto -lqca-qt5
SUBDIRS = example_0_connected \
          example_1_echoClient \
          example_2_rosterHandling \
          example_3_transferHandling \
#          example_4_callHandling \
          example_5_rpcInterface \
          example_6_rpcClient \
          example_7_archiveHandling \
          example_8_server \
          example_9_vCard
#          GuiClient
          
