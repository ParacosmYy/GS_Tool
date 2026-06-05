/**
 * @file tree__411.h
 * @brief tree module tree__411
 */
#pragma once
#include <QObject>
#include <QVector>
class tree__411 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit tree__411(QObject *p=nullptr) : QObject(p) {}
    ~tree__411() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

