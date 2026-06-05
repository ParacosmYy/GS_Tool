/**
 * @file algo_1148.cpp
 * @brief Algorithm module 1148
 */
#include "fft1148/algo_1148.h"
QVector<double> algo_1148::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
