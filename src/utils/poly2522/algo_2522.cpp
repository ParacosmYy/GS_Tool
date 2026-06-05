/**
 * @file algo_2522.cpp
 * @brief Algorithm module 2522
 */
#include "poly2522/algo_2522.h"
QVector<double> algo_2522::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
