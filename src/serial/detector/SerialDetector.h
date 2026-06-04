/**
 * @file SerialDetector.h
 * @brief 串口检测器 - 增强版设备检测。支持USB VID/PID详细信息、友好名称和驱动识别。定时轮询检测插入/移除事件
 */

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
    quint16 vid;            ///< USB厂商ID
    QString vendorName;     ///< 厂商名称(如"WCH"/"Silicon Labs")
    QString commonChip;     ///< 常见芯片型号(如"CH340"/"CP2102")
};

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

    /** @brief 生成设备详情摘要文本(tooltip/日志) @return 多行摘要字符串 */
    QString toSummary() const;
};

/** @brief 串口检测器 - 增强版设备检测。定时扫描串口，检测插入/移除事件。支持按VID/描述/端口名/制造商/驱动类型查询 */
class SerialDetector : public QObject {
    Q_OBJECT
public:
    /** @brief 构造串口检测器 @param parent 父对象 */
    explicit SerialDetector(QObject *parent = nullptr);

    /** @brief 析构函数，停止监控并释放资源 */
    ~SerialDetector() override;

    // ---- 监控控制 ----

    /** @brief 启动热插拔监控 @param intervalMs 轮询间隔(毫秒，最小100ms，默认1000ms) */
    void startMonitoring(int intervalMs = 1000);

    /** @brief 停止热插拔监控 */
    void stopMonitoring();

    /** @brief 获取所有已知可用端口 @return 端口信息列表 */
    QList<SerialPortInfo> availablePorts() const;

    /** @brief 获取所有已知端口名称 @return 端口名称字符串列表 */
    QStringList portNames() const;

    /** @brief 查询是否正在监控 @return true=正在监控 */
    bool isMonitoring() const;

    // ---- 查询接口 ----

    /** @brief 按USB VID查找端口 @param vid 厂商ID @return 匹配的端口信息列表 */
    QList<SerialPortInfo> findByVendorId(quint16 vid) const;

    /** @brief 按设备描述关键词查找(大小写不敏感) @param keyword 搜索关键词 @return 匹配的端口信息列表 */
    QList<SerialPortInfo> findByDescription(const QString &keyword) const;

    /** @brief 按端口名称精确查找 @param name 端口名称(如"COM3") @return 匹配的端口信息，未找到返回默认值 */
    SerialPortInfo findByPortName(const QString &name) const;

    /** @brief 按制造商关键词查找(大小写不敏感) @param keyword 搜索关键词 @return 匹配的端口信息列表 */
    QList<SerialPortInfo> findByManufacturer(const QString &keyword) const;

    /** @brief 按驱动类型查找 @param driverType 驱动类型(如"CH340") @return 匹配的端口信息列表 */
    QList<SerialPortInfo> findByDriverType(const QString &driverType) const;

    // ---- 芯片识别 ----

    /** @brief 按VID查找芯片厂商信息 @param vid USB厂商ID @return 厂商条目，未匹配返回空条目 */
    static UsbVendorEntry lookupVendor(quint16 vid);

    /** @brief 按VID和PID识别芯片型号 @param vid USB厂商ID @param pid USB产品ID @return 芯片型号字符串，未识别返回"Unknown" */
    static QString identifyChip(quint16 vid, quint16 pid);

    // ---- 统计计数器 ----

    /** @brief 获取累计端口扫描次数 @return 扫描次数 */
    quint64 totalScans() const { return m_totalScans; }

    /** @brief 获取累计端口插入次数 @return 插入次数 */
    quint64 totalInsertions() const { return m_totalInsertions; }

    /** @brief 获取累计端口移除次数 @return 移除次数 */
    quint64 totalRemovals() const { return m_totalRemovals; }

    /** @brief 获取累计VID查询次数 @return 查询次数 */
    quint64 totalVidLookups() const { return m_totalVidLookups; }

    /** @brief 获取累计芯片识别次数 @return 识别次数 */
    static quint64 totalChipIdentifications() { return s_totalChipIdentifications; }

    /** @brief 获取累计端口变化检测次数(插入+移除事件合计) @return 变化次数 */
    quint64 totalPortChangesDetected() const { return m_totalPortChangesDetected; }

    /** @brief 获取累计过滤查询次数(描述/制造商/端口名/驱动类型查找合计) @return 查询次数 */
    quint64 totalFilterQueries() const { return m_totalFilterQueries; }

    /** @brief 获取累计检测到的不同VID数量 @return 不同VID的数量 */
    int uniqueVidCount() const;

    /** @brief 获取已知芯片厂商数据库条目数 @return 厂商条目数 */
    static int knownVendorCount();

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 端口插入信号 @param info 插入的端口信息 */
    void portInserted(const SerialPortInfo &info);

    /** @brief 端口移除信号 @param info 移除的端口信息 */
    void portRemoved(const SerialPortInfo &info);

    /** @brief 端口列表变更信号 @param current 当前所有端口信息列表 */
    void portsChanged(const QList<SerialPortInfo> &current);

    /** @brief 监控状态变更信号 @param active true=开始监控, false=停止监控 */
    void monitoringChanged(bool active);

private:
    /** @brief 刷新端口列表(检测插入/移除事件) */
    void refreshPorts();

    /** @brief 将QSerialPortInfo转换为内部SerialPortInfo @param info Qt端口信息 @return 内部端口信息 */
    SerialPortInfo fromQtInfo(const QSerialPortInfo &info) const;

    /** @brief 自动生成友好名称(如"CH340 (COM3)") @param info 端口信息 @return 友好名称字符串 */
    static QString generateFriendlyName(const SerialPortInfo &info);

    /** @brief 根据设备描述和制造商识别驱动类型 @param description 设备描述 @param manufacturer 制造商名称 @return 驱动类型字符串 */
    static QString identifyDriverType(const QString &description, const QString &manufacturer);

    QTimer m_timer;                         ///< 轮询定时器
    QMap<QString, SerialPortInfo> m_knownPorts; ///< 已知端口映射(端口名→信息)
    bool m_monitoring = false;              ///< 监控状态标志

    // ---- 统计计数器 ----
    quint64 m_totalScans = 0;       ///< 累计端口扫描次数
    quint64 m_totalInsertions = 0;  ///< 累计端口插入事件次数
    quint64 m_totalRemovals = 0;    ///< 累计端口移除事件次数
    mutable quint64 m_totalVidLookups = 0; ///< 累计VID查询次数
    static inline quint64 s_totalChipIdentifications = 0; ///< 累计芯片识别次数(identifyChip调用)
    quint64 m_totalPortChangesDetected = 0; ///< 累计端口变化检测次数(插入+移除事件合计)
    mutable quint64 m_totalFilterQueries = 0; ///< 累计过滤查询次数(描述/制造商/端口名/驱动类型查找)

    // ---- 已知USB转串口芯片数据库 ----
    static const QVector<UsbVendorEntry> kKnownVendors; ///< 已知USB芯片厂商数据库
};

#endif // SERIALDETECTOR_H
