/**
 * @file algo_2257.cpp
 * @brief Algorithm module 2257
 */
#include "image2257/algo_2257.h"
QVector<double> algo_2257::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
