/** @file SerialDetector.h @brief 串口检测器 - 增强版设备检测。支持USB VID/PID详细信息、友好名称和驱动识别。定时轮询检测插入/移除事件 */

#ifndef SERIALDETECTOR_H
#define SERIALDETECTOR_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QHash>
#include <QSerialPortInfo>

/** @brief 已知USB转串口芯片厂商定义(通过VID匹配) */
struct UsbVendorEntry { quint16 vid; QString vendorName; QString commonChip; };

/** @brief 串口端口信息结构 - 封装系统串口设备关键属性(扩展了友好名称/驱动类型/芯片识别) */
struct SerialPortInfo {
    QString portName;       ///< 端口名称(如"COM3")
    QString description;    ///< 设备描述(QSerialPortInfo::description)
    QString manufacturer;   ///< 制造商名称
    QString serialNumber;   ///< 序列号
    QString systemLocation; ///< 系统设备路径
    quint16 vendorId = 0;   ///< USB厂商ID(VID)
    quint16 productId = 0;  ///< USB产品ID(PID)
    bool isAvailable = false; ///< 是否可用

    // ---- 增强字段 ----
    QString friendlyName;   ///< 友好名称(如"CH340 (COM3)")，自动生成
    QString driverType;     ///< 驱动类型(如"CH340"/"CP2102"/"FT232"/"PL2303"/"Unknown")
    QString chipModel;      ///< 芯片型号(基于VID/PID匹配，如"CH340G"/"CP2102N")
    QString vidHex;         ///< VID十六进制字符串(如"1A86")
    QString pidHex;         ///< PID十六进制字符串(如"7523")

    QString toSummary() const;              ///< 生成设备详情摘要文本(tooltip/日志)
};

/** @brief 串口检测器 - 增强版设备检测。定时扫描串口，检测插入/移除事件。支持按VID/描述/端口名/制造商/驱动类型查询 */
class SerialDetector : public QObject {
    Q_OBJECT
public:
    explicit SerialDetector(QObject *parent = nullptr); ///< 构造
    ~SerialDetector() override;              ///< 析构
    // ---- 监控控制 ----
    void startMonitoring(int intervalMs = 1000); ///< 启动热插拔监控(最小100ms)
    void stopMonitoring();                   ///< 停止监控
    QList<SerialPortInfo> availablePorts() const; ///< 所有已知可用端口
    QStringList portNames() const;           ///< 所有已知端口名称
    bool isMonitoring() const;               ///< 是否正在监控
    // ---- 查询接口 ----
    QList<SerialPortInfo> findByVendorId(quint16 vid) const; ///< 按VID查找
    QList<SerialPortInfo> findByDescription(const QString &keyword) const; ///< 按描述关键词查找(大小写不敏感)
    SerialPortInfo findByPortName(const QString &name) const; ///< 按端口名称精确查找
    QList<SerialPortInfo> findByManufacturer(const QString &keyword) const; ///< 按制造商关键词查找
    QList<SerialPortInfo> findByDriverType(const QString &driverType) const; ///< 按驱动类型查找
    // ---- 芯片识别 ----
    static UsbVendorEntry lookupVendor(quint16 vid); ///< 按VID查找芯片厂商
    static QString identifyChip(quint16 vid, quint16 pid); ///< 按VID/PID识别芯片型号

    // ---- 统计计数器 ----
    quint64 totalScans() const { return m_totalScans; } ///< 累计端口扫描次数
    quint64 totalInsertions() const { return m_totalInsertions; } ///< 累计端口插入次数
    quint64 totalRemovals() const { return m_totalRemovals; } ///< 累计端口移除次数
    quint64 totalVidLookups() const { return m_totalVidLookups; } ///< 累计VID查询次数
    static quint64 totalChipIdentifications() { return s_totalChipIdentifications; } ///< 累计芯片识别次数
    int uniqueVidCount() const;             ///< 累计检测到的不同VID数量
    static int knownVendorCount();          ///< 已知芯片厂商数据库条目数
    void resetStatistics();                 ///< 重置所有统计计数器

signals:
    void portInserted(const SerialPortInfo &info); ///< 端口插入信号
    void portRemoved(const SerialPortInfo &info); ///< 端口移除信号
    void portsChanged(const QList<SerialPortInfo> &current); ///< 端口列表变更信号
    void monitoringChanged(bool active);     ///< 监控状态变更信号

private:
    void refreshPorts();                     ///< 刷新端口列表(检测插入/移除事件)
    SerialPortInfo fromQtInfo(const QSerialPortInfo &info) const; ///< QSerialPortInfo->内部SerialPortInfo
    static QString generateFriendlyName(const SerialPortInfo &info); ///< 自动生成友好名称
    static QString identifyDriverType(const QString &description, const QString &manufacturer); ///< 识别驱动类型

    QTimer m_timer;                         ///< 轮询定时器
    QMap<QString, SerialPortInfo> m_knownPorts; ///< 已知端口映射(端口名→信息)
    bool m_monitoring = false;              ///< 监控状态标志

    // ---- 统计计数器 ----
    quint64 m_totalScans = 0;       ///< 累计端口扫描次数
    quint64 m_totalInsertions = 0;  ///< 累计端口插入事件次数
    quint64 m_totalRemovals = 0;    ///< 累计端口移除事件次数
    mutable quint64 m_totalVidLookups = 0; ///< 累计VID查询次数
    static inline quint64 s_totalChipIdentifications = 0; ///< 累计芯片识别次数(identifyChip调用)

    // ---- 已知USB转串口芯片数据库 ----
    static const QVector<UsbVendorEntry> kKnownVendors; ///< 已知USB芯片厂商数据库
};

#endif // SERIALDETECTOR_H
