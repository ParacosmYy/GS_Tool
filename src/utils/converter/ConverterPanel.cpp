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
    formatLayout->addWidget(new QLabel(tr("源格式："), this));

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

    formatLayout->addWidget(m_fromCombo);

    // 交换按钮
    m_swapBtn->setObjectName("swapBtn");
    m_swapBtn->setToolTip(tr("交换源/目标格式"));
    m_swapBtn->setFixedWidth(40);
    formatLayout->addWidget(m_swapBtn);

    formatLayout->addWidget(new QLabel(tr("目标格式："), this));
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
    outputHeader->addWidget(new QLabel(tr("输出："), this));
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
    m_inputEdit->setPlainText(text);
}

/**
 * @brief 获取输出内容
 */
QString ConverterPanel::output() const
{
    return m_outputEdit->toPlainText();
}

/**
 * @brief 执行格式转换
 */
void ConverterPanel::onConvert()
{
    QByteArray input = m_inputEdit->toPlainText().toUtf8();
    if (input.isEmpty()) {
        m_outputEdit->clear();
        m_copyBtn->setEnabled(false);
        return;
    }

    auto from = static_cast<DataConverter::Format>(
        m_fromCombo->currentData().toInt());
    auto to = static_cast<DataConverter::Format>(
        m_toCombo->currentData().toInt());

    QByteArray result = m_converter.convert(input, from, to);
    m_outputEdit->setPlainText(QString::fromUtf8(result));
    m_copyBtn->setEnabled(true);

    /* 添加到历史记录 */
    QString fromName = DataConverter::formatName(from);
    QString toName = DataConverter::formatName(to);
    QString preview = m_inputEdit->toPlainText();
    if (preview.length() > 30) {
        preview = preview.left(30) + QStringLiteral("...");
    }
    auto* histItem = new QListWidgetItem(
        tr("%1 → %2: %3").arg(fromName, toName, preview), m_historyList);
    while (m_historyList->count() > 30) {
        delete m_historyList->takeItem(0);
    }
}

/**
 * @brief 交换源/目标格式并重新转换
 */
void ConverterPanel::onSwap()
{
    int fromIdx = m_fromCombo->currentIndex();
    int toIdx = m_toCombo->currentIndex();

    m_fromCombo->setCurrentIndex(toIdx);
    m_toCombo->setCurrentIndex(fromIdx);

    // 如果有输出内容，用它作为新的输入
    if (!m_outputEdit->toPlainText().isEmpty()) {
        m_inputEdit->setPlainText(m_outputEdit->toPlainText());
        onConvert();
    }
}

/**
 * @brief 复制输出到剪贴板
 */
void ConverterPanel::onCopy()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_outputEdit->toPlainText());
}

/**
 * @brief 清除转换历史列表
 */
void ConverterPanel::onClearHistory()
{
    m_historyList->clear();
}

/**
 * @brief 从历史记录选择恢复输入
 */
void ConverterPanel::onHistorySelected()
{
    QListWidgetItem* item = m_historyList->currentItem();
    if (!item) {
        return;
    }
    /* 历史只记录了摘要，不做恢复操作 */
}
