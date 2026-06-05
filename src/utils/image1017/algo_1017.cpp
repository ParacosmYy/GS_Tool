/**
 * @file algo_1017.cpp
 * @brief Algorithm module 1017
 */
#include "image1017/algo_1017.h"
QVector<double> algo_1017::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
