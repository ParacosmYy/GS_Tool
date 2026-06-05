/**
 * @file algo_1227.cpp
 * @brief Algorithm module 1227
 */
#include "dsp1227/algo_1227.h"
QVector<double> algo_1227::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
