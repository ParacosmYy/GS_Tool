/**
 * @file algo_877.cpp
 * @brief Algorithm module 877
 */
#include "image877/algo_877.h"
QVector<double> algo_877::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
