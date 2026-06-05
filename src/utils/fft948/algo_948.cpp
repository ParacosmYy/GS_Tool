/**
 * @file algo_948.cpp
 * @brief Algorithm module 948
 */
#include "fft948/algo_948.h"
QVector<double> algo_948::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
