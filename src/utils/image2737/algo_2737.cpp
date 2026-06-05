/**
 * @file algo_2737.cpp
 * @brief Algorithm module 2737
 */
#include "image2737/algo_2737.h"
QVector<double> algo_2737::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
