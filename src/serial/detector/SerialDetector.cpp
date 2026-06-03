#include "serial/detector/SerialDetector.h"
#include <QSerialPortInfo>
#include <algorithm>

SerialDetector::SerialDetector(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SerialDetector::refreshPorts);
}

SerialDetector::~SerialDetector() = default;

void SerialDetector::startMonitoring(int intervalMs)
{
    m_timer.setInterval(qMax(100, intervalMs));
    refreshPorts();
    m_timer.start();
    m_monitoring = true;
    emit monitoringChanged(true);
}

void SerialDetector::stopMonitoring()
{
    m_timer.stop();
    m_monitoring = false;
    emit monitoringChanged(false);
}

QList<SerialPortInfo> SerialDetector::availablePorts() const
{
    return m_knownPorts.values();
}

QStringList SerialDetector::portNames() const
{
    return m_knownPorts.keys();
}

bool SerialDetector::isMonitoring() const { return m_monitoring; }

QList<SerialPortInfo> SerialDetector::findByVendorId(quint16 vid) const
{
    QList<SerialPortInfo> result;
    for (const auto &info : m_knownPorts) {
        if (info.vendorId == vid) result.append(info);
    }
    return result;
}

QList<SerialPortInfo> SerialDetector::findByDescription(const QString &keyword) const
{
    QList<SerialPortInfo> result;
    for (const auto &info : m_knownPorts) {
        if (info.description.contains(keyword, Qt::CaseInsensitive)) result.append(info);
    }
    return result;
}

SerialPortInfo SerialDetector::findByPortName(const QString &name) const
{
    return m_knownPorts.value(name);
}

void SerialDetector::refreshPorts()
{
    QMap<QString, SerialPortInfo> current;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &pi : ports) {
        SerialPortInfo info = fromQtInfo(pi);
        info.isAvailable = true;
        current[info.portName] = info;
    }

    // Detect insertions
    for (const auto &name : current.keys()) {
        if (!m_knownPorts.contains(name)) {
            emit portInserted(current[name]);
        }
    }

    // Detect removals
    for (const auto &name : m_knownPorts.keys()) {
        if (!current.contains(name)) {
            emit portRemoved(m_knownPorts[name]);
        }
    }

    m_knownPorts = current;
    emit portsChanged(current.values());
}

SerialPortInfo SerialDetector::fromQtInfo(const QSerialPortInfo &info) const
{
    SerialPortInfo spi;
    spi.portName = info.portName();
    spi.description = info.description();
    spi.manufacturer = info.manufacturer();
    spi.serialNumber = info.serialNumber();
    spi.systemLocation = info.systemLocation();
    spi.vendorId = info.hasVendorIdentifier() ? info.vendorIdentifier() : 0;
    spi.productId = info.hasProductIdentifier() ? info.productIdentifier() : 0;
    return spi;
}