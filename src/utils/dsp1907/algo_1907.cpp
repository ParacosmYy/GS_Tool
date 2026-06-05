/**
 * @file algo_1907.cpp
 * @brief Algorithm module 1907
 */
#include "dsp1907/algo_1907.h"
QVector<double> algo_1907::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
