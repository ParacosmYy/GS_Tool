/**
 * @file WaveformGeneratorWidget.h
 * @brief 波形发生器面板 -- 左侧参数控制 + 右侧 QPainter 实时预览
 * @author EmbedDebug Team
 * @date 2026-06-06
 *
 * 提供波形类型选择、频率/振幅/偏移/相位/占空比控制,
 * 以及开始/停止流式输出的交互按钮。
 * 右侧画布以 QPainter 绘制 2 个周期的波形预览。
 */

#ifndef WAVEGEN_WAVEFORMGENERATORWIDGET_H
#define WAVEGEN_WAVEFORMGENERATORWIDGET_H

#include <QByteArray>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

#include "utils/wavegen/WaveGenTypes.h"

class WaveformGenerator;

/**
 * @class WaveformGeneratorWidget
 * @brief 波形发生器控制面板 + QPainter 波形预览
 *
 * 布局: 左侧参数区 | 右侧预览画布 + 启停按钮
 * 所有 QSpinBox/QSlider 联动更新预览和 Generator 参数。
 */
class WaveformGeneratorWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造波形发生器面板 @param parent 父控件 */
    explicit WaveformGeneratorWidget(QWidget *parent = nullptr);

    /** @brief 获取内部发生器实例 @return 指针,所有权属于本控件 */
    WaveformGenerator *generator() const;

    /** @brief 获取统计快照 @return 统计结构体 */
    WaveGen::Stats stats() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 流式数据就绪(转发自 WaveformGenerator) @param data 字节流 */
    void dataGenerated(const QByteArray &data);

protected:
    /** @brief 绘制波形预览 @param event 绘图事件 */
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTypeChanged(int index);     ///< 波形类型变更
    void onFrequencyChanged(double v); ///< 频率变更
    void onAmplitudeChanged(int v);    ///< 振幅滑块变更
    void onOffsetChanged(int v);       ///< 偏移滑块变更
    void onPhaseChanged(int v);        ///< 相位滑块变更
    void onDutyCycleChanged(int v);    ///< 占空比变更
    void onStartClicked();             ///< 启动流式输出
    void onStopClicked();              ///< 停止流式输出

private:
    void setupUI();                    ///< 构建界面布局
    void updateParamsFromControls();   ///< 从控件同步参数到 Generator
    void drawWaveform(QPainter &p);    ///< QPainter 绘制波形曲线

    // ---- 控件 ----
    QComboBox *m_typeCombo;            ///< 波形类型选择
    QDoubleSpinBox *m_freqSpin;        ///< 频率输入
    QLabel *m_ampLabel;                ///< 振幅数值显示
    QSlider *m_ampSlider;              ///< 振幅滑块(0~200, /100)
    QLabel *m_offLabel;                ///< 偏移数值显示
    QSlider *m_offSlider;              ///< 偏移滑块(-100~100, /100)
    QLabel *m_phaseLabel;              ///< 相位数值显示
    QSlider *m_phaseSlider;            ///< 相位滑块(0~360 度)
    QLabel *m_dutyLabel;               ///< 占空比数值显示
    QSlider *m_dutySlider;             ///< 占空比滑块(1~99, %)
    QPushButton *m_startBtn;           ///< 开始流式输出
    QPushButton *m_stopBtn;            ///< 停止流式输出

    // ---- 内部状态 ----
    WaveformGenerator *m_generator;    ///< 波形发生器实例
};

#endif // WAVEGEN_WAVEFORMGENERATORWIDGET_H
