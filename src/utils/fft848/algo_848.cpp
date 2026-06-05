/**
 * @file algo_848.cpp
 * @brief Algorithm module 848
 */
#include "fft848/algo_848.h"
QVector<double> algo_848::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
