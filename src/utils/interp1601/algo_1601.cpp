/**
 * @file algo_1601.cpp
 * @brief Algorithm module 1601
 */
#include "interp1601/algo_1601.h"
QVector<double> algo_1601::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
