/**
 * @file algo_1409.cpp
 * @brief Algorithm module 1409
 */
#include "code1409/algo_1409.h"
QVector<double> algo_1409::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
