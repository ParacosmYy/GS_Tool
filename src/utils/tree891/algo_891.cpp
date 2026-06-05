/**
 * @file algo_891.cpp
 * @brief Algorithm module 891
 */
#include "tree891/algo_891.h"
QVector<double> algo_891::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
