/**
 * @file SignalLineWidget.h
 * @brief 信号线状态显示控件 — 可视化展示串口 6 个信号线电平
 *
 * 显示 RTS/CTS/DTR/DSR/DCD/RI 六个信号线的当前电平状态（HIGH/LOW），
 * DTR 和 RTS 信号线支持点击切换（可配置是否可控）。
 *
 * 协作关系:
 *   - SignalLineMonitor: 提供信号线状态数据
 *   - IConnection: 执行 DTR/RTS 切换操作
 */
#ifndef SIGNALLINEWIDGET_H
#define SIGNALLINEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "connection/interface/IConnection.h"

/**
 * @brief 信号线状态显示控件
 *
 * 6 个 QLabel 显示信号线电平状态，DTR 和 RTS 各有一个切换按钮。
 * 通过 QSS 根据信号状态改变指示灯颜色。
 */
class SignalLineWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit SignalLineWidget(QWidget* parent = nullptr);

    /**
     * @brief 更新信号线状态显示
     * @param signals 最新的信号线状态
     */
    void updateSignals(const PinoutSignals& newSignals);

    /**
     * @brief 设置 DTR 信号线是否可控
     * @param controllable true 可控（显示切换按钮），false 只读
     */
    void setDtrControllable(bool controllable);

    /**
     * @brief 设置 RTS 信号线是否可控
     * @param controllable true 可控（显示切换按钮），false 只读
     */
    void setRtsControllable(bool controllable);

signals:
    /**
     * @brief 用户请求切换 DTR 信号线
     * @param enabled true=拉高 DTR，false=拉低 DTR
     */
    void dtrToggleRequested(bool enabled);

    /**
     * @brief 用户请求切换 RTS 信号线
     * @param enabled true=拉高 RTS，false=拉低 RTS
     */
    void rtsToggleRequested(bool enabled);

private:
    /** @brief 初始化 UI 布局和控件 */
    void setupUI();

    QLabel* m_rtsLabel;     ///< RTS 信号线状态标签
    QLabel* m_ctsLabel;     ///< CTS 信号线状态标签
    QLabel* m_dtrLabel;     ///< DTR 信号线状态标签
    QLabel* m_dsrLabel;     ///< DSR 信号线状态标签
    QLabel* m_dcdLabel;     ///< DCD 信号线状态标签
    QLabel* m_riLabel;      ///< RI 信号线状态标签
    QPushButton* m_dtrBtn;  ///< DTR 切换按钮
    QPushButton* m_rtsBtn;  ///< RTS 切换按钮
};

#endif // SIGNALLINEWIDGET_H
