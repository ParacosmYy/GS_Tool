/**
 * @file AhoCorasick2.h
 * @brief crypto algorithm module - AhoCorasick2
 */
#pragma once
#include <QObject>
#include <QVector>
class AhoCorasick2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit AhoCorasick2(QObject *p = nullptr) : QObject(p) {}
    ~AhoCorasick2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

