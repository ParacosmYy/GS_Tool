/**
 * @file algo_1707.cpp
 * @brief Algorithm module 1707
 */
#include "dsp1707/algo_1707.h"
QVector<double> algo_1707::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
