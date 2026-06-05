/**
 * @file algo_2047.cpp
 * @brief Algorithm module 2047
 */
#include "dsp2047/algo_2047.h"
QVector<double> algo_2047::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
