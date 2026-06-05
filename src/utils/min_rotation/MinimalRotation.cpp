/**
 * @file MinimalRotation.cpp
 * @brief 字典序最小旋转实现(Booth算法)
 */

#include <QElapsedTimer>

#include "utils/min_rotation/MinimalRotation.h"

MinimalRotation::MinimalRotation(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

int MinimalRotation::find(const QString& s)
{
    QElapsedTimer timer;
    timer.start();

    int n = s.length();
    int result = 0;

    if (n <= 1) {
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(result);
        return result;
    }

    /* Booth算法: 在s+s上双指针扫描 */
    int i = 0, j = 1, k = 0;
    QString ss = s + s;

    while (i < n && j < n && k < n) {
        QChar a = ss[i + k];
        QChar b = ss[j + k];
        if (a == b) {
            ++k;
        } else {
            if (a > b) {
                /* s[i..i+k] > s[j..j+k], 跳过i开头的所有旋转 */
                i = i + k + 1;
                if (i <= j)
                    i = j + 1;
            } else {
                /* s[i..i+k] < s[j..j+k], 跳过j开头的所有旋转 */
                j = j + k + 1;
                if (j <= i)
                    j = i + 1;
            }
            k = 0;
        }
    }

    result = qMin(i, j);

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalComputations, 1ULL);

    emit computationCompleted(result);
    return result;
}

QString MinimalRotation::rotate(const QString& s)
{
    QElapsedTimer timer;
    timer.start();

    QString result;

    if (s.isEmpty()) {
        m_stats.totalComputations++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalComputations, 1ULL);
        emit computationCompleted(0);
        return result;
    }

    int idx = 0;
    int n = s.length();

    if (n > 1) {
        /* 内联Booth算法避免重复信号 */
        int i = 0, j = 1, k = 0;
        QString ss = s + s;

        while (i < n && j < n && k < n) {
            QChar a = ss[i + k];
            QChar b = ss[j + k];
            if (a == b) {
                ++k;
            } else {
                if (a > b) {
                    i = i + k + 1;
                    if (i <= j)
                        i = j + 1;
                } else {
                    j = j + k + 1;
                    if (j <= i)
                        j = i + 1;
                }
                k = 0;
            }
        }
        idx = qMin(i, j);
    }

    result = s.mid(idx) + s.left(idx);

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalComputations, 1ULL);

    emit computationCompleted(idx);
    return result;
}

void MinimalRotation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
