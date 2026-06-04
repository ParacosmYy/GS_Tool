/**
 * @file SignalGeneratorWidget.h
 * @brief 信号发生器面板 -- 生成正弦/方波/三角/锯齿/噪声测试信号并转换为字节流
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 用于串口调试场景下产生已知波形数据，验证数据链路正确性。
 * 支持自动周期发送，统计接口可追溯生成/输出/错误次数。
 */

#ifndef SIGNALGENERATORWIDGET_H
#define SIGNALGENERATORWIDGET_H

#include <QByteArray>
#include <QComboBox>
#include <QLabel>
#include <QProgressBar>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QWidget>

/**
 * @class SignalGeneratorWidget
 * @brief 信号发生器面板，生成标准测试波形并按指定格式输出字节流
 */
class SignalGeneratorWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 波形类型枚举 */
    enum SignalType {
        Sine,       ///< 正弦波
        Square,     ///< 方波
        Triangle,   ///< 三角波
        Sawtooth,   ///< 锯齿波
        Noise       ///< 随机噪声
    };
    Q_ENUM(SignalType)

    /** @brief 输出字节格式枚举 */
    enum OutputFormat {
        Uint8,      ///< 无符号8位整数
        Int16,      ///< 有符号16位整数
        Uint16,     ///< 无符号16位整数
        Float32     ///< 32位浮点数
    };
    Q_ENUM(OutputFormat)

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalGenerations = 0;       ///< 累计生成次数
        quint64 totalSamplesGenerated = 0;  ///< 累计采样点数
        quint64 totalBytesOutput = 0;       ///< 累计输出字节数
        quint64 totalOutputs = 0;           ///< 累计输出次数
        quint64 errorCount = 0;             ///< 累计错误次数
        quint64 peakSamplesPerGen = 0;      ///< 单次最大采样点数
    };

    /** @brief 构造信号发生器面板 @param parent 父控件 */
    explicit SignalGeneratorWidget(QWidget *parent = nullptr);

    /** @brief 生成指定参数的波形采样数据 @param type 波形类型 @param freqHz 频率 @param amp 振幅 @param offset 直流偏移 @param samples 采样点数 @return 采样值向量 */
    QVector<double> generateSignal(SignalType type, double freqHz,
                                   double amp, double offset, int samples);

    /** @brief 将采样值按格式转换为字节流 @param signal 采样数据 @param fmt 输出格式 @return 字节数组 */
    QByteArray convertToBytes(const QVector<double> &signal, OutputFormat fmt) const;

    /** @brief 获取统计信息 @return 当前统计快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 信号生成完成 @param samples 生成的采样数据 */
    void signalGenerated(const QVector<double> &samples);

    /** @brief 字节流就绪 @param data 输出的字节数据 */
    void dataReady(const QByteArray &data);

    /** @brief 一次完整输出完成 @param bytes 输出字节数 */
    void outputComplete(qint64 bytes);

private slots:
    void onGenerate();          ///< 生成按钮点击
    void onOutput();            ///< 输出按钮点击
    void onAutoSendTick();      ///< 自动发送定时器滴答

private:
    void setupUI();             ///< 初始化界面布局
    void updatePreview();       ///< 刷新预览标签和电平条

    // ---- UI 控件 ----
    QComboBox *m_typeCombo;     ///< 波形类型选择
    QDoubleSpinBox *m_freqSpin; ///< 频率输入
    QDoubleSpinBox *m_ampSpin;  ///< 振幅输入
    QDoubleSpinBox *m_offsetSpin; ///< 直流偏移输入
    QSpinBox *m_samplesSpin;    ///< 采样点数输入
    QComboBox *m_formatCombo;   ///< 输出格式选择
    QPushButton *m_genButton;   ///< 生成按钮
    QPushButton *m_outputButton;///< 输出按钮
    QPushButton *m_autoButton;  ///< 自动发送开关
    QSpinBox *m_intervalSpin;   ///< 自动发送间隔(ms)
    QLabel *m_previewLabel;     ///< 预览文本标签
    QProgressBar *m_levelBar;   ///< 电平指示条

    // ---- 运行时状态 ----
    QVector<double> m_lastSignal;   ///< 最近一次生成的采样数据
    QByteArray m_lastBytes;         ///< 最近一次转换的字节流
    QTimer *m_autoTimer;            ///< 自动发送定时器
    bool m_autoSending;             ///< 自动发送开关状态

    Stats m_stats;                  ///< 统计计数器
};

#endif // SIGNALGENERATORWIDGET_H
