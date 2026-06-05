/**
 * @file algo_2037.cpp
 * @brief Algorithm module 2037
 */
#include "image2037/algo_2037.h"
QVector<double> algo_2037::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
