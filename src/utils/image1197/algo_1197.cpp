/**
 * @file algo_1197.cpp
 * @brief Algorithm module 1197
 */
#include "image1197/algo_1197.h"
QVector<double> algo_1197::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
