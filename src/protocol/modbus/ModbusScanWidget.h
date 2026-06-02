/**
 * @file ModbusScanWidget.h
 * @brief Modbus从站地址扫描器 — 自动扫描总线上的从站设备
 *
 * 逐个地址发送探测请求，收集响应的从站列表。
 * 提供进度条和结果列表的可视化界面。
 */
#ifndef MODBUS_SCAN_WIDGET_H
#define MODBUS_SCAN_WIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include "protocol/modbus/ModbusMaster.h"

/**
 * @brief Modbus从站地址扫描器控件
 * 逐地址发送探测请求，发现总线上存在的从站设备。
 */
class ModbusScanWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModbusScanWidget(QWidget* parent = nullptr);

    /** @brief 设置Modbus主站实例 */
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

signals:
    /** @brief 发现新的从站设备 */
    void slaveFound(int address, QString description);

    /** @brief 扫描进度更新（当前扫描地址） */
    void scanProgress(int address);

    /** @brief 扫描完成 */
    void scanCompleted();

private slots:
    /** @brief 处理扫描响应 */
    void onResponseReceived(const ModbusFrame& frame);

    /** @brief 处理扫描超时（继续下一个地址） */
    void onScanTimeout(int slave, int function);

private:
    /** @brief 扫描下一个地址 */
    void scanNext();

    /** @brief 更新按钮状态和文字 */
    void updateScanButtonState(bool scanning);

    ModbusMaster*  m_master       = nullptr; ///< Modbus主站实例
    QProgressBar*  m_progressBar  = nullptr; ///< 扫描进度条
    QListWidget*   m_resultList   = nullptr; ///< 扫描结果列表
    QPushButton*   m_scanBtn      = nullptr; ///< 开始/停止扫描按钮

    int  m_scanFrom     = 1;    ///< 扫描起始地址
    int  m_scanTo       = 247;  ///< 扫描结束地址
    int  m_currentAddr  = 1;    ///< 当前扫描地址
    bool m_scanning     = false; ///< 是否正在扫描
};

#endif // MODBUS_SCAN_WIDGET_H
