/**
 * @file algo_1207.cpp
 * @brief Algorithm module 1207
 */
#include "dsp1207/algo_1207.h"
QVector<double> algo_1207::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
