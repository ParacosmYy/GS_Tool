/**
 * @file algo_1513.cpp
 * @brief Algorithm module 1513
 */
#include "crypto1513/algo_1513.h"
QVector<double> algo_1513::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
