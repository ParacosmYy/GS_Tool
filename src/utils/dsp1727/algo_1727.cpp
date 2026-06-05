/**
 * @file algo_1727.cpp
 * @brief Algorithm module 1727
 */
#include "dsp1727/algo_1727.h"
QVector<double> algo_1727::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
