/**
 * @file SignalLineWidget.cpp
 * @brief 信号线状态显示控件实现 — 可视化展示串口 6 个信号线电平
 */

#include "serial/signals/SignalLineWidget.h"
#include <QGridLayout>
#include <QHBoxLayout>

/**
 * @brief 构造函数
 * @param parent 父控件
 */
SignalLineWidget::SignalLineWidget(QWidget* parent)
    : QWidget(parent)
    , m_rtsLabel(nullptr)
    , m_ctsLabel(nullptr)
    , m_dtrLabel(nullptr)
    , m_dsrLabel(nullptr)
    , m_dcdLabel(nullptr)
    , m_riLabel(nullptr)
    , m_dtrBtn(nullptr)
    , m_rtsBtn(nullptr)
    , m_totalSignalUpdates(0)
    , m_totalDtrToggles(0)
    , m_totalRtsToggles(0)
{
    setObjectName(QStringLiteral("signalLineWidget"));
    setupUI();
}

/**
 * @brief 更新信号线状态显示
 *
 * 根据 newSignals 更新各信号线标签的状态文字（HIGH/LOW）。
 * 同时更新 DTR/RTS 按钮的选中状态（不触发信号）。
 *
 * @param newSignals 最新的信号线状态
 */
void SignalLineWidget::updateSignals(const PinoutSignals& newSignals)
{
    ++m_totalSignalUpdates;

    // 更新输入信号线标签（只读显示）
    m_ctsLabel->setText(newSignals.cts ? tr("HIGH") : tr("LOW"));
    m_dsrLabel->setText(newSignals.dsr ? tr("HIGH") : tr("LOW"));
    m_dcdLabel->setText(newSignals.dcd ? tr("HIGH") : tr("LOW"));
    m_riLabel->setText(newSignals.ri   ? tr("HIGH") : tr("LOW"));

    // 更新输出信号线标签（含切换按钮）
    m_rtsLabel->setText(newSignals.rts ? tr("HIGH") : tr("LOW"));
    m_dtrLabel->setText(newSignals.dtr ? tr("HIGH") : tr("LOW"));

    // 同步按钮状态（阻塞信号避免递归触发）
    m_rtsBtn->blockSignals(true);
    m_rtsBtn->setChecked(newSignals.rts);
    m_rtsBtn->blockSignals(false);

    m_dtrBtn->blockSignals(true);
    m_dtrBtn->setChecked(newSignals.dtr);
    m_dtrBtn->blockSignals(false);
}

/**
 * @brief 设置 DTR 信号线是否可控
 * @param controllable true 可控（显示切换按钮），false 只读
 */
void SignalLineWidget::setDtrControllable(bool controllable)
{
    m_dtrBtn->setVisible(controllable);
}

/**
 * @brief 设置 RTS 信号线是否可控
 * @param controllable true 可控（显示切换按钮），false 只读
 */
void SignalLineWidget::setRtsControllable(bool controllable)
{
    m_rtsBtn->setVisible(controllable);
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 使用网格布局排列 6 个信号线状态标签和 2 个切换按钮。
 * 布局:
 *   Row 0: RTS名称 + RTS状态 | CTS名称 + CTS状态
 *   Row 1: DTR名称 + DTR状态 | DSR名称 + DSR状态
 *   Row 2: DCD名称 + DCD状态 | RI名称  + RI状态
 *   Row 3: DTR切换按钮        | RTS切换按钮
 */
void SignalLineWidget::setupUI()
{
    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(6);

    int row = 0;

    // --- Row 0: RTS + CTS ---
    {
        auto* nameLbl = new QLabel(tr("RTS"), this);
        nameLbl->setObjectName(QStringLiteral("rtsNameLabel"));
        layout->addWidget(nameLbl, row, 0);

        m_rtsLabel = new QLabel(tr("LOW"), this);
        m_rtsLabel->setObjectName(QStringLiteral("rtsStateLabel"));
        layout->addWidget(m_rtsLabel, row, 1);
    }
    {
        auto* nameLbl = new QLabel(tr("CTS"), this);
        nameLbl->setObjectName(QStringLiteral("ctsNameLabel"));
        layout->addWidget(nameLbl, row, 2);

        m_ctsLabel = new QLabel(tr("LOW"), this);
        m_ctsLabel->setObjectName(QStringLiteral("ctsStateLabel"));
        layout->addWidget(m_ctsLabel, row, 3);
    }
    ++row;

    // --- Row 1: DTR + DSR ---
    {
        auto* nameLbl = new QLabel(tr("DTR"), this);
        nameLbl->setObjectName(QStringLiteral("dtrNameLabel"));
        layout->addWidget(nameLbl, row, 0);

        m_dtrLabel = new QLabel(tr("LOW"), this);
        m_dtrLabel->setObjectName(QStringLiteral("dtrStateLabel"));
        layout->addWidget(m_dtrLabel, row, 1);
    }
    {
        auto* nameLbl = new QLabel(tr("DSR"), this);
        nameLbl->setObjectName(QStringLiteral("dsrNameLabel"));
        layout->addWidget(nameLbl, row, 2);

        m_dsrLabel = new QLabel(tr("LOW"), this);
        m_dsrLabel->setObjectName(QStringLiteral("dsrStateLabel"));
        layout->addWidget(m_dsrLabel, row, 3);
    }
    ++row;

    // --- Row 2: DCD + RI ---
    {
        auto* nameLbl = new QLabel(tr("DCD"), this);
        nameLbl->setObjectName(QStringLiteral("dcdNameLabel"));
        layout->addWidget(nameLbl, row, 0);

        m_dcdLabel = new QLabel(tr("LOW"), this);
        m_dcdLabel->setObjectName(QStringLiteral("dcdStateLabel"));
        layout->addWidget(m_dcdLabel, row, 1);
    }
    {
        auto* nameLbl = new QLabel(tr("RI"), this);
        nameLbl->setObjectName(QStringLiteral("riNameLabel"));
        layout->addWidget(nameLbl, row, 2);

        m_riLabel = new QLabel(tr("LOW"), this);
        m_riLabel->setObjectName(QStringLiteral("riStateLabel"));
        layout->addWidget(m_riLabel, row, 3);
    }
    ++row;

    // --- Row 3: DTR + RTS 切换按钮 ---
    m_dtrBtn = new QPushButton(tr("DTR"), this);
    m_dtrBtn->setObjectName(QStringLiteral("dtrToggleBtn"));
    m_dtrBtn->setCheckable(true);
    m_dtrBtn->setVisible(false);  // 默认不可控
    layout->addWidget(m_dtrBtn, row, 0, 1, 2);

    m_rtsBtn = new QPushButton(tr("RTS"), this);
    m_rtsBtn->setObjectName(QStringLiteral("rtsToggleBtn"));
    m_rtsBtn->setCheckable(true);
    m_rtsBtn->setVisible(false);  // 默认不可控
    layout->addWidget(m_rtsBtn, row, 2, 1, 2);

    // 连接按钮信号
    connect(m_dtrBtn, &QPushButton::toggled,
            this, &SignalLineWidget::dtrToggleRequested);
    connect(m_rtsBtn, &QPushButton::toggled,
            this, &SignalLineWidget::rtsToggleRequested);

    // 统计: DTR/RTS 切换计数
    connect(m_dtrBtn, &QPushButton::toggled, this, [this]() {
        ++m_totalDtrToggles;
    });
    connect(m_rtsBtn, &QPushButton::toggled, this, [this]() {
        ++m_totalRtsToggles;
    });
}

// ---- 统计接口 ----

/** @brief 获取累计信号线状态更新次数 @return 更新总次数 */
quint64 SignalLineWidget::totalSignalUpdates() const
{
    return m_totalSignalUpdates;
}

/** @brief 获取累计DTR切换请求次数 @return DTR切换总次数 */
quint64 SignalLineWidget::totalDtrToggles() const
{
    return m_totalDtrToggles;
}

/** @brief 获取累计RTS切换请求次数 @return RTS切换总次数 */
quint64 SignalLineWidget::totalRtsToggles() const
{
    return m_totalRtsToggles;
}

/** @brief 重置所有统计计数器归零 */
void SignalLineWidget::resetSignalWidgetStatistics()
{
    m_totalSignalUpdates = 0;
    m_totalDtrToggles = 0;
    m_totalRtsToggles = 0;
}
