/**
 * @file ProtocolTimerWidget.h
 * @brief 协议定时分析器控件 -- 测量协议帧间时序、响应延迟、突发检测
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供协议定时分析功能：支持帧间延迟(InterFrame)、响应延迟(ResponseLatency)、
 * 突发检测(Burst)、手动触发(Manual)四种测量模式。实时显示当前时序值(ms)，
 * 统计 min/max/avg/jitter/stddev，检测突发帧(100ms内>5帧)，支持CSV导出。
 */

#ifndef PROTOCOLTIMERWIDGET_H
#define PROTOCOLTIMERWIDGET_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QList>
#include <QString>
#include <QTimer>
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;

/**
 * @class ProtocolTimerWidget
 * @brief 协议定时分析器控件，测量协议帧间时序和突发模式
 */
class ProtocolTimerWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 测量模式 */
    enum class TimingMode {
        InterFrame,       ///< 帧间延迟：连续两帧之间的时间间隔
        ResponseLatency,  ///< 响应延迟：发送到收到响应的时间
        Burst,            ///< 突发检测：自动识别高密度帧区域
        Manual            ///< 手动模式：由用户手动标记起止事件
    };
    Q_ENUM(TimingMode)

    /** @brief 定时事件记录 */
    struct TimingEvent {
        qint64    timestampNs = 0; ///< 纳秒时间戳(自启动起)
        QByteArray data;           ///< 关联数据
        QString    label;          ///< 事件标签
    };

    /** @brief 时序统计结果 */
    struct TimingStats {
        double  minMs    = 0.0;  ///< 最小间隔(ms)
        double  maxMs    = 0.0;  ///< 最大间隔(ms)
        double  avgMs    = 0.0;  ///< 平均间隔(ms)
        double  jitterMs = 0.0;  ///< 抖动(ms): 相邻间隔差的均值
        double  stddevMs = 0.0;  ///< 标准差(ms)
        quint64 count    = 0;    ///< 有效测量数
    };

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalEvents       = 0; ///< 总事件数
        quint64 totalMeasurements = 0; ///< 总测量次数
        quint64 totalResets       = 0; ///< 总重置次数
        quint64 totalExports      = 0; ///< 总导出次数
        double  peakRateHz        = 0; ///< 峰值事件率(Hz)
    };

    /** @brief 构造协议定时分析器 @param parent 父控件 */
    explicit ProtocolTimerWidget(QWidget *parent = nullptr);

    /** @brief 记录一个协议事件 @param data 事件关联数据 @param label 事件标签 */
    void recordEvent(const QByteArray &data, const QString &label = {});

    /** @brief 开始测量 */
    void startMeasurement();

    /** @brief 停止测量 */
    void stopMeasurement();

    /** @brief 设置测量模式 @param mode 目标模式 */
    void setMode(TimingMode mode);

    /** @brief 计算当前时序统计 @return 统计结果快照 */
    TimingStats timingStatistics() const;

    /** @brief 导出时序数据为CSV @param filePath 目标文件路径 */
    void exportTiming(const QString &filePath);

    /** @brief 获取全局统计 @return 当前统计快照 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置所有统计信息 -- 实现在 ProtocolTimerWidgetStats.cpp */
    void resetStatistics();

signals:
    /** @brief 时序更新信号 @param ms 最新间隔(ms) */
    void timingUpdated(double ms);

    /** @brief 突发帧检测信号 @param count 突发帧数 */
    void burstDetected(quint64 count);

    /** @brief 测量完成信号 @param stats 最终统计结果 */
    void measurementComplete(const TimingStats &stats);

private slots:
    void onStartClicked();     ///< 开始按钮点击
    void onStopClicked();      ///< 停止按钮点击
    void onResetClicked();     ///< 重置按钮点击
    void onExportClicked();    ///< 导出按钮点击
    void onModeChanged(int index); ///< 模式选择变更

private:
    void setupUI();            ///< 初始化界面布局
    void updateDisplay();      ///< 刷新时序显示
    void refreshStatsLabels(); ///< 刷新统计标签
    void addEventRow(int row, const TimingEvent &evt, double deltaMs); ///< 添加表格行
    void detectBurst();        ///< 突发帧检测

    // ---- UI 控件 ----
    QComboBox    *m_modeCombo  = nullptr; ///< 模式选择器
    QPushButton  *m_startBtn   = nullptr; ///< 开始按钮
    QPushButton  *m_stopBtn    = nullptr; ///< 停止按钮
    QPushButton  *m_resetBtn   = nullptr; ///< 重置按钮
    QPushButton  *m_exportBtn  = nullptr; ///< 导出按钮
    QLabel       *m_currentLabel = nullptr; ///< 当前时序大字显示
    QLabel       *m_minLabel   = nullptr; ///< 最小值标签
    QLabel       *m_maxLabel   = nullptr; ///< 最大值标签
    QLabel       *m_avgLabel   = nullptr; ///< 平均值标签
    QLabel       *m_jitterLabel = nullptr; ///< 抖动标签
    QLabel       *m_burstLabel = nullptr; ///< 突发指示标签
    QTableWidget *m_eventTable = nullptr; ///< 事件列表

    // ---- 定时器 ----
    QElapsedTimer  m_elapsedTimer;  ///< 高精度运行计时器
    QTimer        *m_burstTimer    = nullptr; ///< 突发检测滑动窗口定时器

    // ---- 测量状态 ----
    bool       m_measuring = false; ///< 是否正在测量
    TimingMode m_mode = TimingMode::InterFrame; ///< 当前模式
    QList<TimingEvent> m_events;   ///< 事件记录列表

    // ---- 突发检测缓冲 ----
    QList<qint64> m_burstWindow;   ///< 滑动窗口内的纳秒时间戳
    quint64       m_burstThreshold = 5;   ///< 突发判定帧数阈值
    qint64        m_burstWindowNs  = 100'000'000LL; ///< 突发窗口(100ms=1e8ns)

    // ---- 统计 ----
    Stats m_stats;                  ///< 全局统计
};

#endif // PROTOCOLTIMERWIDGET_H
