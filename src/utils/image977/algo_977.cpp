/**
 * @file algo_977.cpp
 * @brief Algorithm module 977
 */
#include "image977/algo_977.h"
QVector<double> algo_977::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
