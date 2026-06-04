/**
 * @file BleGattBrowserServices.cpp
 * @brief GATT服务浏览器 -- 服务发现与GATT树操作方法实现
 *
 * 本文件从 BleGattBrowser.cpp 拆分而来，包含服务发现与GATT树相关的槽函数和内部方法:
 *   - onTreeItemChanged:        树选中项变更，更新UUID/属性/按钮状态
 *   - onServicesDiscovered:     服务发现完成回调
 *   - onRefreshClicked:         刷新按钮，重新发现GATT服务
 *   - onConnectionStateChanged: 连接状态变化处理
 *   - populateTree:             填充GATT服务树(三级: Service→Characteristic→Descriptor)
 *   - addDescriptors:           添加描述符子节点
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"
#include <QTreeWidgetItem>
#include <QRandomGenerator>

/// 模拟服务名称映射(短UUID -> 可翻译名称)
static const QMap<QString, const char*> SVC_KEYS = {
    {"00001800", QT_TRANSLATE_NOOP("BleGattBrowser", "通用访问")},
    {"00001801", QT_TRANSLATE_NOOP("BleGattBrowser", "通用属性")},
    {"0000180a", QT_TRANSLATE_NOOP("BleGattBrowser", "设备信息")},
    {"0000ffe0", QT_TRANSLATE_NOOP("BleGattBrowser", "自定义服务")},
    {"6e400001", QT_TRANSLATE_NOOP("BleGattBrowser", "Nordic UART")}
};

/// 特征属性表: {显示名, 属性标志}
static const struct { const char* name; BleGattBrowser::CharProperties prop; }
PROP_TABLE[] = {
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取"),          BleGattBrowser::PropRead},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取|写入"),     BleGattBrowser::PropRead | BleGattBrowser::PropWrite},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取|通知"),     BleGattBrowser::PropRead | BleGattBrowser::PropNotify},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "写入|通知"),     BleGattBrowser::PropWrite | BleGattBrowser::PropNotify},
    {QT_TRANSLATE_NOOP("BleGattBrowser", "读取|写入|通知"),BleGattBrowser::PropRead | BleGattBrowser::PropWrite | BleGattBrowser::PropNotify},
};

// ============================================================================
// 槽函数 -- GATT树交互 / 服务发现
// ============================================================================

/** @brief 树选中项变更，更新UUID/属性/按钮状态 */
void BleGattBrowser::onTreeItemChanged()
{
    QTreeWidgetItem* item = m_serviceTree->currentItem();
    if (!item) {
        m_selectedUuid.clear();
        m_selectedProps = CharProperties();
        m_selectedLabel->setText(tr("未选择特征"));
        updateButtonStates();
        return;
    }
    m_selectedUuid = item->data(1, Qt::DisplayRole).toString();
    m_selectedProps = getItemProperties(item);
    m_selectedLabel->setText(tr("已选择: %1 [%2]")
        .arg(item->data(0, Qt::DisplayRole).toString(),
             item->data(2, Qt::DisplayRole).toString()));
    updateButtonStates();
}

void BleGattBrowser::onServicesDiscovered(const QStringList& services) { populateTree(services); }

/** @brief 刷新按钮: 重新发现GATT服务 */
void BleGattBrowser::onRefreshClicked()
{
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        m_valueDisplay->append(tr("[错误] BLE未连接，无法刷新服务")); return;
    }
    m_valueDisplay->append(tr("[刷新] 正在重新发现GATT服务..."));
    m_discoveryTimer.start();
    const QStringList svcs = m_connection->discoverServices();
    if (!svcs.isEmpty()) populateTree(svcs);
}

/** @brief 连接状态变化: 更新UI、自动发现或停止通知 */
void BleGattBrowser::onConnectionStateChanged()
{
    updateConnectionStatus();
    if (m_connection && m_connection->state() == ConnectionState::Connected) {
        m_discoveryTimer.start();
        m_connection->discoverServices();
    } else {
        m_notificationTimer->stop();
        m_subscribedUuids.clear();
        m_notifyCheck->setChecked(false);
    }
    emit connectionStatusChanged(m_connection && m_connection->state() == ConnectionState::Connected);
}

// ============================================================================
// 内部方法 -- GATT树构建
// ============================================================================

/** @brief 填充GATT服务树(三级: Service→Characteristic→Descriptor) */
void BleGattBrowser::populateTree(const QStringList& services)
{
    m_serviceTree->clear();
    if (m_discoveryTimer.isValid()) {
        m_lastDiscoveryDurationMs = m_discoveryTimer.elapsed();
        m_discoveryTimer.invalidate();
    }
    ++m_totalServiceDiscoveries;

    for (const QString& svcUuid : services) {
        const QString shortUuid = svcUuid.left(8).toLower();
        auto* svcItem = new QTreeWidgetItem(m_serviceTree);
        svcItem->setText(0, tr(SVC_KEYS.value(shortUuid, QT_TRANSLATE_NOOP("BleGattBrowser", "未知服务"))));
        svcItem->setText(1, svcUuid);
        svcItem->setText(2, tr("主要服务"));
        svcItem->setExpanded(true);
        svcItem->setData(0, Qt::UserRole, "service");

        const int charCount = (shortUuid == "00001800") ? 3 : (shortUuid == "6e400001") ? 2 : 2;
        for (int i = 0; i < charCount; ++i) {
            auto* charItem = new QTreeWidgetItem(svcItem);
            const int propIdx = (shortUuid == "6e400001") ? (i == 0 ? 3 : 4) : i % 5;
            charItem->setText(0, tr("特征 %1").arg(i + 1));
            charItem->setText(1, svcUuid.left(8) + QString("-%1").arg(i, 4, 16, QChar('0')));
            charItem->setText(2, tr(PROP_TABLE[propIdx].name));
            charItem->setData(0, Qt::UserRole, "characteristic");
            charItem->setData(3, Qt::UserRole, static_cast<uint32_t>(PROP_TABLE[propIdx].prop));
            addDescriptors(charItem, i);
        }
    }
    m_serviceCountLabel->setText(tr("服务: %1").arg(services.size()));
}

/** @brief 添加描述符子节点 @param charItem 特征节点 @param charIndex 特征索引 */
void BleGattBrowser::addDescriptors(QTreeWidgetItem* charItem, int charIndex)
{
    auto* cccd = new QTreeWidgetItem(charItem);
    cccd->setText(0, tr("CCCD(通知/指示开关)"));
    cccd->setText(1, "00002902-0000-1000-8000-00805f9b34fb");
    cccd->setText(2, tr("读取|写入"));
    cccd->setData(0, Qt::UserRole, "descriptor");

    if (charIndex == 0) {
        auto* udesc = new QTreeWidgetItem(charItem);
        udesc->setText(0, tr("特征用户描述"));
        udesc->setText(1, "00002901-0000-1000-8000-00805f9b34fb");
        udesc->setText(2, tr("读取"));
        udesc->setData(0, Qt::UserRole, "descriptor");
    }
}
