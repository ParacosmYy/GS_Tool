/**
 * @file algo_1247.cpp
 * @brief Algorithm module 1247
 */
#include "dsp1247/algo_1247.h"
QVector<double> algo_1247::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
