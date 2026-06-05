/**
 * @file ProcessMonitor.cpp
 * @brief 多进程指标监控器实现
 */

#include "utils/multiproc/ProcessMonitor.h"

ProcessMonitor::ProcessMonitor(QObject* parent)
    : QObject(parent), m_cpuThreshold(90.0), m_maxHistory(1000) {}

void ProcessMonitor::registerProcess(int pid, const QString& name)
{
    ProcessInfo info;
    info.name = name;
    m_processes[pid] = info;
    m_stats.totalProcesses = m_processes.size();
}

void ProcessMonitor::unregisterProcess(int pid)
{
    m_processes.remove(pid);
    m_stats.totalProcesses = m_processes.size();
}

void ProcessMonitor::updateMetrics(int pid, const Metrics& metrics)
{
    if (!m_processes.contains(pid)) return;

    auto& proc = m_processes[pid];
    proc.current = metrics;

    proc.history.append(metrics);
    if (proc.history.size() > m_maxHistory) {
        proc.history.removeFirst();
    }

    m_stats.totalSamples++;

    emit metricsUpdated(pid, metrics);

    if (metrics.cpuUsage > m_cpuThreshold) {
        m_stats.totalAlerts++;
        emit cpuAlert(pid, metrics.cpuUsage, m_cpuThreshold);
    }
}

void ProcessMonitor::setCpuAlertThreshold(double threshold)
{
    m_cpuThreshold = qBound(0.0, threshold, 100.0);
}

QVector<ProcessMonitor::Metrics> ProcessMonitor::getHistory(
    int pid, int maxSamples) const
{
    if (!m_processes.contains(pid)) return {};

    const auto& hist = m_processes[pid].history;
    int start = qMax(0, hist.size() - maxSamples);
    return hist.mid(start);
}

QMap<int, ProcessMonitor::Metrics> ProcessMonitor::currentSnapshot() const
{
    QMap<int, Metrics> snapshot;
    for (auto it = m_processes.constBegin();
         it != m_processes.constEnd(); ++it) {
        snapshot[it.key()] = it.value().current;
    }
    return snapshot;
}

void ProcessMonitor::resetStatistics()
{
    m_stats = Stats{};
    m_stats.totalProcesses = m_processes.size();
}
