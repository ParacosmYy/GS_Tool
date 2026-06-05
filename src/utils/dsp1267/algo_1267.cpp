/**
 * @file algo_1267.cpp
 * @brief Algorithm module 1267
 */
#include "dsp1267/algo_1267.h"
QVector<double> algo_1267::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
