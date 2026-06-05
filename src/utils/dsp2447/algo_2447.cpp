/**
 * @file algo_2447.cpp
 * @brief Algorithm module 2447
 */
#include "dsp2447/algo_2447.h"
QVector<double> algo_2447::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
