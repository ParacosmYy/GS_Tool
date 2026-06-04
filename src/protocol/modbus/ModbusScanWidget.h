/**
 * @file ModbusScanWidget.h
 * @brief Modbus从站地址扫描器 — 自动扫描总线上的从站设备
 *
 * 逐个地址发送探测请求，收集响应的从站列表。
 * 提供进度条、地址范围输入、结果表格（地址/设备ID/类型）、统计面板。
 */
#ifndef MODBUS_SCAN_WIDGET_H
#define MODBUS_SCAN_WIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include "protocol/modbus/ModbusMaster.h"

/**
 * @brief Modbus从站地址扫描器控件
 * 逐地址发送探测请求，发现总线上存在的从站设备。
 * 结果以表格形式展示：从站地址、设备ID、设备类型、响应功能码。
 */
class ModbusScanWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造Modbus从站地址扫描器 @param parent 父控件指针 */
    explicit ModbusScanWidget(QWidget* parent = nullptr);

    /** @brief 设置Modbus主站实例 @param master ModbusMaster指针，用于发送探测请求 */
    void setModbusMaster(ModbusMaster* master);

    /**
     * @brief 开始扫描指定地址范围
     * @param from 起始地址（含）
     * @param to 结束地址（含）
     */
    void startScan(int from, int to);

    /** @brief 停止正在进行的扫描 */
    void stopScan();

    /**
     * @brief 获取已发现的从站地址列表
     * @return 从站地址列表
     */
    QList<int> foundSlaves() const;

    // ---- 统计接口 ----

    /** @brief 获取累计发起扫描次数 */
    quint64 totalScansInitiated() const;

    /** @brief 获取累计发现的从站总数 */
    quint64 totalSlavesFound() const;

    /** @brief 获取累计成功响应次数 */
    quint64 totalSuccessfulProbes() const;

    /** @brief 获取累计超时次数 */
    quint64 totalTimeouts() const;

    /** @brief 获取累计异常响应次数 */
    quint64 totalErrors() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 发现新的从站设备 */
    void slaveFound(int address, const QString& description);

    /** @brief 扫描进度更新（当前扫描地址） */
    void scanProgress(int address);

    /** @brief 扫描完成 */
    void scanCompleted();

private slots:
    /** @brief 处理扫描响应（解析设备信息并添加到表格） */
    void onResponseReceived(const ModbusFrame& frame);

    /** @brief 处理扫描超时（跳过当前地址，继续下一个） */
    void onScanTimeout(int slave, int function);

    /** @brief 处理Modbus异常响应 */
    void onScanError(ModbusError errorCode);

private:
    /** @brief 扫描下一个地址 */
    void scanNext();

    /** @brief 更新按钮状态和文字 */
    void updateScanButtonState(bool scanning);

    /** @brief 根据功能码推断设备类型描述 */
    static QString inferDeviceType(const ModbusFrame& frame);

    /** @brief 更新统计标签文字 */
    void updateStatsLabel();

    ModbusMaster*   m_master       = nullptr; ///< Modbus主站实例
    QSpinBox*       m_fromSpin     = nullptr; ///< 扫描起始地址
    QSpinBox*       m_toSpin       = nullptr; ///< 扫描结束地址
    QComboBox*      m_probeFunc    = nullptr; ///< 探测功能码选择
    QProgressBar*   m_progressBar  = nullptr; ///< 扫描进度条
    QTableWidget*   m_resultTable  = nullptr; ///< 扫描结果表格
    QPushButton*    m_scanBtn      = nullptr; ///< 开始/停止扫描按钮
    QLabel*         m_statsLabel   = nullptr; ///< 统计信息标签

    int  m_scanFrom     = 1;    ///< 扫描起始地址
    int  m_scanTo       = 247;  ///< 扫描结束地址
    int  m_currentAddr  = 1;    ///< 当前扫描地址
    bool m_scanning     = false; ///< 是否正在扫描

    // ---- 统计计数器 ----
    quint64 m_totalScansInitiated  = 0; ///< 累计发起扫描次数
    quint64 m_totalSlavesFound     = 0; ///< 累计发现从站总数
    quint64 m_totalSuccessfulProbes = 0; ///< 累计成功探测次数
    quint64 m_totalTimeouts        = 0; ///< 累计超时次数
    quint64 m_totalErrors          = 0; ///< 累计异常响应次数
};

#endif // MODBUS_SCAN_WIDGET_H
