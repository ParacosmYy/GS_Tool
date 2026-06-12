#include "apps/serial_station/ui/SerialLogPanel.h"

#include <QtCore/QTime>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace serial_station {

SerialLogPanel::SerialLogPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("serialLogPanel"));
    setupUi();
    connectSignals();
    appendSystem(tr("日志面板就绪"));
}

void SerialLogPanel::appendTx(const QString& text)
{
    appendLine(QStringLiteral("TX"), text);
}

void SerialLogPanel::appendRx(const QString& text)
{
    appendLine(QStringLiteral("RX"), text);
}

void SerialLogPanel::appendSystem(const QString& text)
{
    appendLine(QStringLiteral("SYS"), text);
}

QString SerialLogPanel::plainText() const
{
    return m_logView->toPlainText();
}

void SerialLogPanel::clearLog()
{
    m_lines.clear();
    m_logView->clear();
    m_countLabel->setText(tr("0 条"));
    emit cleared();
}

void SerialLogPanel::updateFilter(const QString& text)
{
    const QString needle = text.trimmed();
    if (needle.isEmpty()) {
        m_logView->setPlainText(m_lines.join(QLatin1Char('\n')));
        return;
    }

    QStringList filtered;
    for (const QString& line : m_lines) {
        if (line.contains(needle, Qt::CaseInsensitive)) {
            filtered.append(line);
        }
    }
    m_logView->setPlainText(filtered.join(QLatin1Char('\n')));
}

void SerialLogPanel::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto* header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(8);

    auto* title = new QLabel(tr("收发日志"), this);
    title->setObjectName(QStringLiteral("serialLogTitle"));

    m_countLabel = new QLabel(tr("0 条"), this);
    m_countLabel->setObjectName(QStringLiteral("serialLogCountLabel"));

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setObjectName(QStringLiteral("serialLogFilterEdit"));
    m_filterEdit->setPlaceholderText(tr("过滤日志"));
    m_filterEdit->setClearButtonEnabled(true);

    m_clearButton = new QPushButton(tr("清空"), this);
    m_clearButton->setObjectName(QStringLiteral("serialLogClearButton"));

    m_exportButton = new QPushButton(tr("导出"), this);
    m_exportButton->setObjectName(QStringLiteral("serialLogExportButton"));

    m_replayButton = new QPushButton(tr("回放"), this);
    m_replayButton->setObjectName(QStringLiteral("serialLogReplayButton"));

    header->addWidget(title);
    header->addWidget(m_countLabel);
    header->addStretch();
    header->addWidget(m_filterEdit);
    header->addWidget(m_clearButton);
    header->addWidget(m_exportButton);
    header->addWidget(m_replayButton);

    m_logView = new QPlainTextEdit(this);
    m_logView->setObjectName(QStringLiteral("serialLogView"));
    m_logView->setReadOnly(true);
    m_logView->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_logView->setPlaceholderText(tr("暂无收发记录"));

    root->addLayout(header);
    root->addWidget(m_logView, 1);
}

void SerialLogPanel::connectSignals()
{
    connect(m_clearButton, &QPushButton::clicked,
            this, &SerialLogPanel::clearLog);
    connect(m_exportButton, &QPushButton::clicked,
            this, &SerialLogPanel::exportRequested);
    connect(m_replayButton, &QPushButton::clicked,
            this, &SerialLogPanel::replayRequested);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &SerialLogPanel::updateFilter);
}

void SerialLogPanel::appendLine(const QString& channel, const QString& text)
{
    const QString content = text.trimmed();
    if (content.isEmpty()) {
        return;
    }

    const QString line = QStringLiteral("[%1] %2 %3")
                             .arg(timestampText(), channel.leftJustified(3), content);
    m_lines.append(line);
    updateFilter(m_filterEdit->text());
    m_countLabel->setText(tr("%1 条").arg(m_lines.count()));
}

QString SerialLogPanel::timestampText() const
{
    return QTime::currentTime().toString(QStringLiteral("HH:mm:ss.zzz"));
}

} // namespace serial_station
