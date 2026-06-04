/**
 * @file ConverterPanel.cpp
 * @brief 数据格式转换面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持格式选择、交换、转换和复制输出。
 */

#include "utils/converter/ConverterPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief 构造函数，初始化转换面板布局
 */
ConverterPanel::ConverterPanel(QWidget *parent)
    : QWidget(parent)
    , m_inputEdit(new QTextEdit(this))
    , m_fromCombo(new QComboBox(this))
    , m_toCombo(new QComboBox(this))
    , m_convertBtn(new QPushButton(tr("\xe2\x86\x92"), this)) // →
    , m_swapBtn(new QPushButton(tr("\xe2\x87\x84"), this))    // ⇄
    , m_copyBtn(new QPushButton(tr("复制结果"), this))
    , m_outputEdit(new QTextEdit(this))
    , m_clearHistoryBtn(new QPushButton(tr("清除历史"), this))
    , m_historyList(new QListWidget(this))
{
    setObjectName(QStringLiteral("ConverterPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 格式选择行
    auto *formatLayout = new QHBoxLayout();
    auto* srcFormatLabel = new QLabel(tr("源格式："), this);
    srcFormatLabel->setObjectName("converterSrcFormatLabel");
    formatLayout->addWidget(srcFormatLabel);

    const auto formats = {
        DataConverter::Hex, DataConverter::Ascii, DataConverter::Base64,
        DataConverter::UrlEncode, DataConverter::Binary,
        DataConverter::Decimal, DataConverter::Octal
    };
    for (auto fmt : formats) {
        QString name = DataConverter::formatName(fmt);
        m_fromCombo->addItem(name, static_cast<int>(fmt));
        m_toCombo->addItem(name, static_cast<int>(fmt));
    }

    m_fromCombo->setObjectName("converterFromCombo");
    m_toCombo->setObjectName("converterToCombo");
    m_inputEdit->setObjectName("converterInputEdit");
    m_outputEdit->setObjectName("converterOutputEdit");

    formatLayout->addWidget(m_fromCombo);

    // 交换按钮
    m_swapBtn->setObjectName("swapBtn");
    m_swapBtn->setToolTip(tr("交换源/目标格式"));
    m_swapBtn->setFixedWidth(40);
    formatLayout->addWidget(m_swapBtn);

    auto* dstFormatLabel = new QLabel(tr("目标格式："), this);
    dstFormatLabel->setObjectName("converterDstFormatLabel");
    formatLayout->addWidget(dstFormatLabel);
    formatLayout->addWidget(m_toCombo);

    // 转换按钮（箭头图标）
    m_convertBtn->setObjectName("convertBtn");
    m_convertBtn->setFixedWidth(60);
    formatLayout->addWidget(m_convertBtn);

    // 默认选择：Hex → ASCII
    m_fromCombo->setCurrentIndex(0);
    m_toCombo->setCurrentIndex(1);

    // 输入区
    m_inputEdit->setPlaceholderText(tr("输入待转换数据..."));

    // 输出区（只读）
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setPlaceholderText(tr("转换结果将显示在此..."));

    // 复制按钮行
    auto *outputHeader = new QHBoxLayout();
    auto* outputLabel = new QLabel(tr("输出："), this);
    outputLabel->setObjectName("converterOutputLabel");
    outputHeader->addWidget(outputLabel);
    outputHeader->addStretch();
    m_copyBtn->setObjectName("copyOutputBtn");
    m_copyBtn->setEnabled(false);
    outputHeader->addWidget(m_copyBtn);

    mainLayout->addLayout(formatLayout);
    mainLayout->addWidget(m_inputEdit);
    mainLayout->addLayout(outputHeader);
    mainLayout->addWidget(m_outputEdit);

    // 历史记录区
    auto* historyHeader = new QHBoxLayout();
    auto* historyLabel = new QLabel(tr("转换历史："), this);
    historyLabel->setObjectName("converterHistoryLabel");
    m_clearHistoryBtn->setObjectName("clearConverterHistoryBtn");
    m_historyList->setObjectName("converterHistoryList");
    m_historyList->setMaximumHeight(80);
    m_historyList->setSelectionMode(QAbstractItemView::SingleSelection);
    historyHeader->addWidget(historyLabel);
    historyHeader->addStretch();
    historyHeader->addWidget(m_clearHistoryBtn);
    mainLayout->addLayout(historyHeader);
    mainLayout->addWidget(m_historyList, 1);

    // 连接信号
    connect(m_convertBtn, &QPushButton::clicked,
            this, &ConverterPanel::onConvert);
    connect(m_swapBtn, &QPushButton::clicked,
            this, &ConverterPanel::onSwap);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &ConverterPanel::onCopy);
    connect(m_clearHistoryBtn, &QPushButton::clicked,
            this, &ConverterPanel::onClearHistory);
    connect(m_historyList, &QListWidget::itemClicked,
            this, &ConverterPanel::onHistorySelected);
}

/**
 * @brief 设置输入内容
 */
void ConverterPanel::setInput(const QString &text)
{
    ++m_totalInputChanges;
    m_inputEdit->setPlainText(text);
}

/**
 * @brief 获取输出内容
 */
QString ConverterPanel::output() const
{
    return m_outputEdit->toPlainText();
}

// onConvert/onSwap/onCopy/onClearHistory/onHistorySelected/resetStatistics
// 已移至 ConverterPanelSlots.cpp
