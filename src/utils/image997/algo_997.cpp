/**
 * @file algo_997.cpp
 * @brief Algorithm module 997
 */
#include "image997/algo_997.h"
QVector<double> algo_997::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
