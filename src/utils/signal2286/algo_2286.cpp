/**
 * @file algo_2286.cpp
 * @brief Algorithm module 2286
 */
#include "signal2286/algo_2286.h"
QVector<double> algo_2286::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
