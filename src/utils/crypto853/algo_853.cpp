/**
 * @file algo_853.cpp
 * @brief Algorithm module 853
 */
#include "crypto853/algo_853.h"
QVector<double> algo_853::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
