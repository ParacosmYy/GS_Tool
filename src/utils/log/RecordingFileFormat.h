/**
 * @file RecordingFileFormat.h
 * @brief 录制文件格式读写器，负责录制数据的序列化和反序列化
 *
 * 提供 F1 数据录制回放子系统的持久化支持，
 * 将录制数据保存到文件或从文件加载，支持元数据管理。
 */

#ifndef RECORDING_FILE_FORMAT_H
#define RECORDING_FILE_FORMAT_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QString>

/**
 * @class RecordingFileFormat
 * @brief 录制文件格式读写类
 *
 * 管理录制文件的读写操作，包括元数据存取、
 * 数据帧的序列化/反序列化和加载进度上报。
 * 支持详细的错误信息追踪和文件完整性校验。
 */
class RecordingFileFormat : public QObject
{
    Q_OBJECT

public:
    /** @brief 文件魔数标识: "EDB" = EmbedDebug Binary */
    static constexpr const char* kMagic = "EDB";
    /** @brief 当前格式版本号 */
    static constexpr quint8 kVersion = 1;
    /** @brief 文件头固定长度: magic(3) + version(1) + metaSize(4) + reserved(4) */
    static constexpr int kHeaderSize = 12;
    /** @brief EOF标记: "EDBE" (4字节) */
    static constexpr const char* kEofMarker = "EDBE";
    /** @brief EOF标记长度 */
    static constexpr int kEofMarkerSize = 4;

    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit RecordingFileFormat(QObject* parent = nullptr);

    /**
     * @brief 将录制数据保存到文件
     * @param filePath 目标文件路径
     * @return true 保存成功
     */
    bool saveToFile(const QString& filePath);

    /**
     * @brief 从文件加载录制数据
     * @param filePath 源文件路径
     * @return true 加载成功
     */
    bool loadFromFile(const QString& filePath);

    /**
     * @brief 获取文件元数据
     * @return 元数据键值对
     */
    QVariantMap metadata() const;

    /**
     * @brief 设置待保存的原始录制数据和元数据
     * @param rawData  原始录制字节流 (EDL记录)
     * @param meta     附加元数据 (通道名/协议/采样率等)
     * @return true 数据非空且设置成功
     */
    bool setData(const QByteArray& rawData, const QVariantMap& meta);

    /**
     * @brief 获取已加载的原始录制数据
     * @return 原始录制字节流
     */
    QByteArray rawData() const;

    /**
     * @brief 检查是否已加载有效数据
     * @return true 已加载非空数据
     */
    bool hasData() const;

    /**
     * @brief 获取最近一次操作的错误描述
     * @return 错误信息字符串，无错误时返回空串
     */
    QString lastError() const;

signals:
    /**
     * @brief 加载进度更新信号
     * @param progress 进度值 0.0 ~ 1.0
     */
    void loadProgress(qreal progress);

    /**
     * @brief 加载完成信号
     * @param metadata 加载后的文件元数据
     */
    void loadCompleted(const QVariantMap& metadata);

private:
    QVariantMap m_metadata;  ///< 文件元数据
    QString     m_filePath;  ///< 当前关联的文件路径
    QByteArray  m_rawData;   ///< 原始录制数据 (EDL记录格式)
    QString     m_lastError; ///< 最近一次操作的错误描述

    // ---- 统计计数器 ----
    quint64 m_totalSaves = 0;       ///< 累计保存次数
    quint64 m_totalLoads = 0;       ///< 累计加载次数
    quint64 m_totalErrors = 0;      ///< 累计错误次数

    // ---- 扩展统计计数器 ----
    quint64 m_totalBytesWritten = 0;       ///< 累计写入字节总数(不含头部和EOF)
    quint64 m_totalBytesRead = 0;          ///< 累计读取字节总数(不含头部和EOF)
    quint64 m_serializationErrors = 0;     ///< 累计序列化(保存)错误次数
    quint64 m_deserializationErrors = 0;   ///< 累计反序列化(加载)错误次数
    quint64 m_lookupsPerformed = 0;        ///< 累计索引查找次数(元数据查询)
    quint64 m_cacheHits = 0;              ///< 累计缓存命中次数(重复加载相同文件)
public:
    /** @brief 获取累计保存次数 @return 保存次数 */
    quint64 totalSaves() const { return m_totalSaves; }
    /** @brief 获取累计加载次数 @return 加载次数 */
    quint64 totalLoads() const { return m_totalLoads; }
    /** @brief 获取累计错误次数 @return 错误次数 */
    quint64 totalErrors() const { return m_totalErrors; }

    /** @brief 获取累计写入字节总数 @return 字节数 */
    quint64 totalBytesWritten() const { return m_totalBytesWritten; }
    /** @brief 获取累计读取字节总数 @return 字节数 */
    quint64 totalBytesRead() const { return m_totalBytesRead; }
    /** @brief 获取累计序列化(保存)错误次数 @return 错误次数 */
    quint64 serializationErrors() const { return m_serializationErrors; }
    /** @brief 获取累计反序列化(加载)错误次数 @return 错误次数 */
    quint64 deserializationErrors() const { return m_deserializationErrors; }
    /** @brief 获取累计索引查找次数(元数据查询) @return 查找次数 */
    quint64 lookupsPerformed() const { return m_lookupsPerformed; }
    /** @brief 获取累计缓存命中次数(重复加载相同文件) @return 命中次数 */
    quint64 cacheHits() const { return m_cacheHits; }

    /** @brief 重置文件格式统计计数器(包含所有统计) */
    void resetFileFormatStatistics();
};

#endif // RECORDING_FILE_FORMAT_H
