/**
 * @file algo_1501.cpp
 * @brief Algorithm module 1501
 */
#include "interp1501/algo_1501.h"
QVector<double> algo_1501::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
