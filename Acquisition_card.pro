#-------------------------------------------------
#
# Project created by QtCreator 2024-04-28T10:09:19
#
#-------------------------------------------------

#QT       += core gui
#QT       += core gui network websockets opengl
QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Zigbee_network
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    util/AppConfig.cpp \
    util/AppLog.cpp \
    util/ClickSlider.cpp \
    util/HitTimer.cpp \
    util/IniFileUtil.cpp \
    util/JsonUtil.cpp \
    util/LimitFileDialog.cpp \
    util/QFileUtil.cpp \
    util/QJsonUtil.cpp \
    util/QPath.cpp \
    util/QSettingsShim.cpp \
    util/QStringUtil.cpp \
    util/qtfilecopier.cpp \
    util/QtGlobalMutex.cpp \
    util/QtHelper.cpp \
    util/QtTimeUtil.cpp \
    util/QWidgetText.cpp \
    util/QWidgetUtil.cpp \
    util/ResizeWidget.cpp \
    util/TextCodecUtil.cpp \
    util/Validators.cpp \
    AppConfigUtil.cpp \
    AppConst.cpp \
    AppDir.cpp \
    Application.cpp \
    NtpService.cpp \
    NtpSynchro.cpp \
    SettingArchive.cpp \
    NetworkConf.cpp \
    NetworkUtil.cpp \
    CmdProcess.cpp \
    mqtthandle.cpp \
    AdcHandle.cpp \
    ring_buffer.cpp \
    fqmqttclient.cpp \
    DmaMemcpy.cpp \
    AppService.cpp \
    TCPServer.cpp \
    tcphandlethread.cpp \
    UdpMessageHandle.cpp \
    AD4134.cpp \
    zigbeedatahandle.cpp \
    JsonFileHandle.cpp \
    BuleTooth.cpp \
    BleHandle.cpp \
    Uart458client.cpp \
    QHttpClient.cpp \
    Uart485Server.cpp \
    uartConfig4g5g.cpp \
    zigbee/base64.c \
    zigbee/cJSON.c \
    zigbee/common.c \
    zigbee/dt_crc.c \
    zigbee/dt_heap.c \
    zigbee/dt_malloc_list.c \
    zigbee/log_upload.c \
    zigbee/ringbuffer.c \
    zigbee/vpopen.c \
    zigbee/dt_uart_mcu_board.c \
    zigbee/dt_uart_manager.c \
    zigbee/dt_uart.c \
    zigbee/proto_parse.c \
    zigbee/wless_param.c \
    zigbee/proto_handle.c \
    zigbee/proto_common.c \
    zigbee/spi_manager.c \
    zigbee/param_parse.c \
    zigbeebase.cpp \
    wrapper.cpp \
    SensorLogManager.cpp

HEADERS += \
        mainwindow.h \
    freqxtype.h \
    util/AppConfig.h \
    util/AppLog.h \
    util/ClickSlider.h \
    util/HexString.h \
    util/HitTimer.h \
    util/IniFileUtil.h \
    util/JsonUtil.h \
    util/LimitFileDialog.h \
    util/QFileUtil.h \
    util/QJsonUtil.h \
    util/QPath.h \
    util/QSafeStr.h \
    util/QSettingsShim.h \
    util/QStringUtil.h \
    util/QtFileCopier \
    util/qtfilecopier.h \
    util/QtGlobalMutex.h \
    util/QtHelper.h \
    util/QtTimeUtil.h \
    util/QTypeNames.h \
    util/QVariantCast.h \
    util/QWidgetText.h \
    util/QWidgetUtil.h \
    util/ResizeWidget.h \
    util/TextCodecUtil.h \
    util/TStringJoiner.h \
    util/TypeNames.h \
    util/Validators.h \
    freqxtype.h \
    mainwindow.h \
    AppConfigUtil.h \
    AppConst.h \
    AppDir.h \
    Application.h \
    Acquisitiontype.h \
    NtpService.h \
    NtpSynchro.h \
    SettingArchive.h \
    NetworkConf.h \
    NetworkUtil.h \
    CmdProcess.h \
    mqtthandle.h \
    AdcHandle.h \
    ring_buffer.h \
    fqmqttclient.h \
    AppService.h \
    Dmamemcpy.h \
    TCPServer.h \
    tcphandlethread.h \
    UdpMessageHanlde.h \
    AD4134.h \
    ad713x.h \
    zigbeedatahandle.h \
    JsonFileHandle.h \
    BuleTooth.h \
    BleHandle.h \
    Uart458client.h \
    QHttpClient.h \
    ThreadSafeMultiVector.h \
    Uart485Server.h \
    uartConfig4g5g.h \
    zigbee/base64.h \
    zigbee/cJSON.h \
    zigbee/common.h \
    zigbee/dt_crc.h \
    zigbee/dt_heap.h \
    zigbee/dt_malloc_list.h \
    zigbee/list.h \
    zigbee/log_upload.h \
    zigbee/ringbuffer.h \
    zigbee/zigbeetype.h \
    zigbee/vpopen.h \
    zigbee/dt_uart_mcu_board.h \
    zigbee/dt_uart_manager.h \
    zigbee/dt_uart.h \
    zigbee/proto_parse.h \
    zigbee/wless_param.h \
    zigbee/proto_handle.h \
    zigbee/proto_common.h \
    zigbee/spi_manager.h \
    zigbee/param_parse.h \
    zigbeebase.h \
    wrapper.h \
    SensorLogManager.h

FORMS += \
        mainwindow.ui

PROJECT_ROOT=$$PWD
#../Acquisition_card
COMN_DIR=$${PROJECT_ROOT}/comn
THIRD_PARTY=$${PROJECT_ROOT}/third_party
INCLUDEPATH += $${PROJECT_ROOT}/util/
INCLUDEPATH += $${THIRD_PARTY}/qmqtt/
LIBS +=  -L$${THIRD_PARTY}/qmqtt/lib/ -lQt5Mqtt
INCLUDEPATH += $${THIRD_PARTY}/jsoncpp/include/
LIBS +=  -L$${THIRD_PARTY}/jsoncpp/lib/ -ljsoncpp
INCLUDEPATH += $${THIRD_PARTY}/WpaClient/include/
LIBS +=  -L$${THIRD_PARTY}/WpaClient/lib/ -lWpaClient
INCLUDEPATH += $${THIRD_PARTY}/NetConfig/include/
LIBS +=  -L$${THIRD_PARTY}/NetConfig/lib/ -lNetConfig
INCLUDEPATH += $${THIRD_PARTY}/Integral/include/
LIBS +=  -L$${THIRD_PARTY}/Integral/lib/ -lIntegral -lm
INCLUDEPATH += $${COMN_DIR}/include
LIBS += -L$${COMN_DIR}/lib/ -lcomn
#LIBS += -L$${PROJECT_ROOT}/lib/

target.path = /userdata/dma/qt
INSTALLS += target
CONFIG += debug
