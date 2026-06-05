/**
 * @file algo_1977.cpp
 * @brief Algorithm module 1977
 */
#include "image1977/algo_1977.h"
QVector<double> algo_1977::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
