#include "ProtocolView.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"
#include "core/ThemeManager.h"
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
    setupContextMenu();
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

    // 启用右键上下文菜单
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);

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

void ProtocolView::setupContextMenu()
{
    // 创建右键上下文菜单
    m_contextMenu = new QMenu(this);
    m_contextMenu->setObjectName("protocolContextMenu");

    // 复制行 - 将选中行的数据以制表符分隔复制到剪贴板
    m_copyRowAction = new QAction(tr("Copy Row"), this);
    m_copyRowAction->setObjectName("protocolCopyRowAction");
    connect(m_copyRowAction, &QAction::triggered, this, &ProtocolView::copyRow);
    m_contextMenu->addAction(m_copyRowAction);

    // 复制原始数据 - 复制选中帧的原始十六进制数据
    m_copyRawAction = new QAction(tr("Copy Raw"), this);
    m_copyRawAction->setObjectName("protocolCopyRawAction");
    connect(m_copyRawAction, &QAction::triggered, this, &ProtocolView::copyRaw);
    m_contextMenu->addAction(m_copyRawAction);

    m_contextMenu->addSeparator();

    // 导出JSON - 将所有帧数据导出为JSON格式
    m_exportJsonAction = new QAction(tr("Export JSON"), this);
    m_exportJsonAction->setObjectName("protocolExportJsonAction");
    connect(m_exportJsonAction, &QAction::triggered, this, &ProtocolView::exportJson);
    m_contextMenu->addAction(m_exportJsonAction);

    m_contextMenu->addSeparator();

    // 清空 - 清除所有数据
    m_clearAction = new QAction(tr("Clear"), this);
    m_clearAction->setObjectName("protocolClearAction");
    connect(m_clearAction, &QAction::triggered, this, &ProtocolView::clear);
    m_contextMenu->addAction(m_clearAction);

    // 连接表格的右键信号
    connect(m_table, &QTableView::customContextMenuRequested,
            this, &ProtocolView::onCustomContextMenu);
}

void ProtocolView::onCustomContextMenu(const QPoint& pos)
{
    // 无数据时只显示清空和导出JSON（置灰）
    bool hasSelection = m_table->selectionModel()->hasSelection();
    m_copyRowAction->setEnabled(hasSelection);
    m_copyRawAction->setEnabled(hasSelection);
    m_exportJsonAction->setEnabled(!m_frames.isEmpty());

    m_contextMenu->popup(m_table->viewport()->mapToGlobal(pos));
}

void ProtocolView::copyRow()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;

    // 收集该行所有列的文本，用制表符分隔
    QStringList cols;

    // 序号列
    QModelIndex idx = m_model->index(row, 0);
    cols << m_model->data(idx).toString();

    // 时间列
    QModelIndex timeIdx = m_model->index(row, 1);
    cols << m_model->data(timeIdx).toString();

    // 动态字段列
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QModelIndex fieldIdx = m_model->index(row, kFixedColumns + i);
        cols << m_model->data(fieldIdx).toString();
    }

    QApplication::clipboard()->setText(cols.join("\t"));
    m_statusLabel->setText(tr("Row copied"));
}

void ProtocolView::copyRaw()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;

    // 从保存的帧数据中提取原始十六进制数据
    const QVariantMap& frame = m_frames[row];
    QString rawData = frame.value("RawData").toString();

    // 如果没有RawData字段（正常帧），则尝试拼接所有字段值
    if (rawData.isEmpty()) {
        QStringList parts;
        for (const auto& name : m_fieldNames) {
            parts << frame.value(name).toString();
        }
        rawData = parts.join(" ");
    }

    QApplication::clipboard()->setText(rawData);
    m_statusLabel->setText(tr("Raw data copied"));
}

void ProtocolView::exportJson()
{
    if (m_frames.isEmpty()) {
        QMessageBox::information(this, tr("Export JSON"), tr("No data to export"));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, tr("Export JSON"),
        QString(), tr("JSON files (*.json)"));

    if (filePath.isEmpty()) return;

    // 构建JSON文档
    QJsonObject root;
    QJsonArray framesArray;

    for (int i = 0; i < m_frames.size(); ++i) {
        const auto& frame = m_frames[i];
        QJsonObject frameObj;

        // 序号（从1开始）
        frameObj["#"] = i + 1;
        // 时间戳
        frameObj["Time"] = frame.value("_frameTime").toString();

        // 动态字段
        for (const auto& name : m_fieldNames) {
            QString value = frame.value(name).toString();
            // 尝试将纯数字字符串转为数值类型
            bool ok = false;
            double numVal = value.toDouble(&ok);
            if (ok) {
                frameObj[name] = numVal;
            } else {
                frameObj[name] = value;
            }
        }

        framesArray.append(frameObj);
    }

    root["frames"] = framesArray;
    root["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["totalFrames"] = static_cast<qint64>(m_totalFrames);
    root["totalErrors"] = static_cast<qint64>(m_totalErrors);

    // 写入文件
    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export JSON"), tr("Cannot write to file"));
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_statusLabel->setText(tr("Exported %1 frames to JSON").arg(m_frames.size()));
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
        // 错误行用语义色标红（从ThemeManager获取error色，适配主题切换）
        item->setForeground(ThemeManager::instance().color(ThemeManager::SemanticColor::Error));
        m_model->setItem(row, kFixedColumns + i, item);
    }

    m_frames.append(errorFields);

    // 错误行同样受最大行数限制，超出时从头部移除旧行（与addFrame一致）
    while (m_model->rowCount() > m_maxRows) {
        m_model->removeRow(0);
        if (!m_frames.isEmpty()) m_frames.removeFirst();
    }

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
