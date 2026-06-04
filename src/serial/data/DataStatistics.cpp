/**
 * @file DataStatistics.cpp
 * @brief 数据统计面板实现 - 构造函数与UI初始化
 *
 * 增强功能:
 *   - 滚动窗口吞吐量: 基于最近5秒的滑动窗口计算平均速率
 *   - 吞吐量直方图: 将速率采样分桶统计(10个桶，覆盖0~10MB/s)
 *   - 采样历史: 记录最近300个采样点(5分钟)，供外部绘制速率曲线
 *   - 使用ByteFormat进行字节格式化
 *
 * update/reset/rate查询见 DataStatisticsUpdate.cpp
 * onRefreshTimer见 DataStatisticsRefresh.cpp
 * 显示格式化+错误显示+健康状态+会话摘要+统计查询getter见 DataStatisticsDisplay.cpp
 * 统计计数器+滚动窗口+直方图+采样历史见 DataStatisticsRolling.cpp
 */
#include "serial/data/DataStatistics.h"
#include "shared/TimerConstants.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLatin1Char>

/** @brief 构造函数，初始化UI、滚动窗口和直方图 @param parent 父控件 */
DataStatistics::DataStatistics(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("dataStatistics");

    setupUI();

    // 初始化直方图桶
    initHistogramBuckets();

    // 启动1秒定时器，用于刷新速率显示和持续时间
    connect(&m_refreshTimer, &QTimer::timeout,
            this, &DataStatistics::onRefreshTimer);
    m_refreshTimer.setInterval(Timers::kDataStatsRefreshMs);

    // 启动采样间隔计时器（用于计算精确速率）
    m_sampleTimer.start();

    // 启动持续时间计时器
    m_stopwatch.start();
    m_refreshTimer.start();
}

/** @brief 初始化统计面板UI(收发字节/速率/峰值/均值/滚动速率/时间标签) */
void DataStatistics::setupUI()
{
    // 主布局：上下排列，紧凑边距
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ---- 收发统计区域(使用统一框架) ----
    mainLayout->addWidget(createStatsFrame(tr("接收:"), m_rxTotalLabel, "rxTotalLabel", "statsRxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("速率:"), m_rxRateLabel, "rxRateLabel", "statsRxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("发送:"), m_txTotalLabel, "txTotalLabel", "statsTxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("速率:"), m_txRateLabel, "txRateLabel", "statsTxFrame"));
    mainLayout->addWidget(createStatsFrame(tr("峰值:"), m_peakRateLabel, "peakRateLabel", "statsPeakFrame"));
    mainLayout->addWidget(createStatsFrame(tr("均值:"), m_avgRateLabel, "avgRateLabel", "statsAvgFrame"));
    mainLayout->addWidget(createStatsFrame(tr("滚动速率:"), m_rollingRateLabel, "rollingRateLabel", "statsRollingFrame"));
    mainLayout->addWidget(createStatsFrame(tr("时间:"), m_elapsedLabel, "elapsedLabel", "statsTimeFrame"));

    // ---- 串口错误计数（默认隐藏，有错误时显示） ----
    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    // ---- 连接健康状态（默认隐藏，空闲超10秒时显示） ----
    m_healthLabel = new QLabel;
    m_healthLabel->setObjectName("healthStatusLabel");
    m_healthLabel->setWordWrap(true);
    m_healthLabel->hide();
    mainLayout->addWidget(m_healthLabel);

    // 底部弹性空间
    mainLayout->addStretch();
}

/** @brief 创建统计项框架 @param label 左侧标签文字 @param valueLabel 值标签引用(输出) @param objectName 值标签objectName @param frameName 框架objectName @return QFrame指针 */
QFrame* DataStatistics::createStatsFrame(const QString& label, QLabel*& valueLabel, const QString& objectName, const QString& frameName)
{
    auto* frame = new QFrame;
    frame->setObjectName(frameName.isEmpty() ? QStringLiteral("statsFrame") : frameName);
    frame->setFrameShape(QFrame::StyledPanel);
    auto* layout = new QFormLayout(frame);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(4);
    layout->setLabelAlignment(Qt::AlignRight);

    valueLabel = new QLabel(tr("0 B"));
    valueLabel->setObjectName(objectName);
    layout->addRow(label, valueLabel);
    return frame;
}
