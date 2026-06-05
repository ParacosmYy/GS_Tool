/**
 * @file algo_1527.cpp
 * @brief Algorithm module 1527
 */
#include "dsp1527/algo_1527.h"
QVector<double> algo_1527::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
