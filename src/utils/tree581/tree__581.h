/**
 * @file tree__581.h
 * @brief tree module tree__581
 */
#pragma once
#include <QObject>
#include <QVector>
class tree__581 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit tree__581(QObject *p=nullptr) : QObject(p) {}
    ~tree__581() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

