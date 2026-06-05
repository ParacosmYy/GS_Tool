/**
 * @file algo_893.cpp
 * @brief Algorithm module 893
 */
#include "crypto893/algo_893.h"
QVector<double> algo_893::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
