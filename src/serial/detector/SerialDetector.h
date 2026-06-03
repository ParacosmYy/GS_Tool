// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QSerialPortInfo>

/**
 * @brief 串口端口信息结构 — 封装系统串口设备的关键属性
 */
struct SerialPortInfo {
    QString portName;       ///< 端口名称(如"COM3")
    QString description;    ///< 设备描述
    QString manufacturer;   ///< 制造商名称
    QString serialNumber;   ///< 序列号
    QString systemLocation; ///< 系统设备路径
    quint16 vendorId = 0;   ///< USB厂商ID
    quint16 productId = 0;  ///< USB产品ID
    bool isAvailable = false; ///< 是否可用
};

/**
 * @brief 串口检测器 — 基于定时轮询的串口热插拔检测
 *
 * 定时扫描系统可用串口，检测插入/移除事件并发射信号通知。
 * 支持按VID、描述和端口名称查询。
 */
class SerialDetector : public QObject {
    Q_OBJECT
public:
    /** @brief 构造串口检测器 @param parent 父对象 */
    explicit SerialDetector(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~SerialDetector() override;

    /** @brief 启动串口热插拔监控(最小间隔100ms) @param intervalMs 轮询间隔(毫秒) */
    void startMonitoring(int intervalMs = 1000);
    /** @brief 停止串口热插拔监控 */
    void stopMonitoring();
    /** @brief 获取当前所有已知可用端口信息列表 @return SerialPortInfo列表 */
    QList<SerialPortInfo> availablePorts() const;
    /** @brief 获取所有已知端口名称列表 @return 端口名称列表 */
    QStringList portNames() const;
    /** @brief 查询是否正在监控 @return true=监控中 */
    bool isMonitoring() const;

    /** @brief 按USB厂商ID(VID)查找端口 @param vid USB厂商ID @return 匹配的SerialPortInfo列表 */
    QList<SerialPortInfo> findByVendorId(quint16 vid) const;
    /** @brief 按设备描述关键词查找端口(大小写不敏感) @param keyword 搜索关键词 @return 匹配的SerialPortInfo列表 */
    QList<SerialPortInfo> findByDescription(const QString &keyword) const;
    /** @brief 按端口名称精确查找端口信息 @param name 端口名称(如"COM3") @return 匹配的SerialPortInfo，未找到返回空对象 */
    SerialPortInfo findByPortName(const QString &name) const;

signals:
    /** @brief 端口插入信号 @param info 新插入端口信息 */
    void portInserted(const SerialPortInfo &info);
    /** @brief 端口移除信号 @param info 被移除端口信息 */
    void portRemoved(const SerialPortInfo &info);
    /** @brief 端口列表变更信号 @param current 当前所有端口信息列表 */
    void portsChanged(const QList<SerialPortInfo> &current);
    /** @brief 监控状态变更信号 @param active true=已启动 false=已停止 */
    void monitoringChanged(bool active);

private:
    /** @brief 刷新端口列表，检测插入/移除事件并发射对应信号 */
    void refreshPorts();
    /** @brief 将QSerialPortInfo转换为项目内部SerialPortInfo结构 @param info Qt串口信息对象 @return 内部SerialPortInfo结构 */
    SerialPortInfo fromQtInfo(const QSerialPortInfo &info) const;

    QTimer m_timer;                         ///< 轮询定时器
    QMap<QString, SerialPortInfo> m_knownPorts; ///< 已知端口映射(端口名→信息)
    bool m_monitoring = false;              ///< 监控状态标志
};
