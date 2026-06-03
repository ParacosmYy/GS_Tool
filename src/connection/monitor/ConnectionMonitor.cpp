#include "connection/monitor/ConnectionMonitor.h"

ConnectionMonitor::ConnectionMonitor(QObject *parent) : QObject(parent), m_pingTimer(new QTimer(this)) {
    connect(m_pingTimer, &QTimer::timeout, this, &ConnectionMonitor::onPingTimer);
}
ConnectionMonitor::~ConnectionMonitor() { stopMonitoring(); }

void ConnectionMonitor::setState(State s) {
    if (s == m_stats.currentState) return;
    State old = m_stats.currentState;
    if (old == Connected && s == Disconnected) {
        m_stats.totalConnectedTime += QDateTime::currentMSecsSinceEpoch() - m_stats.connectedSince;
        emit connectionLost();
    }
    if (s == Connected) {
        m_stats.connectedSince = QDateTime::currentMSecsSinceEpoch();
        if (old == Reconnecting) emit connectionRestored();
    }
    if (s == Reconnecting) m_stats.reconnectCount++;
    m_stats.currentState = s;
    emit stateChanged(s, old);
    emit statsUpdated(m_stats);
}

ConnectionMonitor::State ConnectionMonitor::state() const { return m_stats.currentState; }
ConnectionMonitor::Stats ConnectionMonitor::stats() const { return m_stats; }

void ConnectionMonitor::recordBytesSent(qint64 b) { m_stats.bytesSent += b; emit statsUpdated(m_stats); }
void ConnectionMonitor::recordBytesReceived(qint64 b) { m_stats.bytesReceived += b; emit statsUpdated(m_stats); }
void ConnectionMonitor::recordLatency(double ms) {
    m_stats.latencyMs = ms;
    if (ms > m_latencyThreshold) emit latencyWarning(ms);
    emit statsUpdated(m_stats);
}
void ConnectionMonitor::recordError(const QString &e) { m_stats.errorCount++; m_stats.lastError = e; emit statsUpdated(m_stats); }

void ConnectionMonitor::startMonitoring(int interval) {
    m_pingInterval = interval;
    m_pingTimer->start(interval);
}
void ConnectionMonitor::stopMonitoring() { m_pingTimer->stop(); }
void ConnectionMonitor::resetStats() { m_stats = Stats{}; }

QString ConnectionMonitor::stateString() const {
    switch (m_stats.currentState) {
    case Disconnected: return tr("Disconnected");
    case Connecting: return tr("Connecting");
    case Connected: return tr("Connected");
    case Reconnecting: return tr("Reconnecting");
    case Error: return tr("Error");
    }
    return tr("Unknown");
}

void ConnectionMonitor::onPingTimer() { emit statsUpdated(m_stats); }
