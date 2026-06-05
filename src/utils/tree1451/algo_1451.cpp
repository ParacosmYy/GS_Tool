/**
 * @file algo_1451.cpp
 * @brief Algorithm module 1451
 */
#include "tree1451/algo_1451.h"
QVector<double> algo_1451::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
