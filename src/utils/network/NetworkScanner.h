/**
 * @file NetworkScanner.h
 * @brief 网络设备扫描引擎 - IP 扫描与设备发现
 *
 * 职责:
 *   1. 支持 Ping / ARP / 端口 / 服务 四种扫描模式
 *   2. 异步逐 IP 探测，实时报告进度与发现设备
 *   3. 设备去重与统计计数
 *   4. 枚举本机网络接口信息
 */

#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <QTimer>
#include <QTcpSocket>

#include "utils/network/NetworkTypes.h"

/**
 * @brief 网络设备扫描引擎
 *
 * 通过 QTcpSocket 连接测试模拟 ICMP ping、解析系统 ARP 表、
 * TCP 端口扫描和服务识别，异步发现局域网设备。
 */
class NetworkScanner : public QObject {
    Q_OBJECT
public:
    /**
     * @brief 构造扫描引擎
     * @param parent 父对象
     */
    explicit NetworkScanner(QObject *parent = nullptr);

    /** @brief 析构函数，停止可能正在进行的扫描 */
    ~NetworkScanner() override;

    /**
     * @brief 设置 IP 扫描范围
     * @param startIp 起始 IPv4 地址
     * @param endIp   结束 IPv4 地址
     */
    void setScanRange(const QString &startIp, const QString &endIp);

    /**
     * @brief 设置扫描类型
     * @param type 扫描模式（Ping / Arp / Port / Service）
     */
    void setScanType(ScanType type);

    /** @brief 获取当前扫描类型 @return 扫描类型枚举 */
    ScanType scanType() const;

    /**
     * @brief 设置端口扫描范围（仅 Port / Service 模式生效）
     * @param start 起始端口号
     * @param end   结束端口号
     */
    void setPortRange(int start, int end);

    /**
     * @brief 设置单次连接超时时间
     * @param timeoutMs 超时毫秒数，默认 2000
     */
    void setTimeout(int timeoutMs);

    /** @brief 启动异步扫描任务 */
    void startScan();

    /** @brief 中止正在进行的扫描 */
    void stopScan();

    /** @brief 查询扫描是否正在进行 @return true 正在扫描 */
    bool isScanning() const;

    /** @brief 获取已发现设备列表 @return 设备列表（去重后） */
    QVector<NetworkDevice> discoveredDevices() const;

    /** @brief 枚举本机网络接口 @return 接口信息列表 */
    QVector<NetworkInterface> localInterfaces() const;

    /** @brief 获取当前扫描进度 @return 百分比 0~100 */
    int progress() const;

    // ---- 统计接口 ----
    /** @brief 获取累计扫描次数 @return 计数 */
    quint64 totalScans() const;
    /** @brief 获取累计发现设备数 @return 计数 */
    quint64 totalDevicesFound() const;
    /** @brief 获取累计扫描 IP 数 @return 计数 */
    quint64 totalIpsScanned() const;
    /** @brief 获取累计扫描端口数 @return 计数 */
    quint64 totalPortsScanned() const;
    /** @brief 获取累计超时次数 @return 计数 */
    quint64 totalTimeouts() const;
    /** @brief 获取累计错误次数 @return 计数 */
    quint64 totalErrors() const;
    /** @brief 重置所有扫描统计计数器 */
    void resetStatistics();

signals:
    /** @brief 扫描任务启动 */
    void scanStarted();
    /** @brief 扫描进度更新 @param percent 0~100 */
    void scanProgress(int percent);
    /** @brief 发现新设备 @param device 设备信息 */
    void deviceFound(const NetworkDevice &device);
    /** @brief 扫描完成 @param devices 全部已发现设备 */
    void scanComplete(const QVector<NetworkDevice> &devices);
    /** @brief 扫描出错 @param error 错误描述 */
    void scanError(const QString &error);

private:
    /** @brief 执行 Ping 扫描流程 */
    void doPingScan();
    /** @brief 执行 ARP 表扫描 */
    void doArpScan();
    /** @brief 执行端口扫描 */
    void doPortScan();
    /** @brief 执行服务识别扫描 */
    void doServiceScan();

    /**
     * @brief 对单个 IP 做 TCP 连接探测
     * @param ip   目标 IPv4 地址
     * @param port 目标端口
     * @param timeoutMs 超时毫秒数
     * @return 响应时间(ms)，失败返回 -1
     */
    qint64 probeHost(const QString &ip, int port, int timeoutMs);

    /** @brief 解析系统 ARP 表并更新设备列表 */
    void parseArpTable();

    /**
     * @brief 将 IPv4 地址转为 quint32
     * @param ip IPv4 字符串
     * @return 网络序整数，无效返回 0
     */
    static quint32 ipToInt(const QString &ip);

    /**
     * @brief 将 quint32 转为 IPv4 字符串
     * @param value 网络序整数
     * @return IPv4 字符串
     */
    static QString intToIp(quint32 value);

    /** @brief 尝试将新设备加入列表（去重） */
    void tryAddDevice(const NetworkDevice &device);

    ScanType  m_scanType = ScanType::Ping;     ///< 当前扫描类型
    QString   m_startIp;                       ///< 起始 IP
    QString   m_endIp;                         ///< 结束 IP
    int       m_portStart = 1;                 ///< 起始端口
    int       m_portEnd = 1024;                ///< 结束端口
    int       m_timeoutMs = 2000;              ///< 单次探测超时(ms)
    bool      m_scanning = false;              ///< 是否正在扫描
    int       m_progress = 0;                  ///< 当前进度百分比
    QVector<NetworkDevice> m_devices;          ///< 已发现设备列表

    // ---- 统计计数器 ----
    quint64 m_totalScans = 0;        ///< 累计扫描次数
    quint64 m_totalDevicesFound = 0; ///< 累计发现设备数
    quint64 m_totalIpsScanned = 0;   ///< 累计扫描 IP 数
    quint64 m_totalPortsScanned = 0; ///< 累计扫描端口数
    quint64 m_totalTimeouts = 0;     ///< 累计超时次数
    quint64 m_totalErrors = 0;       ///< 累计错误次数
};
