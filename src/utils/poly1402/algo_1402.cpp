/**
 * @file algo_1402.cpp
 * @brief Algorithm module 1402
 */
#include "poly1402/algo_1402.h"
QVector<double> algo_1402::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
