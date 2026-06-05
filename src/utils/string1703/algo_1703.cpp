/**
 * @file algo_1703.cpp
 * @brief Algorithm module 1703
 */
#include "string1703/algo_1703.h"
QVector<double> algo_1703::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
