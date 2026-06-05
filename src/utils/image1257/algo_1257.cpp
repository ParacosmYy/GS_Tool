/**
 * @file algo_1257.cpp
 * @brief Algorithm module 1257
 */
#include "image1257/algo_1257.h"
QVector<double> algo_1257::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
