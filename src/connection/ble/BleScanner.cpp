/**
 * @file BleScanner.cpp
 * @brief BLE设备扫描器实现
 */

#include "connection/ble/BleScanner.h"

static constexpr int SCAN_TIMEOUT_MS = 10000; ///< 默认扫描超时10秒

BleScanner::BleScanner(QObject* parent)
    : QObject(parent)
    , m_scanTimer(new QTimer(this))
{
    m_scanTimer->setSingleShot(true);
    connect(m_scanTimer, &QTimer::timeout,
            this, &BleScanner::onScanTimeout);
}

BleScanner::~BleScanner()
{
    stopScan();
}

void BleScanner::startScan()
{
    m_devices.clear();
    m_scanTimer->start(SCAN_TIMEOUT_MS);
    // TODO: 集成 QBluetoothDeviceDiscoveryAgent
}

void BleScanner::stopScan()
{
    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
    // TODO: 停止 QBluetoothDeviceDiscoveryAgent
}

QVariantList BleScanner::discoveredDevices() const
{
    return m_devices;
}

void BleScanner::onScanTimeout()
{
    stopScan();
    emit scanFinished();
}
