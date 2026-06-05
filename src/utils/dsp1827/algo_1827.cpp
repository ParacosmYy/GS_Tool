/**
 * @file algo_1827.cpp
 * @brief Algorithm module 1827
 */
#include "dsp1827/algo_1827.h"
QVector<double> algo_1827::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
