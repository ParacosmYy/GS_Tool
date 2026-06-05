/**
 * @file algo_2007.cpp
 * @brief Algorithm module 2007
 */
#include "dsp2007/algo_2007.h"
QVector<double> algo_2007::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
