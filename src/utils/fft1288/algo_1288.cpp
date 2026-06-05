/**
 * @file algo_1288.cpp
 * @brief Algorithm module 1288
 */
#include "fft1288/algo_1288.h"
QVector<double> algo_1288::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
