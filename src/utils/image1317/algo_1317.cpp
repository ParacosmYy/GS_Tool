/**
 * @file algo_1317.cpp
 * @brief Algorithm module 1317
 */
#include "image1317/algo_1317.h"
QVector<double> algo_1317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
