/**
 * @file tree__531.h
 * @brief tree module tree__531
 */
#pragma once
#include <QObject>
#include <QVector>
class tree__531 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit tree__531(QObject *p=nullptr) : QObject(p) {}
    ~tree__531() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

