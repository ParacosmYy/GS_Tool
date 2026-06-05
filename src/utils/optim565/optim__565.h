/**
 * @file optim__565.h
 * @brief optim module optim__565
 */
#pragma once
#include <QObject>
#include <QVector>
class optim__565 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit optim__565(QObject *p=nullptr) : QObject(p) {}
    ~optim__565() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

