/**
 * @file graph__474.h
 * @brief graph module graph__474
 */
#pragma once
#include <QObject>
#include <QVector>
class graph__474 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit graph__474(QObject *p=nullptr) : QObject(p) {}
    ~graph__474() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

