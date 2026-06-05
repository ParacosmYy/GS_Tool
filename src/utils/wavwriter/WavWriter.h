/**
 * @file WavWriter.h
 * @brief WAV文件写入器 — 音频数据导出
 *
 * 功能: 写入PCM WAV文件，支持8/16/24/32位整数和32位浮点，
 *       统计写入文件数/采样数/耗时，文件写入完成信号。
 */
#ifndef WAVWRITER_H
#define WAVWRITER_H

#include <QObject>
#include <QFile>
#include <QVector>

class WavWriter : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalFilesWritten = 0;
        quint64 totalSamplesWritten = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit WavWriter(QObject* parent = nullptr);

    /** @brief 打开WAV文件 @param filename 文件路径 @param sampleRate 采样率 @param channels 声道数 @param bitsPerSample 位深度 */
    bool open(const QString& filename, int sampleRate, int channels,
              int bitsPerSample);

    /** @brief 写入采样数据(浮点) @param samples 采样数据 */
    bool write(const QVector<double>& samples);

    /** @brief 写入采样数据(整数) @param samples 采样数据 */
    bool writeInt(const QVector<int>& samples);

    /** @brief 关闭文件 */
    void close();

    /** @brief 是否已打开 */
    bool isOpen() const { return m_file.isOpen(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 文件写入完成 @param filename 文件名 @param samples 采样数 */
    void fileWritten(QString filename, int samples);

private:
    void writeHeader();
    void updateHeader();

    QFile m_file;
    int m_sampleRate;
    int m_channels;
    int m_bitsPerSample;
    quint32 m_dataSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // WAVWRITER_H
