/**
 * @file algo_1422.cpp
 * @brief Algorithm module 1422
 */
#include "poly1422/algo_1422.h"
QVector<double> algo_1422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
