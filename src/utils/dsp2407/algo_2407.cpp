/**
 * @file algo_2407.cpp
 * @brief Algorithm module 2407
 */
#include "dsp2407/algo_2407.h"
QVector<double> algo_2407::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
