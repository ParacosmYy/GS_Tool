/**
 * @file algo_1747.cpp
 * @brief Algorithm module 1747
 */
#include "dsp1747/algo_1747.h"
QVector<double> algo_1747::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
