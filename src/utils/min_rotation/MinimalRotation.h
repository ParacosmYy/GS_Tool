/**
 * @file MinimalRotation.h
 * @brief 字典序最小旋转(Booth算法)
 *
 * 功能: 在字符串的所有旋转中找到字典序最小的旋转，
 *       统计计算次数/耗时。
 */
#ifndef MINIMALROTATION_H
#define MINIMALROTATION_H

#include <QObject>
#include <QString>

class MinimalRotation : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit MinimalRotation(QObject* parent = nullptr);

    /** @brief 找到字典序最小旋转的起始索引 @param s 输入字符串 @return 旋转起始索引 */
    int find(const QString& s);

    /** @brief 返回字典序最小旋转后的字符串 @param s 输入字符串 @return 旋转后的字符串 */
    QString rotate(const QString& s);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成 @param rotationIndex 最小旋转索引 */
    void computationCompleted(int rotationIndex);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // MINIMALROTATION_H
