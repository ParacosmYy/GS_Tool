/**
 * @file algo_2513.cpp
 * @brief Algorithm module 2513
 */
#include "crypto2513/algo_2513.h"
QVector<double> algo_2513::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
