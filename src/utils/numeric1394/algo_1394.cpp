/**
 * @file algo_1394.cpp
 * @brief Algorithm module 1394
 */
#include "numeric1394/algo_1394.h"
QVector<double> algo_1394::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
