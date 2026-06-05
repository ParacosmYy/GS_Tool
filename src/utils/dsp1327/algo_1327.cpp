/**
 * @file algo_1327.cpp
 * @brief Algorithm module 1327
 */
#include "dsp1327/algo_1327.h"
QVector<double> algo_1327::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
