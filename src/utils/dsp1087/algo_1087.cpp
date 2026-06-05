/**
 * @file algo_1087.cpp
 * @brief Algorithm module 1087
 */
#include "dsp1087/algo_1087.h"
QVector<double> algo_1087::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
