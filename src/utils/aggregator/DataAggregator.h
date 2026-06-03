#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QTimer>

class DataAggregator : public QObject {
    Q_OBJECT
public:
    enum AggregateFunc { Sum, Average, Min, Max, Count, First, Last };
    Q_ENUM(AggregateFunc)

    explicit DataAggregator(QObject *parent = nullptr);
    ~DataAggregator() override;

    void addSource(const QString &name, AggregateFunc func, int windowSize = 100);
    void removeSource(const QString &name);
    void feedValue(const QString &source, double value);
    double aggregateResult(const QString &source) const;
    QMap<QString, double> allResults() const;
    QStringList sources() const;
    void setWindowSize(const QString &source, int size);
    void setAggregateFunc(const QString &source, AggregateFunc func);
    void resetSource(const QString &source);
    void resetAll();

signals:
    void valueAggregated(const QString &source, double result);
    void sourceAdded(const QString &name);
    void sourceRemoved(const QString &name);

private:
    void computeAggregate(const QString &source);
    struct SourceConfig { AggregateFunc func; int windowSize; QList<double> values; double result = 0.0; };
    QMap<QString, SourceConfig> m_sources;
};
