/**
 * @file algo_1347.cpp
 * @brief Algorithm module 1347
 */
#include "dsp1347/algo_1347.h"
QVector<double> algo_1347::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
