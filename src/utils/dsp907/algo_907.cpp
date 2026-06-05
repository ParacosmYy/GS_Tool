/**
 * @file algo_907.cpp
 * @brief Algorithm module 907
 */
#include "dsp907/algo_907.h"
QVector<double> algo_907::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
