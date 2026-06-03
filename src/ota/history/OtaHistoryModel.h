/**
 * @file OtaHistoryModel.h
 * @brief OTA历史记录表格模型 — 显示历次OTA传输的时间/文件/协议/大小/耗时/结果
 *
 * 使用QAbstractTableModel展示OTA升级历史，支持增删查、持久化到SettingsManager。
 * 最大保留200条记录，超出自动淘汰最旧记录。
 */
#ifndef OTAHISTORYMODEL_H
#define OTAHISTORYMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

/**
 * @brief OTA升级历史记录数据结构
 *
 * 记录每次OTA传输的完整信息，包括固件文件名、使用的协议、
 * 文件大小、开始时间、传输耗时和成功/失败状态。
 */
struct OtaRecord {
    QString fileName;       ///< 固件文件名
    QString protocol;       ///< 协议: xmodem-crc / ymodem / zmodem
    qint64 fileSize = 0;    ///< 文件大小(字节)
    QDateTime startTime;    ///< 开始时间
    qint64 durationMs = 0;  ///< 传输耗时(毫秒)
    bool success = false;   ///< 是否成功
    QString errorMessage;   ///< 失败原因(成功时为空)
};

/**
 * @brief OTA历史记录表格模型
 *
 * 提供增删查、持久化到SettingsManager。
 * 列定义: 时间、文件名、协议、大小、耗时、结果。
 * 结果列使用语义色板着色(成功=绿色, 失败=红色)。
 */
class OtaHistoryModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /** @brief 表格列枚举 */
    enum Column {
        ColTime = 0,     ///< 开始时间列
        ColFileName,     ///< 文件名列
        ColProtocol,     ///< 协议列
        ColSize,         ///< 文件大小列
        ColDuration,     ///< 耗时列
        ColResult,       ///< 结果列
        ColCount         ///< 列总数(用于迭代)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit OtaHistoryModel(QObject* parent = nullptr);

    /** @brief 返回行数 @param parent 父索引(无效索引返回总行数) */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 返回列数 @param parent 父索引(始终返回ColCount) */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    /** @brief 返回指定索引的显示数据 @param index 模型索引 @param role 显示角色 */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    /** @brief 返回表头数据 @param section 列号 @param orientation 方向 @param role 显示角色 */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /** @brief 添加一条OTA记录，超出上限自动淘汰最旧记录 @param record OTA记录 */
    void addRecord(const OtaRecord& record);
    /** @brief 清空所有历史记录 */
    void clearHistory();

    /** @brief 获取指定行的记录引用 @param row 行号 */
    const OtaRecord& record(int row) const;
    /** @brief 返回总记录数 */
    int count() const;

    /** @brief 将历史记录持久化到SettingsManager */
    void saveToSettings();
    /** @brief 从SettingsManager加载历史记录 */
    void loadFromSettings();

    /** @brief 获取成功传输次数 */
    int successCount() const;
    /** @brief 获取失败传输次数 */
    int failureCount() const;
    /** @brief 获取成功率（0.0~1.0） */
    double successRate() const;
    /** @brief 获取累计传输总字节数 */
    qint64 totalBytesTransferred() const;
    /** @brief 获取平均传输耗时（毫秒） */
    qint64 averageDurationMs() const;
    /** @brief 生成统计摘要文本 */
    QString statisticsSummary() const;

    // ── 统计计数器 Getter ──

    /** @brief 获取历史记录添加总次数 @return 累计添加次数 */
    quint64 totalEntriesAdded() const;

    /** @brief 获取历史记录移除总次数 @return 累计移除次数(含淘汰) */
    quint64 totalEntriesRemoved() const;

    /** @brief 重置历史记录统计计数器(不影响记录数据本身) */
    void resetHistoryStatistics();

private:
    QVector<OtaRecord> m_records;          ///< 历史记录列表
    static constexpr int kMaxRecords = 200; ///< 最大保留记录数

    // ── 统计计数器 ──
    quint64 m_totalEntriesAdded = 0;       ///< 历史记录添加总次数
    quint64 m_totalEntriesRemoved = 0;     ///< 历史记录移除总次数(含淘汰)
};

#endif // OTAHISTORYMODEL_H
