/**
 * @file PerformanceOverlay.cpp
 * @brief 性能叠加显示控件实现
 *
 * 悬浮面板，实时展示 FPS、帧耗时与内存占用。
 */

#include "utils/perf/PerformanceOverlay.h"
#include "utils/perf/PerformanceMonitor.h"

#include <QVBoxLayout>
#include <QFont>

/** @brief 构造函数，初始化悬浮性能面板UI @param parent 父控件 */
PerformanceOverlay::PerformanceOverlay(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("PerformanceOverlay");
    setupUI();
}

/** @brief 析构函数 */
PerformanceOverlay::~PerformanceOverlay() = default;

/** @brief 设置性能监视器数据源，连接statsUpdated信号到onStatsUpdated槽 @param monitor 监视器实例 */
void PerformanceOverlay::setMonitor(PerformanceMonitor *monitor)
{
    m_monitor = monitor;
    if (m_monitor) {
        connect(m_monitor, &PerformanceMonitor::statsUpdated,
                this,      &PerformanceOverlay::onStatsUpdated);
    }
}

/** @brief 统计数据更新回调（由信号触发），委托给updateStats @param fps 帧率 @param avgFrameMs 平均帧耗时 @param memBytes 内存占用 */
void PerformanceOverlay::onStatsUpdated(double fps, double avgFrameMs, qint64 memBytes)
{
    updateStats(fps, avgFrameMs, memBytes);
}

/** @brief 手动更新统计数据显示(FPS/帧耗时/内存)，同时跟踪FPS历史和极值 @param fps 帧率 @param avgFrameMs 平均帧耗时（ms） @param memBytes 内存占用（字节） */
void PerformanceOverlay::updateStats(double fps, double avgFrameMs, qint64 memBytes)
{
    ++m_totalUpdates;

    /* FPS历史跟踪 */
    m_currentFps = fps;
    m_lastMemBytes = memBytes;
    if (fps > 0) {
        ++m_totalFpsSamples;
        if (fps < m_fpsWarningThreshold) ++m_totalLowFpsWarnings;
        if (fps < m_minFps) m_minFps = fps;
        if (fps > m_maxFps) m_maxFps = fps;
        m_fpsHistory.append(fps);
        if (m_fpsHistory.size() > kFpsHistorySize) {
            m_fpsHistory.removeFirst();
        }
    }
    if (avgFrameMs > 0.0) {
        ++m_totalPipelineLatencySamples;
    }
    if (memBytes > 0) {
        ++m_totalThroughputSamples;
    }

    // FPS 与帧耗时
    m_fpsLabel->setText(tr("FPS: %1 | %2ms")
                            .arg(fps, 0, 'f', 1)
                            .arg(avgFrameMs, 0, 'f', 1));

    // 内存格式化：<1MB → KB，<1GB → MB，否则 GB
    QString memFormatted;
    const double kb = static_cast<double>(memBytes) / 1024.0;
    const double mb = kb / 1024.0;
    const double gb = mb / 1024.0;

    if (mb < 1.0) {
        memFormatted = tr("%1 KB").arg(static_cast<int>(kb));
    } else if (gb < 1.0) {
        memFormatted = tr("%1 MB").arg(mb, 0, 'f', 1);
    } else {
        memFormatted = tr("%1 GB").arg(gb, 0, 'f', 2);
    }

    m_memLabel->setText(tr("MEM: %1").arg(memFormatted));
}

/** @brief 初始化悬浮面板UI(FPS标签+内存标签，鼠标穿透，固定宽度150px) */
void PerformanceOverlay::setupUI()
{
    // 右对齐的垂直布局
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(2);
    layout->setAlignment(Qt::AlignRight | Qt::AlignTop);

    // FPS 标签
    m_fpsLabel = new QLabel(tr("FPS: --"), this);
    m_fpsLabel->setObjectName("fpsLabel");
    QFont fpsFont = m_fpsLabel->font();
    fpsFont.setPointSize(10);
    m_fpsLabel->setFont(fpsFont);

    // 内存标签
    m_memLabel = new QLabel(tr("MEM: --"), this);
    m_memLabel->setObjectName("memLabel");
    QFont memFont = m_memLabel->font();
    memFont.setPointSize(10);
    m_memLabel->setFont(memFont);

    layout->addWidget(m_fpsLabel);
    layout->addWidget(m_memLabel);
    setLayout(layout);

    // 叠加层属性：鼠标穿透、固定宽度
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFixedWidth(150);
}

// fpsStats/setFpsWarningThreshold/isBelowFpsThreshold/performanceSummary/resetStats/resetOverlayStatistics
// 已移至 PerformanceOverlayStats.cpp
