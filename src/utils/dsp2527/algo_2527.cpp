/**
 * @file algo_2527.cpp
 * @brief Algorithm module 2527
 */
#include "dsp2527/algo_2527.h"
QVector<double> algo_2527::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
