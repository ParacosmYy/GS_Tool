/**
 * @file algo_1427.cpp
 * @brief Algorithm module 1427
 */
#include "dsp1427/algo_1427.h"
QVector<double> algo_1427::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
