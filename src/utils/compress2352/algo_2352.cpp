/**
 * @file algo_2352.cpp
 * @brief Algorithm module 2352
 */
#include "compress2352/algo_2352.h"
QVector<double> algo_2352::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
