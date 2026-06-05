/**
 * @file algo_2261.cpp
 * @brief Algorithm module 2261
 */
#include "interp2261/algo_2261.h"
QVector<double> algo_2261::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
