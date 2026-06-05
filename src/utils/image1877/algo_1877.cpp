/**
 * @file algo_1877.cpp
 * @brief Algorithm module 1877
 */
#include "image1877/algo_1877.h"
QVector<double> algo_1877::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
