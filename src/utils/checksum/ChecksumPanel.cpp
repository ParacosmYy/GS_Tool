/**
 * @file ChecksumPanel.cpp
 * @brief 校验和计算面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持十六进制/ASCII/文件三种输入模式，提供结果复制功能。
 */

#include "utils/checksum/ChecksumPanel.h"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFile>
#include <QHBoxLayout>
#include <QMimeData>
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
    resultLayout->addWidget(m_resultLabel);
    resultLayout->addStretch();
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

/**
 * @brief 根据输入模式解析输入数据为字节数组
 */
QByteArray ChecksumPanel::inputData() const
{
    int mode = m_inputModeCombo->currentData().toInt();

    switch (mode) {
    case 0: {
        // 十六进制模式
        QString text = m_inputEdit->toPlainText().simplified();
        text.remove(' ');
        return QByteArray::fromHex(text.toUtf8());
    }
    case 1: {
        // ASCII文本模式
        return m_inputEdit->toPlainText().toUtf8();
    }
    case 2: {
        // 二进制文件模式：输入框内容为文件路径
        QString path = m_inputEdit->toPlainText().trimmed();
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            return file.readAll();
        }
        return QByteArray();
    }
    default:
        return QByteArray();
    }
}

/**
 * @brief 获取当前选中算法
 */
ChecksumCalculator::Algorithm ChecksumPanel::selectedAlgorithm() const
{
    return static_cast<ChecksumCalculator::Algorithm>(
        m_algoCombo->currentData().toInt());
}

/**
 * @brief 返回最近一次计算结果
 */
quint64 ChecksumPanel::result() const
{
    return m_result;
}

/**
 * @brief 执行校验和计算
 */
void ChecksumPanel::onCalculate()
{
    QByteArray data = inputData();
    if (data.isEmpty()) {
        m_resultLabel->setText(tr("结果：无数据"));
        m_copyBtn->setEnabled(false);
        return;
    }

    ++m_totalCalculations;
    auto alg = selectedAlgorithm();
    m_result = m_calculator.calculate(data, alg);
    QString name = ChecksumCalculator::algorithmName(alg);

    // 根据结果位宽选择合适的显示格式
    QString hexResult;
    if (m_result <= 0xFF) {
        hexResult = QStringLiteral("%1").arg(m_result, 2, 16, QLatin1Char('0'));
    } else if (m_result <= 0xFFFF) {
        hexResult = QStringLiteral("%1").arg(m_result, 4, 16, QLatin1Char('0'));
    } else {
        hexResult = QStringLiteral("%1").arg(m_result, 8, 16, QLatin1Char('0'));
    }
    hexResult = hexResult.toUpper();

    m_resultLabel->setText(tr("结果：0x%1 (%2) [%3]")
                               .arg(hexResult)
                               .arg(m_result)
                               .arg(name));

    m_copyBtn->setEnabled(true);
    emit calculated(m_result, name);

    /* 添加到历史记录 */
    QString inputHex = data.toHex(' ').toUpper();
    if (inputHex.length() > 32) {
        inputHex = inputHex.left(32) + QStringLiteral("...");
    }
    addHistoryEntry(m_result, name, inputHex);
}

/**
 * @brief 复制校验和结果到剪贴板
 */
void ChecksumPanel::onCopyResult()
{
    ++m_totalCopyActions;
    ++m_totalCopyToClipboard;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(m_result, 16).toUpper());
}

/**
 * @brief 清除计算历史列表
 */
void ChecksumPanel::onClearHistory()
{
    m_historyList->clear();
}

/**
 * @brief 从历史记录中选择并恢复显示结果
 *
 * 点击历史条目时，解析其中存储的结果值并更新显示。
 */
void ChecksumPanel::onHistoryItemSelected()
{
    QListWidgetItem* item = m_historyList->currentItem();
    if (!item) {
        return;
    }
    /* 从 item data 中恢复结果值 */
    bool ok = false;
    quint64 value = item->data(Qt::UserRole).toULongLong(&ok);
    if (ok) {
        m_result = value;
        m_resultLabel->setText(item->text());
        m_copyBtn->setEnabled(true);
    }
}

/**
 * @brief 添加计算结果到历史记录列表
 *
 * @param value 校验和值
 * @param algoName 算法名称
 * @param inputHex 输入数据十六进制（截断显示）
 */
void ChecksumPanel::addHistoryEntry(quint64 value, const QString& algoName,
                                    const QString& inputHex)
{
    /* 根据结果位宽选择格式 */
    QString hexResult;
    if (value <= 0xFF) {
        hexResult = QStringLiteral("%1").arg(value, 2, 16, QLatin1Char('0'));
    } else if (value <= 0xFFFF) {
        hexResult = QStringLiteral("%1").arg(value, 4, 16, QLatin1Char('0'));
    } else {
        hexResult = QStringLiteral("%1").arg(value, 8, 16, QLatin1Char('0'));
    }
    hexResult = hexResult.toUpper();

    QString displayText = tr("[%1] %2 ← 0x%3")
                              .arg(algoName)
                              .arg(hexResult)
                              .arg(inputHex);

    auto* historyItem = new QListWidgetItem(displayText, m_historyList);
    historyItem->setData(Qt::UserRole, value);

    /* 限制历史记录最多50条 */
    while (m_historyList->count() > 50) {
        delete m_historyList->takeItem(0);
    }
}

/**
 * @brief 拖拽进入事件 — 接受文件拖入
 */
void ChecksumPanel::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/**
 * @brief 拖拽移动事件
 */
void ChecksumPanel::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

/**
 * @brief 放下事件 — 将拖入文件路径填入输入框并切换为文件模式
 */
void ChecksumPanel::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    const QString filePath = urls.first().toLocalFile();
    if (filePath.isEmpty()) {
        return;
    }

    /* 切换到文件模式并填入路径 */
    for (int i = 0; i < m_inputModeCombo->count(); ++i) {
        if (m_inputModeCombo->itemData(i).toInt() == 2) {
            m_inputModeCombo->setCurrentIndex(i);
            break;
        }
    }
    m_inputEdit->setPlainText(filePath);
    event->acceptProposedAction();
}

// ---- 统计计数 ----

/**
 * @brief 获取累计计算次数（面板层面）
 */
quint64 ChecksumPanel::totalCalculations() const
{
    return m_totalCalculations;
}

/**
 * @brief 获取累计复制到剪贴板次数
 */
quint64 ChecksumPanel::totalCopyActions() const
{
    return m_totalCopyActions;
}

/**
 * @brief 获取累计算法切换次数
 */
quint64 ChecksumPanel::totalAlgorithmChanges() const
{
    return m_totalAlgorithmChanges;
}

/**
 * @brief 获取累计复制结果到剪贴板操作次数
 */
quint64 ChecksumPanel::totalCopyToClipboard() const
{
    return m_totalCopyToClipboard;
}

/**
 * @brief 获取累计输入内容更新次数
 */
quint64 ChecksumPanel::totalInputUpdates() const
{
    return m_totalInputUpdates;
}

/**
 * @brief 重置所有面板统计计数器(计算次数/复制次数/算法切换/剪贴板复制/输入更新)
 */
void ChecksumPanel::resetPanelStatistics()
{
    m_totalCalculations = 0;
    m_totalCopyActions = 0;
    m_totalAlgorithmChanges = 0;
    m_totalCopyToClipboard = 0;
    m_totalInputUpdates = 0;
}
