/**
 * @file WavReader.h
 * @brief WAV文件读取器 — 音频数据导入
 *
 * 功能: 读取PCM WAV文件，解析格式头，支持多种位深度，
 *       统计读取文件数/采样数/耗时，文件读取完成信号。
 */
#ifndef WAVREADER_H
#define WAVREADER_H

#include <QObject>
#include <QFile>
#include <QVector>

class WavReader : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalFilesRead = 0;
        quint64 totalSamplesRead = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit WavReader(QObject* parent = nullptr);

    /** @brief 打开WAV文件 @param filename 文件路径 @return 是否成功 */
    bool open(const QString& filename);

    /** @brief 读取全部采样 @return 浮点采样数据 */
    QVector<double> readAll();

    /** @brief 读取指定数量采样 @param count 采样数 @return 浮点采样数据 */
    QVector<double> read(int count);

    /** @brief 关闭文件 */
    void close();

    int sampleRate() const { return m_sampleRate; }
    int channels() const { return m_channels; }
    int bitsPerSample() const { return m_bitsPerSample; }
    quint64 sampleCount() const { return m_sampleCount; }
    bool isOpen() const { return m_file.isOpen(); }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 文件读取完成 @param filename 文件名 @param samples 采样数 */
    void fileRead(QString filename, int samples);

private:
    double readSample();

    QFile m_file;
    int m_sampleRate;
    int m_channels;
    int m_bitsPerSample;
    quint64 m_dataOffset;
    quint64 m_dataSize;
    quint64 m_sampleCount;
    quint64 m_samplesRead;
    Stats m_stats;
    double m_timeSum;
};

#endif // WAVREADER_H
