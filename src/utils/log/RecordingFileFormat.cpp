/**
 * @file RecordingFileFormat.cpp
 * @brief 录制文件格式读写器实现
 */

#include "utils/log/RecordingFileFormat.h"

RecordingFileFormat::RecordingFileFormat(QObject* parent)
    : QObject(parent)
{
}

bool RecordingFileFormat::saveToFile(const QString& filePath)
{
    Q_UNUSED(filePath)
    // TODO: 将录制数据序列化并写入文件
    return false;
}

bool RecordingFileFormat::loadFromFile(const QString& filePath)
{
    Q_UNUSED(filePath)
    // TODO: 从文件反序列化录制数据，期间通过 loadProgress 上报进度
    return false;
}

QVariantMap RecordingFileFormat::metadata() const
{
    return m_metadata;
}
