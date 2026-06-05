/**
 * @file algo_1722.cpp
 * @brief Algorithm module 1722
 */
#include "poly1722/algo_1722.h"
QVector<double> algo_1722::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
