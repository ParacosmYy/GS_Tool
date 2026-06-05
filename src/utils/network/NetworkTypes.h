/**
 * @file NetworkTypes.h
 * @brief 网络拓扑发现模块 - 公共数据类型定义
 *
 * 职责:
 *   1. 定义网络设备描述结构体 (NetworkDevice)
 *   2. 定义本机网络接口描述结构体 (NetworkInterface)
 *   3. 定义扫描类型枚举 (ScanType)
 */

#pragma once
#include <QString>
#include <QDateTime>
#include <QMetaType>

/**
 * @brief 扫描类型枚举
 *
 * 控制网络扫描引擎的工作模式。
 */
enum class ScanType {
    Ping,    ///< ICMP 连接测试模拟（QTcpSocket 连通性探测）
    Arp,     ///< ARP 表读取（Windows: arp -a 命令解析）
    Port,    ///< TCP 端口扫描（QTcpSocket 逐端口探测）
    Service  ///< 服务识别扫描（端口 + 常见服务指纹匹配）
};

/** @brief 向 Qt 元类型系统注册 ScanType */
Q_DECLARE_METATYPE(ScanType)

/**
 * @brief 网络设备描述结构体
 *
 * 存储扫描发现的一台网络设备的全部已知信息。
 */
struct NetworkDevice {
    QString ipAddress;       ///< IPv4 地址，如 "192.168.1.100"
    QString macAddress;      ///< MAC 地址，如 "AA:BB:CC:DD:EE:FF"
    QString hostname;        ///< 主机名（反向 DNS 或 NetBIOS）
    QString vendor;          ///< 网卡厂商标识（OUI 查询结果）
    int     port = 0;        ///< 发现的开放端口号，0 表示未检测
    QString service;         ///< 端口对应的服务名称，如 "HTTP"
    bool    isOnline = false;///< 设备在线状态
    qint64  responseTimeMs = 0; ///< 响应时间（毫秒）
    QDateTime lastSeen;      ///< 最后发现时间

    /** @brief 默认构造 */
    NetworkDevice() = default;

    /**
     * @brief 以 IP 地址作为唯一性判据
     * @param other 另一个设备
     * @return true 两者 IP 相同
     */
    bool operator==(const NetworkDevice &other) const {
        return ipAddress == other.ipAddress;
    }
};

/** @brief 向 Qt 元类型系统注册 NetworkDevice */
Q_DECLARE_METATYPE(NetworkDevice)

/**
 * @brief 本机网络接口描述结构体
 *
 * 存储本机一块网卡（或虚拟接口）的地址信息。
 */
struct NetworkInterface {
    QString name;         ///< 接口名称，如 "eth0"、"WLAN"
    QString ipAddress;    ///< 接口 IPv4 地址
    QString subnetMask;   ///< 子网掩码，如 "255.255.255.0"
    QString macAddress;   ///< 接口 MAC 地址
    bool    isUp = false; ///< 接口是否处于 UP 状态

    /** @brief 默认构造 */
    NetworkInterface() = default;

    /**
     * @brief 以 IP 地址作为唯一性判据
     * @param other 另一个接口
     * @return true 两者 IP 相同
     */
    bool operator==(const NetworkInterface &other) const {
        return ipAddress == other.ipAddress;
    }
};

/** @brief 向 Qt 元类型系统注册 NetworkInterface */
Q_DECLARE_METATYPE(NetworkInterface)
