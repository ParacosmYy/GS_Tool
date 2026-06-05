/**
 * @file algo_2747.cpp
 * @brief Algorithm module 2747
 */
#include "dsp2747/algo_2747.h"
QVector<double> algo_2747::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
