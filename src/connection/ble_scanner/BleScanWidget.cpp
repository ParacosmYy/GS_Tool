/**
 * @file BleScanWidget.cpp
 * @brief BLE扫描控件实现 — UI布局、设备列表管理、信号连接与交互
 *
 * QTreeWidget展示设备列表，4列: 名称、地址、信号强度条、类型。
 * 支持按RSSI/名称排序，点击展开服务UUID子项。
 * 统计getter/resetWidgetStatistics 见 BleScanWidgetStats.cpp。
 */

#include "connection/ble_scanner/BleScanWidget.h"
#include "connection/ble_scanner/BleDeviceScanner.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <algorithm>

/** @brief 构造BLE扫描控件，初始化UI布局 @param parent 父控件 */
BleScanWidget::BleScanWidget(QWidget* parent)
    : QWidget(parent)
    , m_treeWidget(new QTreeWidget(this))
    , m_startBtn(new QPushButton(tr("开始扫描"), this))
    , m_stopBtn(new QPushButton(tr("停止"), this))
    , m_sortCombo(new QComboBox(this))
    , m_countLabel(new QLabel(tr("设备: 0"), this))
    , m_statusLabel(new QLabel(tr("就绪"), this))
{
    setupUI();
}

/** @brief 构建UI布局: 工具栏 + 设备列表 + 状态栏 */
void BleScanWidget::setupUI()
{
    setObjectName("BleScanWidget");

    /* 控件命名 */
    m_treeWidget->setObjectName("treeBleDevices");
    m_startBtn->setObjectName("btnBleStartScan");
    m_stopBtn->setObjectName("btnBleStopScan");
    m_sortCombo->setObjectName("comboBleSort");
    m_countLabel->setObjectName("labelBleCount");
    m_statusLabel->setObjectName("labelBleStatus");

    /* 停止按钮默认禁用 */
    m_stopBtn->setEnabled(false);

    /* 排序下拉框 */
    m_sortCombo->addItem(tr("按信号强度"), 0);
    m_sortCombo->addItem(tr("按名称"), 1);
    m_sortCombo->addItem(tr("按地址"), 2);

    /* TreeWidget 列配置 */
    m_treeWidget->setHeaderLabels({
        tr("设备名称"), tr("地址"), tr("信号强度"), tr("类型")
    });
    m_treeWidget->header()->setStretchLastSection(true);
    m_treeWidget->setColumnWidth(0, 140);
    m_treeWidget->setColumnWidth(1, 140);
    m_treeWidget->setColumnWidth(2, 100);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setSortingEnabled(false);

    /* 工具栏布局 */
    auto toolbar = new QHBoxLayout();
    toolbar->addWidget(m_startBtn);
    toolbar->addWidget(m_stopBtn);
    toolbar->addSpacing(16);
    toolbar->addWidget(new QLabel(tr("排序:"), this));
    toolbar->addWidget(m_sortCombo);
    toolbar->addStretch();
    toolbar->addWidget(m_countLabel);

    /* 主布局 */
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->addLayout(toolbar);
    mainLayout->addWidget(m_treeWidget, 1);
    mainLayout->addWidget(m_statusLabel);

    /* 信号连接 */
    connect(m_startBtn, &QPushButton::clicked,
            this, &BleScanWidget::onStartScan);
    connect(m_stopBtn, &QPushButton::clicked,
            this, &BleScanWidget::onStopScan);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BleScanWidget::onSortChanged);
    connect(m_treeWidget, &QTreeWidget::itemClicked,
            this, &BleScanWidget::onItemClicked);
}

/** @brief 设置扫描器实例，连接deviceFound/deviceUpdated/scanComplete信号 @param scanner BleDeviceScanner对象 */
void BleScanWidget::setScanner(BleDeviceScanner* scanner)
{
    if (m_scanner) {
        disconnect(m_scanner, nullptr, this, nullptr);
        m_treeWidget->clear();
    }
    m_scanner = scanner;
    if (m_scanner) {
        connect(m_scanner, &BleDeviceScanner::deviceFound,
                this, &BleScanWidget::onDeviceFound);
        connect(m_scanner, &BleDeviceScanner::deviceUpdated,
                this, &BleScanWidget::onDeviceUpdated);
        connect(m_scanner, &BleDeviceScanner::scanComplete,
                this, &BleScanWidget::onScanComplete);
    }
}

/** @brief 获取当前选中设备 @return 选中设备的BleDeviceInfo */
BleDeviceInfo BleScanWidget::selectedDevice() const
{
    QTreeWidgetItem* cur = m_treeWidget->currentItem();
    if (!cur) { return BleDeviceInfo{}; }
    const QString addr = cur->data(1, Qt::UserRole).toString();
    if (m_scanner && !addr.isEmpty()) {
        return m_scanner->deviceByAddress(addr);
    }
    return BleDeviceInfo{};
}

/** @brief 扫描按钮点击，启动扫描并刷新UI状态 */
void BleScanWidget::onStartScan()
{
    if (!m_scanner) { return; }
    ++m_totalScanTriggers;
    m_treeWidget->clear();
    m_scanner->startScan();
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_statusLabel->setText(tr("扫描中..."));
}

/** @brief 停止按钮点击 */
void BleScanWidget::onStopScan()
{
    if (!m_scanner) { return; }
    m_scanner->stopScan();
}

/** @brief 新设备发现回调，在树中添加设备行 @param info 设备信息 */
void BleScanWidget::onDeviceFound(const BleDeviceInfo& info)
{
    auto* item = new QTreeWidgetItem(m_treeWidget);
    item->setData(1, Qt::UserRole, info.address);
    updateDeviceRow(item, info);

    /* 添加服务UUID子项 */
    for (const QString& uuid : info.serviceUuids) {
        auto* svcItem = new QTreeWidgetItem(item);
        svcItem->setText(0, tr("服务"));
        svcItem->setText(1, uuid);
        svcItem->setFirstColumnSpanned(false);
    }

    ++m_totalUiRefreshes;
    m_countLabel->setText(tr("设备: %1").arg(m_treeWidget->topLevelItemCount()));
}

/** @brief 设备信息更新回调(RSSI变化)，查找并更新对应行 @param info 更新后的设备信息 */
void BleScanWidget::onDeviceUpdated(const BleDeviceInfo& info)
{
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
        if (item->data(1, Qt::UserRole).toString() == info.address) {
            updateDeviceRow(item, info);
            break;
        }
    }
    ++m_totalUiRefreshes;
}

/** @brief 扫描完成回调，恢复按钮状态 */
void BleScanWidget::onScanComplete()
{
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_statusLabel->setText(
        tr("扫描完成 — 共 %1 个设备").arg(m_treeWidget->topLevelItemCount()));
}

/** @brief 排序方式变更，重新排列树控件 @param index 下拉框选中索引 */
void BleScanWidget::onSortChanged(int index)
{
    Q_UNUSED(index)
    ++m_totalSortChanges;
    refreshFullList();
}

/** @brief 设备列表点击回调，选中设备并发射deviceSelected信号 @param item 点击的树节点 @param column 点击的列 */
void BleScanWidget::onItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    if (!item) { return; }

    /* 仅对顶层设备行响应（非UUID子项） */
    if (item->parent()) { return; }

    const QString addr = item->data(1, Qt::UserRole).toString();
    if (addr.isEmpty()) { return; }

    ++m_totalDeviceSelections;
    if (m_scanner) {
        const BleDeviceInfo info = m_scanner->deviceByAddress(addr);
        if (info.isValid()) {
            emit deviceSelected(info);
        }
    }
}

/** @brief 更新设备行的文本和数据 @param item 树节点 @param info 设备信息 */
void BleScanWidget::updateDeviceRow(QTreeWidgetItem* item,
                                     const BleDeviceInfo& info)
{
    const QString name = info.name.isEmpty()
        ? tr("(未命名)") : info.name;
    item->setText(0, name);
    item->setText(1, info.address);
    item->setText(2, rssiToBar(info.rssi));
    item->setText(3, deviceTypeStr(info.type));

    /* RSSI着色: 绿=强、黄=中、红=弱 */
    if (info.rssi >= -50) {
        item->setForeground(2, QColor(76, 175, 80));
    } else if (info.rssi >= -70) {
        item->setForeground(2, QColor(255, 193, 7));
    } else {
        item->setForeground(2, QColor(244, 67, 54));
    }

    /* Tooltip显示详细信息 */
    item->setToolTip(0, tr("%1\n地址: %2\nRSSI: %3 dBm\n类型: %4")
        .arg(name, info.address)
        .arg(info.rssi)
        .arg(deviceTypeStr(info.type)));
}

/** @brief RSSI值转信号强度条字符串 @param rssi 信号强度dBm @return 可视化信号条 */
QString BleScanWidget::rssiToBar(int rssi) const
{
    int bars = 0;
    if (rssi >= -40)      { bars = 5; }
    else if (rssi >= -55) { bars = 4; }
    else if (rssi >= -65) { bars = 3; }
    else if (rssi >= -75) { bars = 2; }
    else if (rssi >= -85) { bars = 1; }

    QString bar;
    for (int i = 0; i < bars; ++i) {
        bar += QChar(0x2588); /* █ 全填充块 */
    }
    for (int i = bars; i < 5; ++i) {
        bar += QChar(0x2591); /* ░ 浅填充块 */
    }
    bar += QStringLiteral(" %1 dBm").arg(rssi);
    return bar;
}

/** @brief 设备类型枚举转显示字符串 @param type 设备类型 @return 类型名称 */
QString BleScanWidget::deviceTypeStr(BleDeviceType type) const
{
    switch (type) {
    case BleDeviceType::Peripheral: return tr("外围设备");
    case BleDeviceType::Central:    return tr("中心设备");
    default:                        return tr("未知");
    }
}

/** @brief 全量刷新设备列表（排序后重建） */
void BleScanWidget::refreshFullList()
{
    if (!m_scanner) { return; }

    QList<BleDeviceInfo> devs = m_scanner->discoveredDevices();
    const int sortIdx = m_sortCombo->currentData().toInt();

    switch (sortIdx) {
    case 0: /* RSSI降序 */
        std::sort(devs.begin(), devs.end(),
            [](const BleDeviceInfo& a, const BleDeviceInfo& b) {
                return a.rssi > b.rssi;
            });
        break;
    case 1: /* 名称升序 */
        std::sort(devs.begin(), devs.end(),
            [](const BleDeviceInfo& a, const BleDeviceInfo& b) {
                return a.name.toLower() < b.name.toLower();
            });
        break;
    case 2: /* 地址升序 */
        std::sort(devs.begin(), devs.end(),
            [](const BleDeviceInfo& a, const BleDeviceInfo& b) {
                return a.address < b.address;
            });
        break;
    default:
        break;
    }

    m_treeWidget->clear();
    for (const BleDeviceInfo& info : devs) {
        onDeviceFound(info);
    }
}

// resetWidgetStatistics 见 BleScanWidgetStats.cpp
