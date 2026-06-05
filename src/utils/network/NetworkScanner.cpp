/**
 * @file NetworkScanner.cpp
 * @brief 网络设备扫描引擎实现 - IP 扫描与设备发现
 */

#include "utils/network/NetworkScanner.h"

#include <QNetworkInterface>
#include <QProcess>
#include <QElapsedTimer>
#include <QTcpSocket>
#include <QRegularExpression>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造扫描引擎 @param parent 父对象 */
NetworkScanner::NetworkScanner(QObject *parent) : QObject(parent) {}

/** @brief 析构，停止可能残留的扫描 */
NetworkScanner::~NetworkScanner() { stopScan(); }

// ============================================================================
// 配置接口
// ============================================================================

/**
 * @brief 设置 IP 扫描范围
 * @param startIp 起始 IPv4 地址
 * @param endIp   结束 IPv4 地址
 */
void NetworkScanner::setScanRange(const QString &startIp, const QString &endIp) {
    m_startIp = startIp;
    m_endIp   = endIp;
}

/**
 * @brief 设置扫描类型
 * @param type 扫描模式
 */
void NetworkScanner::setScanType(ScanType type) { m_scanType = type; }

/** @brief 获取当前扫描类型 @return 扫描类型枚举 */
ScanType NetworkScanner::scanType() const { return m_scanType; }

/**
 * @brief 设置端口扫描范围
 * @param start 起始端口号
 * @param end   结束端口号
 */
void NetworkScanner::setPortRange(int start, int end) {
    m_portStart = start;
    m_portEnd   = end;
}

/**
 * @brief 设置单次探测超时
 * @param timeoutMs 超时毫秒数
 */
void NetworkScanner::setTimeout(int timeoutMs) { m_timeoutMs = timeoutMs; }

// ============================================================================
// 扫描控制
// ============================================================================

/** @brief 启动异步扫描任务，按当前配置的扫描类型分发 */
void NetworkScanner::startScan() {
    if (m_scanning) return;
    m_scanning = true;
    m_progress = 0;
    m_devices.clear();
    ++m_totalScans;
    emit scanStarted();

    switch (m_scanType) {
    case ScanType::Ping:    doPingScan();    break;
    case ScanType::Arp:     doArpScan();     break;
    case ScanType::Port:    doPortScan();    break;
    case ScanType::Service: doServiceScan(); break;
    }
}

/** @brief 中止正在进行的扫描 */
void NetworkScanner::stopScan() {
    if (!m_scanning) return;
    m_scanning = false;
    m_progress = 0;
}

/** @brief 查询扫描是否正在进行 @return true 正在扫描 */
bool NetworkScanner::isScanning() const { return m_scanning; }

/** @brief 获取已发现设备列表 @return 去重后的设备列表 */
QVector<NetworkDevice> NetworkScanner::discoveredDevices() const { return m_devices; }

// ============================================================================
// 本机接口枚举
// ============================================================================

/** @brief 枚举本机网络接口 @return 接口信息列表 */
QVector<NetworkInterface> NetworkScanner::localInterfaces() const {
    QVector<NetworkInterface> result;
    const auto ifaces = QNetworkInterface::allInterfaces();
    for (const auto &iface : ifaces) {
        const auto addrs = iface.addressEntries();
        for (const auto &addr : addrs) {
            if (addr.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
            NetworkInterface ni;
            ni.name       = iface.name();
            ni.ipAddress  = addr.ip().toString();
            ni.subnetMask = addr.netmask().toString();
            ni.macAddress = iface.hardwareAddress();
            ni.isUp       = (iface.flags() & QNetworkInterface::IsUp) != 0;
            result.append(ni);
        }
    }
    return result;
}

/** @brief 获取当前扫描进度 @return 百分比 0~100 */
int NetworkScanner::progress() const { return m_progress; }

// ============================================================================
// Ping 扫描（QTcpSocket 连接测试模拟）
// ============================================================================

/** @brief 执行 Ping 扫描：逐 IP 用 TCP 80 端口做连通性探测 */
void NetworkScanner::doPingScan() {
    const quint32 start = ipToInt(m_startIp);
    const quint32 end   = ipToInt(m_endIp);
    if (start == 0 || end == 0 || start > end) {
        emit scanError(tr("Invalid IP range"));
        m_scanning = false;
        return;
    }

    const quint32 total = end - start + 1;
    quint32 scanned = 0;

    for (quint32 ip = start; ip <= end && m_scanning; ++ip) {
        const QString ipStr = intToIp(ip);
        const qint64 rtt = probeHost(ipStr, 80, m_timeoutMs);
        ++m_totalIpsScanned;
        ++scanned;

        if (rtt >= 0) {
            NetworkDevice dev;
            dev.ipAddress     = ipStr;
            dev.isOnline      = true;
            dev.responseTimeMs = rtt;
            dev.lastSeen      = QDateTime::currentDateTime();
            tryAddDevice(dev);
        }

        m_progress = static_cast<int>((scanned * 100) / total);
        emit scanProgress(m_progress);
    }

    m_scanning = false;
    emit scanComplete(m_devices);
}

// ============================================================================
// ARP 表扫描
// ============================================================================

/** @brief 执行 ARP 扫描：解析系统 ARP 缓存表 */
void NetworkScanner::doArpScan() {
    parseArpTable();
    m_progress = 100;
    emit scanProgress(100);
    m_scanning = false;
    emit scanComplete(m_devices);
}

/** @brief 解析 Windows ARP 表 (arp -a) 并更新设备列表 */
void NetworkScanner::parseArpTable() {
    QProcess proc;
    proc.start("arp", QStringList{"-a"});
    if (!proc.waitForFinished(5000)) {
        ++m_totalErrors;
        return;
    }

    const QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    // Windows arp -a 格式: "  192.168.1.1   aa-bb-cc-dd-ee-ff   dynamic"
    static const QRegularExpression re(
        R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\s+([\da-fA-F]{2}[:-][\da-fA-F]{2}[:-][\da-fA-F]{2}[:-][\da-fA-F]{2}[:-][\da-fA-F]{2}[:-][\da-fA-F]{2}))");

    QRegularExpressionMatchIterator it = re.globalMatch(output);
    while (it.hasNext()) {
        const auto match = it.next();
        NetworkDevice dev;
        dev.ipAddress  = match.captured(1);
        dev.macAddress = match.captured(2);
        dev.isOnline   = true;
        dev.lastSeen   = QDateTime::currentDateTime();
        ++m_totalIpsScanned;
        tryAddDevice(dev);
    }
}

// ============================================================================
// 端口扫描
// ============================================================================

/** @brief 执行端口扫描：对每个 IP 逐端口 TCP 探测 */
void NetworkScanner::doPortScan() {
    const quint32 start = ipToInt(m_startIp);
    const quint32 end   = ipToInt(m_endIp);
    if (start == 0 || end == 0 || start > end) {
        emit scanError(tr("Invalid IP range"));
        m_scanning = false;
        return;
    }

    const quint32 ipCount = end - start + 1;
    const int portCount   = m_portEnd - m_portStart + 1;
    const quint64 total   = ipCount * portCount;
    quint64 scanned = 0;

    for (quint32 ip = start; ip <= end && m_scanning; ++ip) {
        const QString ipStr = intToIp(ip);
        bool anyOpen = false;

        for (int port = m_portStart; port <= m_portEnd && m_scanning; ++port) {
            const qint64 rtt = probeHost(ipStr, port, m_timeoutMs);
            ++m_totalPortsScanned;
            ++scanned;

            if (rtt >= 0) {
                anyOpen = true;
                NetworkDevice dev;
                dev.ipAddress     = ipStr;
                dev.port          = port;
                dev.isOnline      = true;
                dev.responseTimeMs = rtt;
                dev.lastSeen      = QDateTime::currentDateTime();
                tryAddDevice(dev);
            }
        }

        ++m_totalIpsScanned;
        if (total > 0) {
            m_progress = static_cast<int>((scanned * 100) / total);
            emit scanProgress(m_progress);
        }
    }

    m_scanning = false;
    emit scanComplete(m_devices);
}

// ============================================================================
// 服务识别扫描
// ============================================================================

/** @brief 执行服务扫描：对常见端口做服务指纹识别 */
void NetworkScanner::doServiceScan() {
    static const struct { int port; QString name; } services[] = {
        { 21, "FTP" },  { 22, "SSH" },  { 23, "Telnet" },
        { 25, "SMTP" }, { 53, "DNS" },  { 80, "HTTP" },
        { 110,"POP3" }, { 143,"IMAP" }, { 443,"HTTPS" },
        { 993,"IMAPS" },{ 995,"POP3S" },{ 3306,"MySQL" },
        { 5432,"PostgreSQL" }, { 8080,"HTTP-Alt" }, { 8443,"HTTPS-Alt" },
    };

    const quint32 start = ipToInt(m_startIp);
    const quint32 end   = ipToInt(m_endIp);
    if (start == 0 || end == 0 || start > end) {
        emit scanError(tr("Invalid IP range"));
        m_scanning = false;
        return;
    }

    const int svcCount = sizeof(services) / sizeof(services[0]);
    const quint64 total = (end - start + 1) * svcCount;
    quint64 scanned = 0;

    for (quint32 ip = start; ip <= end && m_scanning; ++ip) {
        const QString ipStr = intToIp(ip);

        for (int i = 0; i < svcCount && m_scanning; ++i) {
            const qint64 rtt = probeHost(ipStr, services[i].port, m_timeoutMs);
            ++m_totalPortsScanned;
            ++scanned;

            if (rtt >= 0) {
                NetworkDevice dev;
                dev.ipAddress     = ipStr;
                dev.port          = services[i].port;
                dev.service       = services[i].name;
                dev.isOnline      = true;
                dev.responseTimeMs = rtt;
                dev.lastSeen      = QDateTime::currentDateTime();
                tryAddDevice(dev);
            }
        }

        ++m_totalIpsScanned;
        if (total > 0) {
            m_progress = static_cast<int>((scanned * 100) / total);
            emit scanProgress(m_progress);
        }
    }

    m_scanning = false;
    emit scanComplete(m_devices);
}

// ============================================================================
// 内部工具方法
// ============================================================================

/**
 * @brief TCP 连接探测单台主机
 * @param ip   目标 IP
 * @param port 目标端口
 * @param timeoutMs 超时毫秒数
 * @return 响应时间(ms)，失败返回 -1
 */
qint64 NetworkScanner::probeHost(const QString &ip, int port, int timeoutMs) {
    QTcpSocket socket;
    QElapsedTimer timer;
    timer.start();

    socket.connectToHost(ip, port);
    if (!socket.waitForConnected(timeoutMs)) {
        if (socket.error() == QAbstractSocket::SocketTimeoutError) {
            ++m_totalTimeouts;
        } else if (socket.error() != QAbstractSocket::ConnectionRefusedError) {
            ++m_totalErrors;
        }
        socket.abort();
        return -1;
    }

    const qint64 elapsed = timer.elapsed();
    socket.abort();
    return elapsed;
}

/**
 * @brief IPv4 字符串转 quint32 网络序整数
 * @param ip IPv4 字符串
 * @return 网络序整数，无效返回 0
 */
quint32 NetworkScanner::ipToInt(const QString &ip) {
    const QStringList parts = ip.split('.');
    if (parts.size() != 4) return 0;
    quint32 result = 0;
    for (const auto &part : parts) {
        bool ok = false;
        const quint32 octet = part.toUInt(&ok);
        if (!ok || octet > 255) return 0;
        result = (result << 8) | octet;
    }
    return result;
}

/**
 * @brief quint32 网络序整数转 IPv4 字符串
 * @param value 网络序整数
 * @return IPv4 字符串
 */
QString NetworkScanner::intToIp(quint32 value) {
    return QString("%1.%2.%3.%4")
        .arg((value >> 24) & 0xFF)
        .arg((value >> 16) & 0xFF)
        .arg((value >>  8) & 0xFF)
        .arg(value & 0xFF);
}

/**
 * @brief 尝试将设备加入列表（IP 去重）
 * @param device 待添加设备
 */
void NetworkScanner::tryAddDevice(const NetworkDevice &device) {
    for (const auto &existing : m_devices) {
        if (existing.ipAddress == device.ipAddress) return;
    }
    m_devices.append(device);
    ++m_totalDevicesFound;
    emit deviceFound(device);
}
