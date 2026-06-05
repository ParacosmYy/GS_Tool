/**
 * @file BleScanWidget.h
 * @brief BLE扫描控件 — 设备列表、信号强度条和扫描控制界面
 *
 * 职责: QTreeWidget展示设备列表(名称/地址/RSSI条/类型)，
 * 扫描控制按钮，按RSSI/名称排序，点击展开服务UUID列表。
 */
#ifndef BLESCANWIDGET_H
#define BLESCANWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>

#include "connection/ble_scanner/BleScanTypes.h"

class BleDeviceScanner;

/**
 * @brief BLE扫描可视化控件
 *
 * 包含: 设备列表(QTreeWidget)、扫描/停止按钮、排序选择、设备计数标签。
 * 点击设备项可展开其服务UUID列表。RSSI以信号条形式可视化。
 */
class BleScanWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造BLE扫描控件 @param parent 父控件 */
    explicit BleScanWidget(QWidget* parent = nullptr);

    /** @brief 设置扫描器实例 @param scanner BleDeviceScanner对象指针 */
    void setScanner(BleDeviceScanner* scanner);

    /** @brief 获取当前选中的设备信息 @return 选中的BleDeviceInfo，无选中返回无效对象 */
    BleDeviceInfo selectedDevice() const;

    // ---- 统计接口 ----
    /** @brief 获取累计UI刷新次数 */
    quint64 totalUiRefreshes() const { return m_totalUiRefreshes; }
    /** @brief 获取累计设备选择次数 */
    quint64 totalDeviceSelections() const { return m_totalDeviceSelections; }
    /** @brief 获取累计排序变更次数 */
    quint64 totalSortChanges() const { return m_totalSortChanges; }
    /** @brief 获取累计扫描触发次数 */
    quint64 totalScanTriggers() const { return m_totalScanTriggers; }
    /** @brief 重置UI统计计数器 */
    void resetWidgetStatistics();

signals:
    /** @brief 用户选中了一个设备 */
    void deviceSelected(const BleDeviceInfo& info);

private slots:
    void onStartScan();         ///< 扫描按钮点击
    void onStopScan();          ///< 停止按钮点击
    void onDeviceFound(const BleDeviceInfo& info);   ///< 新设备发现
    void onDeviceUpdated(const BleDeviceInfo& info); ///< 设备信息更新
    void onScanComplete();      ///< 扫描完成
    void onSortChanged(int index); ///< 排序方式变更
    void onItemClicked(QTreeWidgetItem* item, int column); ///< 设备列表点击

private:
    void setupUI();             ///< 构建UI布局
    void updateDeviceRow(QTreeWidgetItem* item,
                         const BleDeviceInfo& info); ///< 更新设备行数据
    QString rssiToBar(int rssi) const;    ///< RSSI值转信号强度条字符串
    QString deviceTypeStr(BleDeviceType type) const; ///< 设备类型转显示字符串
    void refreshFullList();     ///< 全量刷新设备列表

    // ---- UI控件 ----
    QTreeWidget* m_treeWidget;
    QPushButton* m_startBtn;
    QPushButton* m_stopBtn;
    QComboBox*   m_sortCombo;
    QLabel*      m_countLabel;
    QLabel*      m_statusLabel;

    BleDeviceScanner* m_scanner = nullptr;

    // ---- 统计计数器 ----
    quint64 m_totalUiRefreshes = 0;
    quint64 m_totalDeviceSelections = 0;
    quint64 m_totalSortChanges = 0;
    quint64 m_totalScanTriggers = 0;
};

#endif // BLESCANWIDGET_H
