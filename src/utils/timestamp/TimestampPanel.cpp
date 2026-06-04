/**
 * @file TimestampPanel.cpp
 * @brief 时间戳工具面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持自动检测和指定格式的双向转换，结果可复制。
 *
 * 转换逻辑与操作槽函数见：@see TimestampPanelConvert.cpp
 */

#include "utils/timestamp/TimestampPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

/**
 * @brief 构造函数，初始化时间戳面板布局
 */
TimestampPanel::TimestampPanel(QWidget *parent)
    : QWidget(parent)
    , m_timestampEdit(new QLineEdit(this))
    , m_formatCombo(new QComboBox(this))
    , m_resultLabel(new QLabel(tr("结果：-"), this))
    , m_convertBtn(new QPushButton(tr("转换"), this))
    , m_nowBtn(new QPushButton(tr("当前时间"), this))
    , m_copyBtn(new QPushButton(tr("复制"), this))
    , m_clearHistoryBtn(new QPushButton(tr("清除历史"), this))
    , m_historyList(new QListWidget(this))
{
    setObjectName(QStringLiteral("TimestampPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 输入行
    auto *inputLayout = new QHBoxLayout();
    auto* tsLabel = new QLabel(tr("时间戳："), this);
    tsLabel->setObjectName("timestampLabel");
    inputLayout->addWidget(tsLabel);
    m_timestampEdit->setObjectName("timestampInput");
    m_timestampEdit->setPlaceholderText(tr("输入时间戳或日期..."));
    inputLayout->addWidget(m_timestampEdit);

    // 格式选择
    m_formatCombo->setObjectName("timestampFormatCombo");
    m_formatCombo->addItem(tr("自动检测"), 0);
    m_formatCombo->addItem(tr("Unix 秒"), 1);
    m_formatCombo->addItem(tr("Unix 毫秒"), 2);
    m_formatCombo->addItem(tr("ISO 日期"), 3);

    inputLayout->addWidget(m_formatCombo);
    m_convertBtn->setObjectName("timestampConvertBtn");
    m_nowBtn->setObjectName("timestampNowBtn");
    inputLayout->addWidget(m_convertBtn);
    inputLayout->addWidget(m_nowBtn);

    // 结果 + 复制按钮
    auto *resultLayout = new QHBoxLayout();
    /* 结果样式由QSS主题控制，使用objectName匹配 */
    m_resultLabel->setObjectName("timestampResultLabel");
    m_resultLabel->setProperty("class", "resultLabel");
    m_resultLabel->setWordWrap(true);
    resultLayout->addWidget(m_resultLabel);
    resultLayout->addStretch();

    m_copyBtn->setObjectName("timestampCopyResultBtn");
    m_copyBtn->setEnabled(false);
    resultLayout->addWidget(m_copyBtn);

    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(resultLayout);

    // 历史记录区
    auto* historyHeader = new QHBoxLayout();
    auto* historyLabel = new QLabel(tr("转换历史："), this);
    historyLabel->setObjectName("timestampHistoryLabel");
    m_clearHistoryBtn->setObjectName("clearTimestampHistoryBtn");
    m_historyList->setObjectName("timestampHistoryList");
    m_historyList->setMaximumHeight(100);
    m_historyList->setSelectionMode(QAbstractItemView::SingleSelection);
    historyHeader->addWidget(historyLabel);
    historyHeader->addStretch();
    historyHeader->addWidget(m_clearHistoryBtn);

    mainLayout->addLayout(historyHeader);
    mainLayout->addWidget(m_historyList, 1);
    mainLayout->addStretch();

    // 连接信号
    connect(m_convertBtn, &QPushButton::clicked,
            this, &TimestampPanel::onConvert);
    connect(m_nowBtn, &QPushButton::clicked,
            this, &TimestampPanel::onNow);
    connect(m_timestampEdit, &QLineEdit::returnPressed,
            this, &TimestampPanel::onConvert);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &TimestampPanel::onCopy);
    connect(m_clearHistoryBtn, &QPushButton::clicked,
            this, &TimestampPanel::onClearHistory);
    connect(m_historyList, &QListWidget::itemClicked,
            this, &TimestampPanel::onHistorySelected);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { ++m_totalFormatChanges; ++m_totalFormatsSelected; });
}

// onConvert/convertByFormat/onNow/onCopy/onClearHistory/onHistorySelected/resetTimestampPanelStatistics
// 已移至 TimestampPanelConvert.cpp
