#include "apps/serial_station/SerialStationConfig.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

namespace serial_station {

namespace {

QString configText(const char* sourceText)
{
    return QCoreApplication::translate("SerialPortConfig", sourceText);
}

bool isKnownDataBits(QSerialPort::DataBits dataBits)
{
    switch (dataBits) {
    case QSerialPort::Data5:
    case QSerialPort::Data6:
    case QSerialPort::Data7:
    case QSerialPort::Data8:
        return true;
    }

    return false;
}

bool isKnownParity(QSerialPort::Parity parity)
{
    switch (parity) {
    case QSerialPort::NoParity:
    case QSerialPort::EvenParity:
    case QSerialPort::OddParity:
    case QSerialPort::SpaceParity:
    case QSerialPort::MarkParity:
        return true;
    }

    return false;
}

bool isKnownStopBits(QSerialPort::StopBits stopBits)
{
    switch (stopBits) {
    case QSerialPort::OneStop:
    case QSerialPort::OneAndHalfStop:
    case QSerialPort::TwoStop:
        return true;
    }

    return false;
}

bool isKnownFlowControl(QSerialPort::FlowControl flowControl)
{
    switch (flowControl) {
    case QSerialPort::NoFlowControl:
    case QSerialPort::HardwareControl:
    case QSerialPort::SoftwareControl:
        return true;
    }

    return false;
}

QString dataBitsText(QSerialPort::DataBits dataBits)
{
    switch (dataBits) {
    case QSerialPort::Data5:
        return QStringLiteral("5");
    case QSerialPort::Data6:
        return QStringLiteral("6");
    case QSerialPort::Data7:
        return QStringLiteral("7");
    case QSerialPort::Data8:
        return QStringLiteral("8");
    }

    return configText("?");
}

QString parityShortText(QSerialPort::Parity parity)
{
    switch (parity) {
    case QSerialPort::NoParity:
        return QStringLiteral("N");
    case QSerialPort::EvenParity:
        return QStringLiteral("E");
    case QSerialPort::OddParity:
        return QStringLiteral("O");
    case QSerialPort::SpaceParity:
        return QStringLiteral("S");
    case QSerialPort::MarkParity:
        return QStringLiteral("M");
    }

    return configText("?");
}

QString stopBitsText(QSerialPort::StopBits stopBits)
{
    switch (stopBits) {
    case QSerialPort::OneStop:
        return QStringLiteral("1");
    case QSerialPort::OneAndHalfStop:
        return QStringLiteral("1.5");
    case QSerialPort::TwoStop:
        return QStringLiteral("2");
    }

    return configText("?");
}

QString flowControlText(QSerialPort::FlowControl flowControl)
{
    switch (flowControl) {
    case QSerialPort::NoFlowControl:
        return configText("无流控");
    case QSerialPort::HardwareControl:
        return configText("硬件流控");
    case QSerialPort::SoftwareControl:
        return configText("软件流控");
    }

    return configText("未知流控");
}

QString lineStateText(bool enabled)
{
    if (enabled) {
        return QStringLiteral("on");
    }

    return QStringLiteral("off");
}

} // namespace

QString SerialPortConfig::normalizedPortName() const
{
    return portName.trimmed();
}

SerialPortConfig SerialPortConfig::normalized() const
{
    SerialPortConfig config = *this;
    config.portName = normalizedPortName();
    return config;
}

QString SerialPortConfig::validationError() const
{
    if (normalizedPortName().isEmpty()) {
        return configText("串口端口名为空");
    }

    if (baudRate <= 0) {
        return configText("串口波特率必须大于 0");
    }

    if (!isKnownDataBits(dataBits)) {
        return configText("串口数据位不受支持");
    }

    if (!isKnownParity(parity)) {
        return configText("串口校验位不受支持");
    }

    if (!isKnownStopBits(stopBits)) {
        return configText("串口停止位不受支持");
    }

    if (!isKnownFlowControl(flowControl)) {
        return configText("串口流控不受支持");
    }

    return {};
}

bool SerialPortConfig::isValid() const
{
    return validationError().isEmpty();
}

QString SerialPortConfig::summary() const
{
    const QString frameFormat =
        dataBitsText(dataBits) + parityShortText(parity) + stopBitsText(stopBits);

    const QStringList parts = {
        normalizedPortName().isEmpty() ? configText("<未选择端口>") : normalizedPortName(),
        QString::number(baudRate),
        frameFormat,
        flowControlText(flowControl),
        QStringLiteral("DTR=%1").arg(lineStateText(dtrEnabled)),
        QStringLiteral("RTS=%1").arg(lineStateText(rtsEnabled)),
    };

    return parts.join(QLatin1Char(' '));
}

bool SerialStationConfig::isReconnectEnabled() const
{
    return autoReconnect && reconnectIntervalMs > 0;
}

} // namespace serial_station
