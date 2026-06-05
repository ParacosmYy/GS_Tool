/**
 * @file algo_1308.cpp
 * @brief Algorithm module 1308
 */
#include "fft1308/algo_1308.h"
QVector<double> algo_1308::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
