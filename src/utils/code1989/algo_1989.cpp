/**
 * @file algo_1989.cpp
 * @brief Algorithm module 1989
 */
#include "code1989/algo_1989.h"
QVector<double> algo_1989::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
