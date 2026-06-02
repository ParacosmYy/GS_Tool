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
#include <QVariantMap>
#include <QString>

/**
 * @class RecordingFileFormat
 * @brief 录制文件格式读写类
 *
 * 管理录制文件的读写操作，包括元数据存取、
 * 数据帧的序列化/反序列化和加载进度上报。
 */
class RecordingFileFormat : public QObject
{
    Q_OBJECT

public:
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
};

#endif // RECORDING_FILE_FORMAT_H
