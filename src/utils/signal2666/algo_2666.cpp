/**
 * @file algo_2666.cpp
 * @brief Algorithm module 2666
 */
#include "signal2666/algo_2666.h"
QVector<double> algo_2666::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
