/**
 * @file TimestampPanel.cpp
 * @brief 时间戳工具面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/timestamp/TimestampPanel.h"

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
{
    setObjectName(QStringLiteral("TimestampPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 输入行
    auto *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel(tr("时间戳："), this));
    m_timestampEdit->setPlaceholderText(tr("输入时间戳或日期..."));
    inputLayout->addWidget(m_timestampEdit);

    // 格式选择
    m_formatCombo->addItem(tr("自动检测"), 0);
    m_formatCombo->addItem(tr("Unix 秒"), 1);
    m_formatCombo->addItem(tr("Unix 毫秒"), 2);
    m_formatCombo->addItem(tr("ISO 日期"), 3);

    inputLayout->addWidget(m_formatCombo);
    inputLayout->addWidget(m_convertBtn);
    inputLayout->addWidget(m_nowBtn);

    // 结果
    m_resultLabel->setStyleSheet(QStringLiteral("font-size: 12pt;"));
    m_resultLabel->setWordWrap(true);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_resultLabel);
    mainLayout->addStretch();

    // 连接信号
    connect(m_convertBtn, &QPushButton::clicked,
            this, &TimestampPanel::onConvert);
    connect(m_nowBtn, &QPushButton::clicked,
            this, &TimestampPanel::onNow);
    connect(m_timestampEdit, &QLineEdit::returnPressed,
            this, &TimestampPanel::onConvert);
}

/**
 * @brief 执行时间戳转换
 */
void TimestampPanel::onConvert()
{
    QString text = m_timestampEdit->text().trimmed();
    if (text.isEmpty()) {
        m_resultLabel->setText(tr("结果：无输入"));
        return;
    }

    QDateTime dt = m_analyzer.parseTimestamp(text);
    if (!dt.isValid()) {
        m_resultLabel->setText(tr("结果：无法解析"));
        return;
    }

    qint64 secs = dt.toSecsSinceEpoch();
    qint64 millis = secs * 1000;

    m_resultLabel->setText(
        tr("日期时间：%1\n"
           "Unix 秒：%2\n"
           "Unix 毫秒：%3\n"
           "ISO：%4")
            .arg(dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
            .arg(secs)
            .arg(millis)
            .arg(dt.toString(Qt::ISODate)));
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
