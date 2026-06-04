/**
 * @file ConfigProfileManager.h
 * @brief 设备配置档案管理器 -- 多设备配置切换/导入导出
 *
 * 管理目标设备的配置档案。嵌入式开发者经常使用多个设备，
 * 每个设备需要不同的串口设置、波特率、协议配置。
 * 本组件管理命名档案，支持JSON格式导入导出。
 *
 * 职责:
 *   - 创建/更新/删除设备配置档案
 *   - 快速切换当前活跃档案
 *   - 按名称/设备类型搜索档案
 *   - JSON格式导入导出
 *   - 持久化到AppData目录
 *
 * 设计模式: 单例模式(Q_GLOBAL_STATIC)
 * 协作: SettingsController/ConnectionController/SerialConfigPanel
 */

#ifndef CONFIGPROFILEMANAGER_H
#define CONFIGPROFILEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QMap>
#include <QDateTime>
#include <QVariant>

/**
 * @brief 设备配置档案管理器
 *
 * 单例模式，全局唯一实例。管理多个设备配置档案，
 * 每个档案包含串口设置、协议配置和自定义字段。
 * 支持档案的创建、切换、导入导出和持久化存储。
 */
class ConfigProfileManager : public QObject {
    Q_OBJECT

public:
    // ── 数据结构定义 ──

    /** @brief 串口配置参数 */
    struct SerialConfig {
        QString portName;           ///< 串口名称 (如 "COM3", "/dev/ttyUSB0")
        qint32 baudRate = 115200;   ///< 波特率
        int dataBits = 8;           ///< 数据位 (5~8)
        int stopBits = 1;           ///< 停止位 (1 或 2)
        QString parity;             ///< 校验: "None"/"Even"/"Odd"
        QString flowControl;        ///< 流控: "None"/"Hardware"/"Software"
    };

    /** @brief 协议配置参数 */
    struct ProtocolConfig {
        QString protocolName;                       ///< 协议名称
        QByteArray headerPattern;                   ///< 帧头模式
        QByteArray footerPattern;                   ///< 帧尾模式
        int frameLength = 0;                        ///< 帧长度 (0表示变长)
        QMap<QString, QVariant> customSettings;     ///< 自定义协议设置
    };

    /** @brief 设备配置档案 */
    struct DeviceProfile {
        QString name;                               ///< 档案名称 (唯一标识)
        QString description;                        ///< 档案描述
        QString deviceType;                         ///< 设备类型: "MCU"/"FPGA"/"Modbus"/"Custom"
        SerialConfig serial;                        ///< 串口配置
        ProtocolConfig protocol;                    ///< 协议配置
        QMap<QString, QVariant> customFields;       ///< 自定义扩展字段
        QDateTime createdAt;                        ///< 创建时间
        QDateTime lastUsed;                         ///< 最后使用时间
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalProfilesCreated = 0;           ///< 总创建档案数
        quint64 totalProfilesLoaded = 0;            ///< 总加载档案数
        quint64 totalProfilesDeleted = 0;           ///< 总删除档案数
        quint64 totalImports = 0;                   ///< 总导入次数
        quint64 totalExports = 0;                   ///< 总导出次数
        quint64 totalSwitches = 0;                  ///< 总切换次数
        int peakProfiles = 0;                       ///< 峰值档案数量
        int activeProfileIndex = -1;                ///< 当前活跃档案索引
    };

    // ── 单例访问 ──

    /**
     * @brief 获取全局唯一实例
     * @return 档案管理器引用
     */
    static ConfigProfileManager& instance();

    // ── 档案CRUD ──

    /**
     * @brief 创建新档案
     * @param profile 档案数据 (createdAt/lastUsed会被自动设置)
     * @return 新档案索引，-1表示失败(名称重复)
     */
    int createProfile(const DeviceProfile& profile);

    /**
     * @brief 更新已有档案
     * @param index 档案索引
     * @param profile 新的档案数据
     * @return 是否成功
     */
    bool updateProfile(int index, const DeviceProfile& profile);

    /**
     * @brief 删除档案
     * @param index 档案索引
     * @return 是否成功
     */
    bool removeProfile(int index);

    /**
     * @brief 获取指定索引的档案
     * @param index 档案索引
     * @return 档案数据 (索引无效返回默认构造的DeviceProfile)
     */
    DeviceProfile profile(int index) const;

    /**
     * @brief 获取所有档案
     * @return 档案列表
     */
    QList<DeviceProfile> allProfiles() const;

    // ── 档案切换 ──

    /**
     * @brief 获取当前活跃档案索引
     * @return 索引，-1表示无活跃档案
     */
    int currentIndex() const;

    /**
     * @brief 切换到指定档案
     * @param index 目标档案索引
     * @return 是否成功
     */
    bool switchToProfile(int index);

    /**
     * @brief 获取当前活跃档案
     * @return 当前档案数据 (无活跃档案返回默认构造)
     */
    DeviceProfile currentProfile() const;

    // ── 搜索 ──

    /**
     * @brief 按名称查找档案
     * @param name 档案名称
     * @return 档案索引，-1表示未找到
     */
    int findByName(const QString& name) const;

    /**
     * @brief 按设备类型查找档案
     * @param type 设备类型
     * @return 匹配的档案列表
     */
    QList<DeviceProfile> findByDeviceType(const QString& type) const;

    // ── 导入导出 ──

    /**
     * @brief 导出单个档案到JSON文件
     * @param index 档案索引
     * @param filePath 目标文件路径
     * @return 是否成功
     */
    bool exportProfile(int index, const QString& filePath);

    /**
     * @brief 从JSON文件导入档案
     * @param filePath 源文件路径
     * @return 是否成功
     */
    bool importProfile(const QString& filePath);

    /**
     * @brief 导出所有档案到指定目录
     * @param dirPath 目标目录路径
     * @return 是否成功
     */
    bool exportAll(const QString& dirPath);

    // ── 持久化 ──

    /**
     * @brief 保存所有档案到磁盘
     * @return 是否成功
     */
    bool save();

    /**
     * @brief 从磁盘加载所有档案
     * @return 是否成功
     */
    bool load();

    // ── 统计 ──

    /**
     * @brief 获取运行时统计信息
     * @return 统计数据常引用
     */
    const Stats& stats() const;

    /**
     * @brief 重置统计数据 (在独立编译单元中实现)
     */
    void resetStatistics();

signals:
    /**
     * @brief 档案已创建信号
     * @param index 新档案索引
     */
    void profileCreated(int index);

    /**
     * @brief 档案已删除信号
     * @param index 被删除档案的原索引
     */
    void profileRemoved(int index);

    /**
     * @brief 档案已切换信号
     * @param index 新活跃档案索引
     */
    void profileSwitched(int index);

    /**
     * @brief 档案已更新信号
     * @param index 被更新档案索引
     */
    void profileUpdated(int index);

private:
    friend struct ConfigProfileManagerHolder;
    /**
     * @brief 构造函数 (私有，单例模式)
     * @param parent 父对象
     */
    explicit ConfigProfileManager(QObject* parent = nullptr);

    // ── 序列化辅助 ──

    /**
     * @brief 将SerialConfig序列化为JSON对象
     * @param config 串口配置
     * @return JSON对象
     */
    QJsonObject serialConfigToJson(const SerialConfig& config) const;

    /**
     * @brief 从JSON对象反序列化SerialConfig
     * @param obj JSON对象
     * @return 串口配置
     */
    SerialConfig jsonToSerialConfig(const QJsonObject& obj) const;

    /**
     * @brief 将ProtocolConfig序列化为JSON对象
     * @param config 协议配置
     * @return JSON对象
     */
    QJsonObject protocolConfigToJson(const ProtocolConfig& config) const;

    /**
     * @brief 从JSON对象反序列化ProtocolConfig
     * @param obj JSON对象
     * @return 协议配置
     */
    ProtocolConfig jsonToProtocolConfig(const QJsonObject& obj) const;

    /**
     * @brief 将DeviceProfile序列化为JSON对象
     * @param prof 设备档案
     * @return JSON对象
     */
    QJsonObject deviceProfileToJson(const DeviceProfile& prof) const;

    /**
     * @brief 从JSON对象反序列化DeviceProfile
     * @param obj JSON对象
     * @return 设备档案
     */
    DeviceProfile jsonToDeviceProfile(const QJsonObject& obj) const;

    /**
     * @brief 获取持久化文件路径
     * @return AppData目录下的档案文件路径
     */
    QString profileFilePath() const;

    // ── 成员变量 ──

    QList<DeviceProfile> m_profiles;    ///< 档案列表
    int m_currentIndex = -1;            ///< 当前活跃档案索引 (-1表示无)
    QString m_storagePath;              ///< 存储目录路径
    Stats m_stats;                      ///< 运行时统计
};

#endif // CONFIGPROFILEMANAGER_H
