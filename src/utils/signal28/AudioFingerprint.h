/**
 * @file AudioFingerprint.h
 * @brief 音频指纹 — 频谱图/星座图/哈希匹配
 *
 * 功能: 基于频谱图+星座图的音频指纹识别，支持指纹生成、
 *       哈希数据库构建与查询、鲁棒匹配(抗噪/抗时移)，
 *       统计指纹生成数/查询次数/匹配命中率/平均处理耗时。
 */
#ifndef AUDIOFINGERPRINT_H
#define AUDIOFINGERPRINT_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QHash>

/**
 * @class AudioFingerprint
 * @brief 音频指纹识别器，基于频谱星座图哈希
 */
class AudioFingerprint : public QObject {
    Q_OBJECT
public:
    /** 频谱峰值点 */
    struct PeakPoint {
        int timeFrame;      ///< 时间帧索引
        int freqBin;        ///< 频率bin索引
        double magnitude;   ///< 幅度
    };

    /** 指纹哈希 */
    struct FingerprintHash {
        quint64 hash;           ///< 组合哈希值
        int timeOffset;         ///< 时间偏移(帧)
        int sourceId;           ///< 音频源标识
    };

    /** 匹配结果 */
    struct MatchResult {
        bool found;                 ///< 是否匹配成功
        int sourceId;               ///< 匹配音频源标识
        double confidence;          ///< 置信度(0~1)
        int matchedPairs;           ///< 匹配哈希对数
        double timeOffsetSec;       ///< 时间偏移(秒)
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalFingerprintsGenerated = 0; ///< 总生成指纹数
        quint64 totalQueries = 0;               ///< 总查询次数
        quint64 totalHits = 0;                  ///< 总命中次数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
    };

    explicit AudioFingerprint(QObject* parent = nullptr);

    /** 设置参数 */
    void setSampleRate(double rate);
    void setFftSize(int size);
    void setPeakThreshold(double threshold);
    void setFanOut(int fanOut);
    void setTargetZoneWidth(int frames);

    /** 从音频数据生成频谱图 */
    QVector<QVector<double>> computeSpectrogram(const QVector<double>& audio);

    /** 从频谱图提取星座图峰值点 */
    QList<PeakPoint> extractPeaks(const QVector<QVector<double>>& spectrogram);

    /** 从峰值点生成指纹哈希 */
    QList<FingerprintHash> generateFingerprint(const QList<PeakPoint>& peaks,
                                               int sourceId);

    /** 将指纹注册到数据库 */
    void registerFingerprint(const QList<FingerprintHash>& fingerprints);

    /** 查询匹配 */
    MatchResult query(const QList<FingerprintHash>& fingerprints);

    /** 便捷: 完整的识别流程 */
    MatchResult identify(const QVector<double>& audio);

    /** 清空指纹数据库 */
    void clearDatabase();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 指纹生成完成信号 */
    void fingerprintGenerated(int hashCount, int sourceId);
    /** 匹配结果信号 */
    void matchResult(bool found, double confidence);

private:
    /** STFT单帧 */
    void fftFrame(const QVector<double>& frame,
                  QVector<double>& magnitude) const;
    /** 局部峰值检测 */
    QList<PeakPoint> findLocalPeaks(const QVector<double>& magnitude,
                                    int timeFrame) const;
    /** 组合哈希(锚点+目标点) */
    quint64 combinatorialHash(const PeakPoint& anchor,
                              const PeakPoint& target) const;

    double m_sampleRate;
    int m_fftSize;
    double m_peakThreshold;
    int m_fanOut;
    int m_targetZoneWidth;
    QHash<quint64, QList<FingerprintHash>> m_database;
    Stats m_stats;
    double m_timeSum;
};

#endif // AUDIOFINGERPRINT_H
