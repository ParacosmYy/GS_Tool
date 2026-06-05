/**
 * @file algo_1307.cpp
 * @brief Algorithm module 1307
 */
#include "dsp1307/algo_1307.h"
QVector<double> algo_1307::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
