// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QSerialPortInfo>

struct SerialPortInfo {
    QString portName;
    QString description;
    QString manufacturer;
    QString serialNumber;
    QString systemLocation;
    quint16 vendorId = 0;
    quint16 productId = 0;
    bool isAvailable = false;
};

class SerialDetector : public QObject {
    Q_OBJECT
public:
    explicit SerialDetector(QObject *parent = nullptr);
    ~SerialDetector() override;

    void startMonitoring(int intervalMs = 1000);
    void stopMonitoring();
    QList<SerialPortInfo> availablePorts() const;
    QStringList portNames() const;
    bool isMonitoring() const;

    // Filter helpers
    QList<SerialPortInfo> findByVendorId(quint16 vid) const;
    QList<SerialPortInfo> findByDescription(const QString &keyword) const;
    SerialPortInfo findByPortName(const QString &name) const;

signals:
    void portInserted(const SerialPortInfo &info);
    void portRemoved(const SerialPortInfo &info);
    void portsChanged(const QList<SerialPortInfo> &current);
    void monitoringChanged(bool active);

private:
    void refreshPorts();
    SerialPortInfo fromQtInfo(const QSerialPortInfo &info) const;

    QTimer m_timer;
    QMap<QString, SerialPortInfo> m_knownPorts;
    bool m_monitoring = false;
};