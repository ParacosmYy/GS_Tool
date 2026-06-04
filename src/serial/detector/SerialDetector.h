/** @file SerialDetector.h @brief 串口检测器 - 增强版设备检测。USB VID/PID详细信息、友好名称、驱动识别。定时轮询检测插入/移除事件 */
#ifndef SERIALDETECTOR_H
#define SERIALDETECTOR_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QHash>
#include <QSerialPortInfo>

/** @brief 已知USB转串口芯片厂商定义(通过VID匹配) */
struct UsbVendorEntry {
    quint16 vid;        ///< USB厂商ID
    QString vendorName; ///< 厂商名称(如"WCH"/"Silicon Labs")
    QString commonChip; ///< 常见芯片型号(如"CH340"/"CP2102")
};

/** @brief 串口端口信息结构 - 封装系统串口设备关键属性(扩展友好名称/驱动类型/芯片识别) */
struct SerialPortInfo {
    QString portName;       ///< 端口名称(如"COM3")
    QString description;    ///< 设备描述
    QString manufacturer;   ///< 制造商名称
    QString serialNumber;   ///< 序列号
    QString systemLocation; ///< 系统设备路径
    quint16 vendorId = 0;   ///< USB厂商ID(VID)
    quint16 productId = 0;  ///< USB产品ID(PID)
    bool isAvailable = false; ///< 是否可用
    QString friendlyName;   ///< 友好名称(如"CH340 (COM3)")，自动生成
    QString driverType;     ///< 驱动类型(如"CH340"/"CP2102"/"FT232"/"PL2303"/"Unknown")
    QString chipModel;      ///< 芯片型号(基于VID/PID匹配)
    QString vidHex;         ///< VID十六进制字符串(如"1A86")
    QString pidHex;         ///< PID十六进制字符串(如"7523")
    QString toSummary() const; ///< 生成设备详情摘要文本(tooltip/日志)
};

/** @brief 串口检测器 - 定时扫描串口，检测插入/移除事件。支持按VID/描述/端口名/制造商/驱动类型查询 */
class SerialDetector : public QObject {
    Q_OBJECT
public:
    explicit SerialDetector(QObject *parent = nullptr);
    ~SerialDetector() override;

    void startMonitoring(int intervalMs = 1000); ///< 启动热插拔监控(默认1000ms)
    void stopMonitoring();                        ///< 停止热插拔监控
    QList<SerialPortInfo> availablePorts() const; ///< 获取所有已知可用端口
    QStringList portNames() const;                ///< 获取所有已知端口名称
    bool isMonitoring() const;                    ///< 查询是否正在监控

    QList<SerialPortInfo> findByVendorId(quint16 vid) const;            ///< 按VID查找端口
    QList<SerialPortInfo> findByDescription(const QString &keyword) const; ///< 按描述关键词查找
    SerialPortInfo findByPortName(const QString &name) const;           ///< 按端口名精确查找
    QList<SerialPortInfo> findByManufacturer(const QString &keyword) const; ///< 按制造商查找
    QList<SerialPortInfo> findByDriverType(const QString &driverType) const; ///< 按驱动类型查找

    static UsbVendorEntry lookupVendor(quint16 vid);       ///< 按VID查找芯片厂商信息
    static QString identifyChip(quint16 vid, quint16 pid); ///< 按VID/PID识别芯片型号

    // ---- 统计计数器 ----
    quint64 totalScans() const { return m_totalScans; }
    quint64 totalInsertions() const { return m_totalInsertions; }
    quint64 totalRemovals() const { return m_totalRemovals; }
    quint64 totalVidLookups() const { return m_totalVidLookups; }
    static quint64 totalChipIdentifications() { return s_totalChipIdentifications; }
    quint64 totalPortChangesDetected() const { return m_totalPortChangesDetected; }
    quint64 totalFilterQueries() const { return m_totalFilterQueries; }
    int uniqueVidCount() const;
    static int knownVendorCount();
    void resetStatistics();

signals:
    void portInserted(const SerialPortInfo &info);  ///< 端口插入信号
    void portRemoved(const SerialPortInfo &info);   ///< 端口移除信号
    void portsChanged(const QList<SerialPortInfo> &current); ///< 端口列表变更信号
    void monitoringChanged(bool active);            ///< 监控状态变更信号

private:
    void refreshPorts();
    SerialPortInfo fromQtInfo(const QSerialPortInfo &info) const;
    static QString generateFriendlyName(const SerialPortInfo &info);
    static QString identifyDriverType(const QString &description, const QString &manufacturer);

    QTimer m_timer;                          ///< 轮询定时器
    QMap<QString, SerialPortInfo> m_knownPorts; ///< 已知端口映射(端口名→信息)
    bool m_monitoring = false;              ///< 监控状态标志

    quint64 m_totalScans = 0;       ///< 累计端口扫描次数
    quint64 m_totalInsertions = 0;  ///< 累计端口插入事件次数
    quint64 m_totalRemovals = 0;    ///< 累计端口移除事件次数
    mutable quint64 m_totalVidLookups = 0; ///< 累计VID查询次数
    static inline quint64 s_totalChipIdentifications = 0; ///< 累计芯片识别次数
    quint64 m_totalPortChangesDetected = 0; ///< 累计端口变化检测次数
    mutable quint64 m_totalFilterQueries = 0; ///< 累计过滤查询次数
    static const QVector<UsbVendorEntry> kKnownVendors; ///< 已知USB芯片厂商数据库
};

#endif // SERIALDETECTOR_H
