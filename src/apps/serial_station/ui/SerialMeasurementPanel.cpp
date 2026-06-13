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

void SerialMeasurementPanel::setTrendLines(const QStringList& lines)
{
    if (lines.isEmpty()) {
        m_trendView->clear();
        return;
    }

    m_trendLabel->setText(tr("最近 %1 帧").arg(lines.size()));
    m_trendView->setPlainText(lines.join(QLatin1Char('\n')));
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
    m_view->setMaximumHeight(96);

    m_trendLabel = new QLabel(tr("最近帧"), this);
    m_trendLabel->setObjectName(QStringLiteral("serialMeasurementTrendLabel"));

    m_trendView = new QPlainTextEdit(this);
    m_trendView->setObjectName(QStringLiteral("serialMeasurementTrendView"));
    m_trendView->setReadOnly(true);
    m_trendView->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_trendView->setMaximumHeight(118);

    root->addLayout(header);
    root->addWidget(m_view);
    root->addWidget(m_trendLabel);
    root->addWidget(m_trendView);
}

void SerialMeasurementPanel::showEmptyState()
{
    m_summaryLabel->setText(tr("暂无测量数据"));
    m_view->clear();
    if (m_trendLabel != nullptr) {
        m_trendLabel->setText(tr("最近帧"));
    }
    if (m_trendView != nullptr) {
        m_trendView->clear();
    }
}

} // namespace serial_station
