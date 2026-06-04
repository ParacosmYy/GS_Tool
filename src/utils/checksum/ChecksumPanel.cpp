/**
 * @file ChecksumPanel.cpp
 * @brief 校验和计算面板 UI 实现 — 构造函数
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持十六进制/ASCII/文件三种输入模式，提供结果复制功能。
 *
 * UI 槽函数与拖放事件见：@see ChecksumPanelSlots.cpp
 * 数据解析、算法查询、历史格式化与统计接口见：@see ChecksumPanelAlgo.cpp
 */

#include "utils/checksum/ChecksumPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

/**
 * @brief 构造函数，初始化面板布局
 */
ChecksumPanel::ChecksumPanel(QWidget *parent)
    : QWidget(parent)
    , m_inputEdit(new QTextEdit(this))
    , m_algoCombo(new QComboBox(this))
    , m_inputModeCombo(new QComboBox(this))
    , m_resultLabel(new QLabel(tr("结果：-"), this))
    , m_calcBtn(new QPushButton(tr("计算"), this))
    , m_copyBtn(new QPushButton(tr("复制结果"), this))
    , m_clearHistoryBtn(new QPushButton(tr("清除历史"), this))
    , m_historyList(new QListWidget(this))
{
    setObjectName(QStringLiteral("ChecksumPanel"));
    setAcceptDrops(true);

    auto *mainLayout = new QVBoxLayout(this);

    // 算法选择行
    auto *algoLayout = new QHBoxLayout();
    auto* algoLabel = new QLabel(tr("算法："), this);
    algoLabel->setObjectName("checksumAlgoLabel");
    algoLayout->addWidget(algoLabel);

    const auto algorithms = {
        ChecksumCalculator::CRC8, ChecksumCalculator::CRC16Ccitt,
        ChecksumCalculator::CRC16Modbus, ChecksumCalculator::CRC16Kermit,
        ChecksumCalculator::CRC32, ChecksumCalculator::CRC32C,
        ChecksumCalculator::Xor8, ChecksumCalculator::Sum8,
        ChecksumCalculator::Sum16, ChecksumCalculator::Sum32,
        ChecksumCalculator::CustomCrc
    };
    for (auto alg : algorithms) {
        m_algoCombo->addItem(ChecksumCalculator::algorithmName(alg),
                             static_cast<int>(alg));
    }
    m_algoCombo->setObjectName("checksumAlgoCombo");
    connect(m_algoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { ++m_totalAlgorithmChanges; });
    algoLayout->addWidget(m_algoCombo);
    algoLayout->addStretch();
    m_calcBtn->setObjectName("checksumCalcBtn");
    algoLayout->addWidget(m_calcBtn);

    // 输入模式行
    auto *modeLayout = new QHBoxLayout();
    auto* modeLabel = new QLabel(tr("输入模式："), this);
    modeLabel->setObjectName("checksumModeLabel");
    modeLayout->addWidget(modeLabel);

    m_inputModeCombo->setObjectName("inputModeCombo");
    m_inputModeCombo->addItem(tr("十六进制"), 0);
    m_inputModeCombo->addItem(tr("ASCII文本"), 1);
    m_inputModeCombo->addItem(tr("二进制文件"), 2);
    modeLayout->addWidget(m_inputModeCombo);
    modeLayout->addStretch();

    // 统计: 输入模式切换计数
    connect(m_inputModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { ++m_totalFormatChanges; });

    // 输入区
    m_inputEdit->setObjectName("checksumInputEdit");
    m_inputEdit->setPlaceholderText(
        tr("输入十六进制数据（如：01 02 FF）..."));
    m_inputEdit->setMaximumHeight(100);

    // 结果行
    auto *resultLayout = new QHBoxLayout();
    /* 结果样式由QSS主题控制，使用objectName匹配 */
    m_resultLabel->setObjectName("checksumResultLabel");
    m_resultLabel->setProperty("class", "resultLabel");
    resultLayout->addWidget(m_resultLabel);
    resultLayout->addStretch();
    m_copyBtn->setObjectName("copyResultBtn");
    m_copyBtn->setEnabled(false);
    resultLayout->addWidget(m_copyBtn);

    // 历史记录区
    auto* historyLabel = new QLabel(tr("计算历史："), this);
    historyLabel->setObjectName("checksumHistoryLabel");

    m_historyList->setObjectName("checksumHistoryList");
    m_historyList->setMaximumHeight(120);
    m_historyList->setSelectionMode(QAbstractItemView::SingleSelection);

    m_clearHistoryBtn->setObjectName("clearHistoryBtn");

    auto* historyHeaderLayout = new QHBoxLayout();
    historyHeaderLayout->addWidget(historyLabel);
    historyHeaderLayout->addStretch();
    historyHeaderLayout->addWidget(m_clearHistoryBtn);

    mainLayout->addLayout(algoLayout);
    mainLayout->addLayout(modeLayout);
    mainLayout->addWidget(m_inputEdit);
    mainLayout->addLayout(resultLayout);
    mainLayout->addLayout(historyHeaderLayout);
    mainLayout->addWidget(m_historyList, 1);

    // 连接信号
    connect(m_calcBtn, &QPushButton::clicked,
            this, &ChecksumPanel::onCalculate);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &ChecksumPanel::onCopyResult);
    connect(m_clearHistoryBtn, &QPushButton::clicked,
            this, &ChecksumPanel::onClearHistory);
    connect(m_historyList, &QListWidget::itemClicked,
            this, &ChecksumPanel::onHistoryItemSelected);
    connect(m_inputEdit, &QTextEdit::textChanged,
            this, [this]() { ++m_totalInputUpdates; });
}

// onCalculate/onCopyResult/onClearHistory/onHistoryItemSelected/dragEnterEvent/dragMoveEvent/dropEvent
// 已移至 ChecksumPanelSlots.cpp
