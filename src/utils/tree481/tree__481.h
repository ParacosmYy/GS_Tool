/**
 * @file tree__481.h
 * @brief tree module tree__481
 */
#pragma once
#include <QObject>
#include <QVector>
class tree__481 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit tree__481(QObject *p=nullptr) : QObject(p) {}
    ~tree__481() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

