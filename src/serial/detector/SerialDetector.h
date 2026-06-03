/**
 * @file SerialDetector.h
 * @brief 串口检测器 - 增强版设备检测，支持USB VID/PID详细信息、友好名称和驱动识别
 *
 * 职责:
 *   1. 定时轮询检测串口设备的插入/移除事件
 *   2. 提供丰富的设备信息(VID/PID/友好名称/驱动类型/芯片型号)
 *   3. 按VID、描述、端口名、制造商等多种维度查询
 *   4. 记录设备变更统计(扫描次数/插入次数/移除次数/VID匹配次数)
 *
 * 协作关系:
 *   - PortWatcher: 低层热插拔检测，可组合使用
 *   - SerialConfigPanel: 通过信号更新端口列表
 *   - MainWindow: 监听设备变更事件
 *
 * 所属层级: 数据层（检测系统硬件状态，不涉及UI）
 */

#ifndef SERIALDETECTOR_H
#define SERIALDETECTOR_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QHash>
#include <QSerialPortInfo>

/**
 * @brief 已知USB转串口芯片厂商定义
 *
 * 通过VID匹配常见芯片厂商，用于友好名称显示。
 */
struct UsbVendorEntry {
    quint16 vid;              ///< USB厂商ID
    QString vendorName;       ///< 厂商名称(如"Winchiphead")
    QString commonChip;       ///< 常见芯片型号(如"CH340")
};

/**
 * @brief 串口端口信息结构 - 封装系统串口设备的关键属性
 *
 * 在QSerialPortInfo基础上扩展了友好名称、驱动类型和芯片识别信息。
 */
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

    /**
     * @brief 生成设备详情摘要文本(用于tooltip/日志)
     * @return 多行设备详情字符串
     */
    QString toSummary() const;
};

/**
 * @brief 串口检测器 - 增强版设备检测
 *
 * 定时扫描系统可用串口，检测插入/移除事件并发射信号通知。
 * 支持按VID、描述、端口名称、制造商、驱动类型等维度查询。
 * 内置已知USB转串口芯片数据库，自动识别芯片型号。
 *
 * 使用方式:
 * @code
 *   auto* detector = new SerialDetector(this);
 *   connect(detector, &SerialDetector::portInserted,
 *           this, &MyClass::onPortInserted);
 *   detector->startMonitoring(1000);
 * @endcode
 */
class SerialDetector : public QObject {
    Q_OBJECT
public:
    /** @brief 构造串口检测器 @param parent 父对象 */
    explicit SerialDetector(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~SerialDetector() override;

    // ---- 监控控制 ----

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

    // ---- 查询接口 ----

    /** @brief 按USB厂商ID(VID)查找端口 @param vid USB厂商ID @return 匹配的SerialPortInfo列表 */
    QList<SerialPortInfo> findByVendorId(quint16 vid) const;
    /** @brief 按设备描述关键词查找端口(大小写不敏感) @param keyword 搜索关键词 @return 匹配的SerialPortInfo列表 */
    QList<SerialPortInfo> findByDescription(const QString &keyword) const;
    /** @brief 按端口名称精确查找端口信息 @param name 端口名称(如"COM3") @return 匹配的SerialPortInfo，未找到返回空对象 */
    SerialPortInfo findByPortName(const QString &name) const;
    /** @brief 按制造商关键词查找端口(大小写不敏感) @param keyword 制造商关键词 @return 匹配的SerialPortInfo列表 */
    QList<SerialPortInfo> findByManufacturer(const QString &keyword) const;
    /** @brief 按驱动类型查找端口(精确匹配driverType字段) @param driverType 驱动类型 @return 匹配的SerialPortInfo列表 */
    QList<SerialPortInfo> findByDriverType(const QString &driverType) const;

    // ---- 芯片识别 ----

    /**
     * @brief 根据VID查找已知芯片厂商信息
     * @param vid USB厂商ID
     * @return 厂商信息，未匹配返回空vendorName
     */
    static UsbVendorEntry lookupVendor(quint16 vid);

    /**
     * @brief 根据VID/PID组合识别芯片型号
     * @param vid USB厂商ID
     * @param pid USB产品ID
     * @return 芯片型号字符串，未匹配返回"Unknown"
     */
    static QString identifyChip(quint16 vid, quint16 pid);

    // ---- 统计计数器 ----

    /** @brief 获取累计端口扫描次数 @return 扫描总次数 */
    quint64 totalScans() const { return m_totalScans; }
    /** @brief 获取累计端口插入事件次数 @return 插入总次数 */
    quint64 totalInsertions() const { return m_totalInsertions; }
    /** @brief 获取累计端口移除事件次数 @return 移除总次数 */
    quint64 totalRemovals() const { return m_totalRemovals; }
    /** @brief 获取累计VID查询次数 @return VID查询总次数 */
    quint64 totalVidLookups() const { return m_totalVidLookups; }
    /** @brief 获取累计检测到的不同VID数量 @return 不同VID计数 */
    int uniqueVidCount() const;
    /** @brief 获取已知芯片厂商数据库条目数 @return 数据库大小 */
    static int knownVendorCount();
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 端口插入信号 @param info 新插入端口详细信息 */
    void portInserted(const SerialPortInfo &info);
    /** @brief 端口移除信号 @param info 被移除端口详细信息 */
    void portRemoved(const SerialPortInfo &info);
    /** @brief 端口列表变更信号 @param current 当前所有端口信息列表 */
    void portsChanged(const QList<SerialPortInfo> &current);
    /** @brief 监控状态变更信号 @param active true=已启动 false=已停止 */
    void monitoringChanged(bool active);

private:
    /** @brief 刷新端口列表，检测插入/移除事件并发射对应信号 */
    void refreshPorts();
    /** @brief 将QSerialPortInfo转换为项目内部SerialPortInfo结构(含增强信息) @param info Qt串口信息对象 @return 内部SerialPortInfo结构 */
    SerialPortInfo fromQtInfo(const QSerialPortInfo &info) const;
    /** @brief 自动生成友好名称(芯片型号+端口名) @param info 端口信息 @return 友好名称字符串 */
    static QString generateFriendlyName(const SerialPortInfo &info);
    /** @brief 识别驱动类型(CH340/CP2102等) @param description 设备描述 @param manufacturer 制造商 @return 驱动类型字符串 */
    static QString identifyDriverType(const QString &description, const QString &manufacturer);

    QTimer m_timer;                         ///< 轮询定时器
    QMap<QString, SerialPortInfo> m_knownPorts; ///< 已知端口映射(端口名→信息)
    bool m_monitoring = false;              ///< 监控状态标志

    // ---- 统计计数器 ----
    quint64 m_totalScans = 0;       ///< 累计端口扫描次数
    quint64 m_totalInsertions = 0;  ///< 累计端口插入事件次数
    quint64 m_totalRemovals = 0;    ///< 累计端口移除事件次数
    mutable quint64 m_totalVidLookups = 0; ///< 累计VID查询次数

    // ---- 已知USB转串口芯片数据库 ----
    static const QVector<UsbVendorEntry> kKnownVendors; ///< 已知USB芯片厂商数据库
};

#endif // SERIALDETECTOR_H
