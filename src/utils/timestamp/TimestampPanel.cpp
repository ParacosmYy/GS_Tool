/**
 * @file TimestampPanel.cpp
 * @brief 时间戳工具面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持自动检测和指定格式的双向转换，结果可复制。
 */

#include "utils/timestamp/TimestampPanel.h"

#include <QApplication>
#include <QClipboard>
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
            this, [this]() { ++m_totalFormatChanges; });
}

/**
 * @brief 执行时间戳转换（入口）
 */
void TimestampPanel::onConvert()
{
    ++m_totalConversions;
    ++m_totalAnalyses;
    QString text = m_timestampEdit->text().trimmed();
    if (text.isEmpty()) {
        m_resultLabel->setText(tr("结果：无输入"));
        m_copyBtn->setEnabled(false);
        return;
    }

    int fmt = m_formatCombo->currentData().toInt();
    convertByFormat(text, fmt);
}

/**
 * @brief 根据格式执行转换
 */
void TimestampPanel::convertByFormat(const QString &text, int formatIndex)
{
    QDateTime dt;
    bool inputIsTimestamp = false;
    qint64 secs = 0;
    qint64 millis = 0;

    switch (formatIndex) {
    case 1: {
        // Unix 秒
        bool ok = false;
        secs = text.toLongLong(&ok);
        if (ok) {
            dt = m_analyzer.unixToDatetime(secs, false);
            inputIsTimestamp = true;
        }
        break;
    }
    case 2: {
        // Unix 毫秒
        bool ok = false;
        millis = text.toLongLong(&ok);
        if (ok) {
            dt = m_analyzer.unixToDatetime(millis, true);
            secs = millis / 1000;
            inputIsTimestamp = true;
        }
        break;
    }
    case 3: {
        // ISO 日期
        dt = QDateTime::fromString(text, Qt::ISODate);
        if (!dt.isValid()) {
            dt = QDateTime::fromString(text,
                QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        }
        break;
    }
    default: {
        // 自动检测
        dt = m_analyzer.parseTimestamp(text);
        break;
    }
    }

    if (!dt.isValid()) {
        m_resultLabel->setText(tr("结果：无法解析"));
        m_copyBtn->setEnabled(false);
        return;
    }

    ++m_totalTimestampParses;

    if (!inputIsTimestamp) {
        secs = m_analyzer.datetimeToUnix(dt, false);
    }
    millis = secs * 1000;

    QString result = tr(
        "日期时间：%1\n"
        "ISO 格式：%2\n"
        "Unix 秒：%3\n"
        "Unix 毫秒：%4")
        .arg(dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
        .arg(dt.toString(Qt::ISODate))
        .arg(secs)
        .arg(millis);

    m_resultLabel->setText(result);
    m_copyBtn->setEnabled(true);

    /* 添加到历史记录 */
    auto* histItem = new QListWidgetItem(
        tr("%1 → %2").arg(text, dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))),
        m_historyList);
    histItem->setData(Qt::UserRole, text);
    while (m_historyList->count() > 30) {
        delete m_historyList->takeItem(0);
    }
}

/**
 * @brief 填入当前时间戳
 */
void TimestampPanel::onNow()
{
    qint64 now = TimestampAnalyzer::currentUnix(false);
    m_timestampEdit->setText(QString::number(now));
    onConvert();
}

/**
 * @brief 复制结果到剪贴板
 */
void TimestampPanel::onCopy()
{
    ++m_totalCopyActions;
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_resultLabel->text());
}

/**
 * @brief 清除转换历史列表
 */
void TimestampPanel::onClearHistory()
{
    m_historyList->clear();
}

/**
 * @brief 从历史记录选择恢复输入
 */
void TimestampPanel::onHistorySelected()
{
    QListWidgetItem* item = m_historyList->currentItem();
    if (!item) {
        return;
    }
    const QString text = item->data(Qt::UserRole).toString();
    m_timestampEdit->setText(text);
    onConvert();
}

/**
 * @brief 重置时间戳面板统计计数器
 */
void TimestampPanel::resetTimestampPanelStatistics()
{
    m_totalConversions = 0;
    m_totalCopyActions = 0;
    m_totalAnalyses = 0;
    m_totalFormatChanges = 0;
    m_totalTimestampParses = 0;
}
