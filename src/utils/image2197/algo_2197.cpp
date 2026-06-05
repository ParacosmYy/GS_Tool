/**
 * @file algo_2197.cpp
 * @brief Algorithm module 2197
 */
#include "image2197/algo_2197.h"
QVector<double> algo_2197::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
