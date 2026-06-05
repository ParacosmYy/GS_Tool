/**
 * @file algo_1493.cpp
 * @brief Algorithm module 1493
 */
#include "crypto1493/algo_1493.h"
QVector<double> algo_1493::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
