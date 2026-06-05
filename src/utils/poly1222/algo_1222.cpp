/**
 * @file algo_1222.cpp
 * @brief Algorithm module 1222
 */
#include "poly1222/algo_1222.h"
QVector<double> algo_1222::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
