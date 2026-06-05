/**
 * @file PidTunerWidget.h
 * @brief PID调试器面板 — 交互式参数调整与实时响应曲线
 *
 * 功能:
 *   - 左侧参数面板: Kp/Ki/Kd 旋钮框 + 被控对象参数
 *   - 右侧响应曲线: QPainter 自绘 setpoint / output / control 三条曲线
 *   - 激励类型选择 (阶跃/斜坡/正弦/方波/脉冲)
 *   - 启动 / 停止 / 单步 按钮
 *   - 实时显示性能指标 (超调量/调节时间/上升时间/稳态误差)
 *   - 统计: quint64 计数器追踪仿真/重绘次数
 *
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#ifndef PIDTUNERWIDGET_H
#define PIDTUNERWIDGET_H

#include <QWidget>
#include <QVector>
#include "utils/pid/PidTunerTypes.h"

class QDoubleSpinBox;
class QComboBox;
class QPushButton;
class QLabel;
class PidSimulator;

/**
 * @class PidTunerWidget
 * @brief PID参数交互式调试面板，包含参数编辑器与实时响应曲线
 */
class PidTunerWidget : public QWidget {
    Q_OBJECT
public:
    /** 运行统计 */
    struct Stats {
        quint64 simulationsTriggered = 0; ///< 用户触发的仿真次数
        quint64 repaintCount         = 0; ///< 曲线重绘次数
    };

    explicit PidTunerWidget(QWidget* parent = nullptr);

    /** 获取当前统计 */
    const Stats& stats() const { return m_stats; }

    /** 重置统计计数器 */
    void resetStatistics();

signals:
    /** 任一PID参数变化时发射 */
    void parametersChanged();

protected:
    /** 自绘响应曲线 */
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onStepClicked();
    void onParamChanged();

private:
    /** 创建左侧参数面板 */
    QWidget* createParamPanel();

    /** 创建右侧响应曲线区域 */
    QWidget* createChartArea();

    /** 从UI读取当前参数并触发仿真 */
    void runSimulation();

    /** 更新性能指标标签 */
    void updateMetricsLabels(const PidResponse& resp);

    /* ---- UI 组件 ---- */
    QDoubleSpinBox* m_spinKp;
    QDoubleSpinBox* m_spinKi;
    QDoubleSpinBox* m_spinKd;
    QDoubleSpinBox* m_spinSetpoint;
    QDoubleSpinBox* m_spinGain;
    QDoubleSpinBox* m_spinTau;
    QDoubleSpinBox* m_spinDelay;
    QDoubleSpinBox* m_spinNoise;
    QComboBox*      m_comboType;
    QPushButton*    m_btnStart;
    QPushButton*    m_btnStop;
    QPushButton*    m_btnStep;
    QLabel*         m_labelOvershoot;
    QLabel*         m_labelSettling;
    QLabel*         m_labelRise;
    QLabel*         m_labelSSE;

    /* ---- 数据 ---- */
    PidSimulator*   m_simulator;
    PidResponse     m_lastResponse;
    bool            m_running;

    /* ---- 统计 ---- */
    Stats m_stats;
};

#endif // PIDTUNERWIDGET_H
