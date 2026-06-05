/**
 * @file algo_2707.cpp
 * @brief Algorithm module 2707
 */
#include "dsp2707/algo_2707.h"
QVector<double> algo_2707::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
