#include "ProtocolView.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QPalette>

ProtocolView::ProtocolView(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ProtocolView::setupUI()
{
    setObjectName("protocolView");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 顶部工具栏
    auto* toolbar = new QWidget;
    toolbar->setObjectName("protocolToolbar");
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);

    m_statusLabel = new QLabel(tr("No frames"));
    m_statusLabel->setObjectName("protocolStatusLabel");
    m_clearBtn = new QPushButton(tr("Clear"));
    m_clearBtn->setObjectName("protocolClearBtn");
    m_clearBtn->setFixedWidth(60);
    m_exportBtn = new QPushButton(tr("Export"));
    m_exportBtn->setObjectName("protocolExportBtn");
    m_exportBtn->setFixedWidth(60);

    toolLayout->addWidget(m_statusLabel, 1);
    toolLayout->addWidget(m_exportBtn);
    toolLayout->addWidget(m_clearBtn);
    layout->addWidget(toolbar);

    // 表格
    m_table = new QTableView;
    m_table->setObjectName("protocolTable");
    m_model = new QStandardItemModel(this);
    // 初始列: 序号 + 时间
    m_model->setHorizontalHeaderLabels({tr("#"), tr("Time")});
    m_table->setModel(m_model);

    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 序号列窄，时间列固定宽度
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 100);

    layout->addWidget(m_table, 1);

    // 信号连接
    connect(m_clearBtn, &QPushButton::clicked, this, &ProtocolView::clear);
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (m_frames.isEmpty()) {
            QMessageBox::information(this, tr("Export"), tr("No data to export"));
            return;
        }

        QString filePath = QFileDialog::getSaveFileName(this, tr("Export Protocol Data"),
                                                         QString(), tr("CSV files (*.csv)"));
        if (filePath.isEmpty()) return;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Export"), tr("Cannot write to file"));
            return;
        }

        QTextStream stream(&file);
        // 写表头
        stream << "#,Time";
        for (const auto& name : m_fieldNames) {
            stream << "," << name;
        }
        stream << "\n";

        // 写数据
        for (int i = 0; i < m_frames.size(); ++i) {
            const auto& frame = m_frames[i];
            stream << (i + 1) << ","
                   << frame.value("_frameTime").toString();
            for (const auto& name : m_fieldNames) {
                stream << "," << frame.value(name).toString();
            }
            stream << "\n";
        }
        file.close();
        m_statusLabel->setText(tr("Exported %1 frames").arg(m_frames.size()));
    });
}

void ProtocolView::addFrame(const QVariantMap& fields)
{
    // 动态更新列头
    updateColumnHeaders(fields);

    // 添加行
    int row = m_model->rowCount();
    m_totalFrames++;

    // 序号
    auto* idxItem = new QStandardItem(QString::number(m_totalFrames));
    idxItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 0, idxItem);

    // 时间
    auto* timeItem = new QStandardItem(fields.value("_frameTime").toString());
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 1, timeItem);

    // 字段值
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QString value = fields.value(m_fieldNames[i]).toString();
        auto* item = new QStandardItem(value);
        item->setTextAlignment(Qt::AlignCenter);
        m_model->setItem(row, kFixedColumns + i, item);
    }

    // 保存帧数据
    m_frames.append(fields);

    // 超过最大行数时移除旧行
    while (m_model->rowCount() > m_maxRows) {
        m_model->removeRow(0);
        if (!m_frames.isEmpty()) m_frames.removeFirst();
    }

    // 自动滚动到最新行
    m_table->scrollToBottom();

    // 更新状态
    m_statusLabel->setText(tr("Frames: %1 | Errors: %2")
                               .arg(m_totalFrames)
                               .arg(m_totalErrors));
}

void ProtocolView::clear()
{
    m_model->removeRows(0, m_model->rowCount());
    m_frames.clear();
    m_totalFrames = 0;
    m_totalErrors = 0;
    m_statusLabel->setText(tr("No frames"));
}

void ProtocolView::setMaxRows(int max)
{
    m_maxRows = max;
}

int ProtocolView::rowCount() const
{
    return m_model->rowCount();
}

QList<QVariantMap> ProtocolView::allFrames() const
{
    return m_frames;
}

void ProtocolView::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    Q_UNUSED(rawFrame);
    addFrame(fields);
}

void ProtocolView::onFrameError(const QString& reason, const QByteArray& rawFrame)
{
    m_totalErrors++;

    // 添加一行错误记录
    QVariantMap errorFields;
    errorFields["_frameTime"] = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    errorFields["Error"] = reason;
    errorFields["RawData"] = HexConverter::toHexString(rawFrame);

    // 动态添加 Error 和 RawData 列（如果不存在）
    if (!m_fieldNames.contains("Error")) {
        m_fieldNames.append("Error");
        m_model->setHorizontalHeaderItem(kFixedColumns + m_fieldNames.indexOf("Error"),
                                          new QStandardItem("Error"));
    }
    if (!m_fieldNames.contains("RawData")) {
        m_fieldNames.append("RawData");
        m_model->setHorizontalHeaderItem(kFixedColumns + m_fieldNames.indexOf("RawData"),
                                          new QStandardItem("RawData"));
    }

    int row = m_model->rowCount();
    m_totalFrames++;

    auto* idxItem = new QStandardItem(QString::number(m_totalFrames));
    idxItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 0, idxItem);

    auto* timeItem = new QStandardItem(errorFields["_frameTime"].toString());
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 1, timeItem);

    // 填充各列
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QString value = errorFields.value(m_fieldNames[i]).toString();
        auto* item = new QStandardItem(value);
        item->setTextAlignment(Qt::AlignCenter);
        // 错误行用语义色标红（从应用palette获取error色）
        item->setForeground(QColor(ThemeColors::kErrorHex));
        m_model->setItem(row, kFixedColumns + i, item);
    }

    m_frames.append(errorFields);
    m_table->scrollToBottom();
    m_statusLabel->setText(tr("Frames: %1 | Errors: %2")
                               .arg(m_totalFrames)
                               .arg(m_totalErrors));
}

void ProtocolView::updateColumnHeaders(const QVariantMap& fields)
{
    // 从fields中提取用户字段名（排除内部字段 _rawPayload, _rawFrame, _frameTime）
    for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
        if (it.key().startsWith('_')) continue;
        if (!m_fieldNames.contains(it.key())) {
            m_fieldNames.append(it.key());
            int col = kFixedColumns + m_fieldNames.size() - 1;
            m_model->setHorizontalHeaderItem(col, new QStandardItem(it.key()));
            if (col == 2) {
                m_table->setColumnWidth(col, 100);
            }
        }
    }
}
