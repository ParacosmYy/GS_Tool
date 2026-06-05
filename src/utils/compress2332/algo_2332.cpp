/**
 * @file algo_2332.cpp
 * @brief Algorithm module 2332
 */
#include "compress2332/algo_2332.h"
QVector<double> algo_2332::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
