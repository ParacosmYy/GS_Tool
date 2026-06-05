/**
 * @file algo_937.cpp
 * @brief Algorithm module 937
 */
#include "image937/algo_937.h"
QVector<double> algo_937::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
