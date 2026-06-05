/**
 * @file algo_2173.cpp
 * @brief Algorithm module 2173
 */
#include "crypto2173/algo_2173.h"
QVector<double> algo_2173::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
