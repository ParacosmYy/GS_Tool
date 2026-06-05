/**
 * @file algo_1047.cpp
 * @brief Algorithm module 1047
 */
#include "dsp1047/algo_1047.h"
QVector<double> algo_1047::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
