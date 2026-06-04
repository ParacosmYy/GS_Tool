/**
 * @file FrequencyCounterWidget.h
 * @brief 频率计数器控件 -- 测量数据流中的数据包频率、字节吞吐率及模式重复率
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供软件频率计功能：在可配置的滑动时间窗口(1/5/10/60秒)内
 * 统计数据包到达频率(Hz)、字节吞吐率(B/s)、特定模式出现频率，
 * 并实时显示当前值、峰值、均值及累计计数。
 */

#ifndef FREQUENCYCOUNTERWIDGET_H
#define FREQUENCYCOUNTERWIDGET_H

#include <QElapsedTimer>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;

/**
 * @class FrequencyCounterWidget
 * @brief 软件频率计数器控件，测量数据流中的频率和模式重复率
 */
class FrequencyCounterWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 测量窗口大小 */
    enum class WindowSize {
        Sec1  = 1,  ///< 1秒窗口
        Sec5  = 5,  ///< 5秒窗口
        Sec10 = 10, ///< 10秒窗口
        Sec60 = 60  ///< 60秒窗口
    };
    Q_ENUM(WindowSize)

    /** @brief 频率通道，描述单个被测通道的实时/峰值/平均频率 */
    struct FreqChannel {
        QString name;            ///< 通道名称
        double currentFreq = 0;  ///< 当前频率(Hz)
        double peakFreq    = 0;  ///< 峰值频率(Hz)
        double avgFreq     = 0;  ///< 平均频率(Hz)
        quint64 totalEvents = 0; ///< 累计事件数
    };

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalEvents         = 0; ///< 总事件数
        quint64 totalBytesProcessed  = 0; ///< 总处理字节数
        quint64 totalWindows         = 0; ///< 累计窗口数
        double  peakPacketRate       = 0; ///< 峰值数据包率(Hz)
        double  peakByteRate         = 0; ///< 峰值字节率(B/s)
        int     activeChannels       = 0; ///< 活跃通道数
    };

    /** @brief 构造频率计数器控件 @param parent 父控件 */
    explicit FrequencyCounterWidget(QWidget *parent = nullptr);

    /** @brief 记录一次数据包到达事件 */
    void recordPacket();

    /** @brief 记录接收到的字节数 @param bytes 字节数 */
    void recordBytes(qint64 bytes);

    /** @brief 记录特定模式的一次匹配 @param pattern 模式名称 */
    void recordPattern(const QString &pattern);

    /** @brief 设置测量窗口大小 @param size 窗口大小 */
    void setWindowSize(WindowSize size);

    /** @brief 获取全局统计 @return 当前统计快照 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置所有计数器和统计 */
    void resetStatistics();

signals:
    /** @brief 频率更新信号 @param channel 通道名称 @param freq 当前频率 */
    void frequencyUpdated(const QString &channel, double freq);

    /** @brief 测量窗口到期信号 */
    void windowExpired();

private slots:
    void onWindowTick();      ///< 每秒定时器回调
    void onResetClicked();    ///< 重置按钮点击
    void onWindowChanged(int index); ///< 窗口选择变更

private:
    void setupUI();           ///< 初始化界面布局
    void updateDisplay();     ///< 刷新频率显示
    void updatePatternTable(double ws); ///< 更新模式频率表

    // ---- UI 控件 ----
    QLabel       *m_packetFreqLabel;  ///< 数据包频率大数字显示
    QLabel       *m_byteRateLabel;    ///< 字节率显示
    QLabel       *m_peakLabel;        ///< 峰值频率显示
    QLabel       *m_avgLabel;         ///< 平均频率显示
    QLabel       *m_totalLabel;       ///< 累计显示
    QTableWidget *m_patternTable;     ///< 模式频率表
    QPushButton  *m_resetBtn;         ///< 重置按钮
    QComboBox    *m_windowCombo;      ///< 窗口选择

    // ---- 定时器 ----
    QTimer       m_windowTimer;       ///< 每秒测量窗口定时器
    QElapsedTimer m_elapsedTimer;     ///< 总运行计时器

    // ---- 当前窗口计数器 ----
    quint64 m_windowPackets  = 0;     ///< 当前窗口数据包计数
    quint64 m_windowBytes    = 0;     ///< 当前窗口字节计数
    QMap<QString, quint64> m_windowPatterns; ///< 当前窗口模式计数

    // ---- 历史统计 ----
    double  m_lastPacketFreq   = 0;   ///< 上次数据包频率
    double  m_lastByteRate     = 0;   ///< 上次字节率
    double  m_peakPacketFreq   = 0;   ///< 峰值数据包频率
    double  m_peakByteRateLocal = 0;  ///< 峰值字节率
    quint64 m_totalPackets     = 0;   ///< 总数据包数
    quint64 m_totalBytes       = 0;   ///< 总字节数
    double  m_freqSum          = 0;   ///< 频率累计(用于计算平均)
    int     m_windowCount      = 0;   ///< 已完成窗口计数

    WindowSize m_windowSize = WindowSize::Sec1; ///< 当前窗口大小

    Stats m_stats;                    ///< 全局统计
};

#endif // FREQUENCYCOUNTERWIDGET_H
