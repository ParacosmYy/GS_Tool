#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QDateTime>

class ConnectionMonitor : public QObject {
    Q_OBJECT
public:
    enum State { Disconnected, Connecting, Connected, Reconnecting, Error };
    Q_ENUM(State)
    struct Stats {
        State currentState = Disconnected;
        qint64 connectedSince = 0;
        qint64 totalConnectedTime = 0;
        int reconnectCount = 0;
        int errorCount = 0;
        qint64 bytesSent = 0;
        qint64 bytesReceived = 0;
        double latencyMs = 0.0;
        QString lastError;
    };

    explicit ConnectionMonitor(QObject *parent = nullptr);
    ~ConnectionMonitor() override;
    void setState(State state);
    State state() const;
    Stats stats() const;
    void recordBytesSent(qint64 bytes);
    void recordBytesReceived(qint64 bytes);
    void recordLatency(double ms);
    void recordError(const QString &error);
    void startMonitoring(int pingIntervalMs = 5000);
    void stopMonitoring();
    void resetStats();
    QString stateString() const;
signals:
    void stateChanged(State newState, State oldState);
    void statsUpdated(const Stats &stats);
    void connectionLost();
    void connectionRestored();
    void latencyWarning(double ms);
private:
    void onPingTimer();
    Stats m_stats;
    QTimer *m_pingTimer = nullptr;
    int m_pingInterval = 5000;
    double m_latencyThreshold = 500.0;
};
