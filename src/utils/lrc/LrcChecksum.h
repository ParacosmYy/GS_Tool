/**
 * @file LrcChecksum.h
 * @brief LRC校验和 — 纵向冗余校验
 */
#ifndef LRCCHECKSUM_H
#define LRCCHECKSUM_H

#include <QObject>
#include <QByteArray>

class LrcChecksum : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalBytesProcessed = 0;
    };

    explicit LrcChecksum(QObject* parent = nullptr);

    /** @brief 计算LRC @param data 数据 @return LRC值 */
    quint8 compute(const QByteArray& data);

    /** @brief 验证(包含LRC的数据) @param data 含LRC的数据 @return 是否通过 */
    bool verify(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computed(quint8 lrcValue);

private:
    Stats m_stats;
};

#endif // LRCCHECKSUM_H
