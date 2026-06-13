#include "apps/serial_station/ui/SerialMeasurementPanel.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

namespace serial_station {

SerialMeasurementPanel::SerialMeasurementPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("serialMeasurementPanel"));
    setupUi();
    showEmptyState();
}

void SerialMeasurementPanel::setSummaryLines(const QStringList& lines)
{
    if (lines.isEmpty()) {
        showEmptyState();
        return;
    }

    m_summaryLabel->setText(tr("%1 个通道").arg(lines.size()));
    m_view->setPlainText(lines.join(QLatin1Char('\n')));
}

void SerialMeasurementPanel::clear()
{
    showEmptyState();
}

void SerialMeasurementPanel::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(8);

    auto* header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(8);

    auto* title = new QLabel(tr("测量通道"), this);
    title->setObjectName(QStringLiteral("serialMeasurementTitle"));

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("serialMeasurementSummaryLabel"));

    header->addWidget(title);
    header->addStretch();
    header->addWidget(m_summaryLabel);

    m_view = new QPlainTextEdit(this);
    m_view->setObjectName(QStringLiteral("serialMeasurementView"));
    m_view->setReadOnly(true);
    m_view->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_view->setMaximumHeight(110);

    root->addLayout(header);
    root->addWidget(m_view);
}

void SerialMeasurementPanel::showEmptyState()
{
    m_summaryLabel->setText(tr("暂无测量数据"));
    m_view->clear();
}

} // namespace serial_station
