/**
 * @file algo_2109.cpp
 * @brief Algorithm module 2109
 */
#include "code2109/algo_2109.h"
QVector<double> algo_2109::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
