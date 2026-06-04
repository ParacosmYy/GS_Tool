/**
 * @file ConfigProfileManager.h
 * @brief 设备配置档案管理器 -- 多设备配置切换/导入导出
 *
 * 管理目标设备的配置档案(串口设置+协议配置+自定义字段)，
 * 支持JSON格式导入导出，持久化到AppData目录。单例模式。
 *
 * 协作: SettingsController / ConnectionController / SerialConfigPanel
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

class QJsonObject;

/** @brief 设备配置档案管理器 — 单例，管理多设备配置切换 */
class ConfigProfileManager : public QObject {
    Q_OBJECT

public:
    /** @brief 串口配置参数 */
    struct SerialConfig {
        QString portName;           ///< 串口名称 (如 "COM3")
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
        int frameLength = 0;                        ///< 帧长度 (0=变长)
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

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProfilesCreated = 0;   ///< 总创建数
        quint64 totalProfilesLoaded = 0;    ///< 总加载数
        quint64 totalProfilesDeleted = 0;   ///< 总删除数
        quint64 totalImports = 0;           ///< 总导入次数
        quint64 totalExports = 0;           ///< 总导出次数
        quint64 totalSwitches = 0;          ///< 总切换次数
        int peakProfiles = 0;               ///< 峰值档案数
        int activeProfileIndex = -1;        ///< 当前活跃索引
    };

    /** @brief 获取全局唯一实例 @return 档案管理器引用 */
    static ConfigProfileManager& instance();

    // ── 档案CRUD ──
    /** @brief 创建新档案 @return 新索引，-1=失败(名称重复) */
    int createProfile(const DeviceProfile& profile);
    /** @brief 更新已有档案 @param index 档案索引 @return 是否成功 */
    bool updateProfile(int index, const DeviceProfile& profile);
    /** @brief 删除档案 @return 是否成功 */
    bool removeProfile(int index);
    /** @brief 获取指定索引档案 */
    DeviceProfile profile(int index) const;
    /** @brief 获取所有档案 */
    QList<DeviceProfile> allProfiles() const;

    // ── 切换与查询 ──
    /** @brief 当前活跃档案索引 (-1=无) */
    int currentIndex() const;
    /** @brief 切换到指定档案 @return 是否成功 */
    bool switchToProfile(int index);
    /** @brief 获取当前活跃档案 */
    DeviceProfile currentProfile() const;
    /** @brief 按名称查找 @return 索引，-1=未找到 */
    int findByName(const QString& name) const;
    /** @brief 按设备类型查找 */
    QList<DeviceProfile> findByDeviceType(const QString& type) const;

    // ── 导入导出 ──
    /** @brief 导出单个档案到JSON */
    bool exportProfile(int index, const QString& filePath);
    /** @brief 从JSON导入档案 */
    bool importProfile(const QString& filePath);
    /** @brief 导出所有档案到指定目录 */
    bool exportAll(const QString& dirPath);

    // ── 持久化 ──
    bool save();    ///< 保存所有档案到磁盘
    bool load();    ///< 从磁盘加载所有档案

    // ── 统计 ──
    const Stats& stats() const; ///< 获取统计
    void resetStatistics();     ///< 重置统计 (在独立编译单元实现)

signals:
    void profileCreated(int index);     ///< 档案已创建
    void profileRemoved(int index);     ///< 档案已删除
    void profileSwitched(int index);    ///< 档案已切换
    void profileUpdated(int index);     ///< 档案已更新

private:
    friend struct ConfigProfileManagerHolder;
    explicit ConfigProfileManager(QObject* parent = nullptr); ///< 私有构造(单例)

    // 序列化辅助
    QJsonObject serialConfigToJson(const SerialConfig& config) const;
    SerialConfig jsonToSerialConfig(const QJsonObject& obj) const;
    QJsonObject protocolConfigToJson(const ProtocolConfig& config) const;
    ProtocolConfig jsonToProtocolConfig(const QJsonObject& obj) const;
    QJsonObject deviceProfileToJson(const DeviceProfile& prof) const;
    DeviceProfile jsonToDeviceProfile(const QJsonObject& obj) const;
    QString profileFilePath() const;

    QList<DeviceProfile> m_profiles;    ///< 档案列表
    int m_currentIndex = -1;            ///< 当前活跃索引
    QString m_storagePath;              ///< 存储目录
    Stats m_stats;                      ///< 运行统计
};

#endif // CONFIGPROFILEMANAGER_H
