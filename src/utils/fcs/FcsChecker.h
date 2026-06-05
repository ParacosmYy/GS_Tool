/**
 * @file FcsChecker.h
 * @brief FCS帧校验序列 — HDLC/PPP FCS-16/FCS-32
 */
#ifndef FCSCHECKER_H
#define FCSCHECKER_H

#include <QObject>
#include <QByteArray>

class FcsChecker : public QObject {
    Q_OBJECT
public:
    enum class Type { FCS16, FCS32 };

    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalBytesProcessed = 0;
    };

    explicit FcsChecker(Type type = Type::FCS16, QObject* parent = nullptr);

    /** @brief 计算FCS @param data 数据 @return FCS值 */
    quint32 compute(const QByteArray& data);

    /** @brief 验证 @param data 含FCS的数据 @return 是否通过 */
    bool verify(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computed(quint32 fcsValue);

private:
    quint32 computeCRC16(const QByteArray& data);
    quint32 computeCRC32(const QByteArray& data);

    Type m_type;
    Stats m_stats;
};

#endif // FCSCHECKER_H
