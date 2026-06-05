/**
 * @file algo_1522.cpp
 * @brief Algorithm module 1522
 */
#include "poly1522/algo_1522.h"
QVector<double> algo_1522::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
