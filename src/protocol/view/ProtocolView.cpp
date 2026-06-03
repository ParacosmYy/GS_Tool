/**
 * @file ProtocolView.cpp
 * @brief 协议解析结果展示实现
 *
 * 实现增强的协议数据表格视图:
 *   - 字段值着色: 数值按范围变色(正常绿/警告黄/错误红)
 *   - 错误行标红: 解析失败帧整行高亮
 *   - 列宽自动调整: 每50帧或前3帧触发
 *   - 右键菜单: 复制行/原始数据/导出JSON
 */
#include "protocol/view/ProtocolView.h"
#include "utils/crypto/HexConverter.h"
#include "core/theme/Constants.h"
#include "core/theme/ThemeManager.h"
#include <QFileDialog>
#include "core/widgets/EdDialog.h"
#include <QFile>
#include <QTextStream>

// ============================================================
// 构造与UI
// ============================================================

/** @brief 构造协议帧视图(表格+右键菜单+滚动到底部) @param parent 父控件 */
ProtocolView::ProtocolView(QWidget* parent) : QWidget(parent)
{
    setupUI();
    setupContextMenu();
}

/**
 * @brief 初始化UI布局
 *
 * 创建工具栏(状态标签+清除/导出按钮)和数据表格，
 * 连接清除按钮到clear()，导出按钮弹出文件对话框写CSV。
 */
void ProtocolView::setupUI()
{
    setObjectName("protocolView");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 工具栏
    auto* toolbar = new QWidget;
    toolbar->setObjectName("protocolToolbar");
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);
    m_statusLabel = new QLabel(tr("暂无数据"));
    m_statusLabel->setObjectName("protocolStatusLabel");
    m_clearBtn = new QPushButton(tr("清除"));
    m_clearBtn->setObjectName("protocolClearBtn");
    m_clearBtn->setFixedWidth(60);
    m_exportBtn = new QPushButton(tr("导出"));
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
    m_model->setHorizontalHeaderLabels({tr("#"), tr("时间")});
    m_table->setModel(m_model);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 100);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_table, 1);

    // 信号连接
    connect(m_clearBtn, &QPushButton::clicked, this, &ProtocolView::clear);
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        if (m_frames.isEmpty()) {
            EdDialog::error(this, tr("导出"), tr("无数据可导出"));
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(
            this, tr("导出协议数据"), QString(), tr("CSV 文件 (*.csv)"));
        if (filePath.isEmpty()) return;
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            EdDialog::error(this, tr("导出"), tr("无法写入文件"));
            return;
        }
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << "#,Time";
        for (const auto& name : m_fieldNames) stream << "," << name;
        stream << "\n";
        for (int i = 0; i < m_frames.size(); ++i) {
            const auto& frame = m_frames[i];
            stream << (i + 1) << "," << frame.value("_frameTime").toString();
            for (const auto& name : m_fieldNames) stream << "," << frame.value(name).toString();
            stream << "\n";
        }
        stream.flush();
        if (file.error() != QFile::NoError) {
            EdDialog::error(this, tr("导出"), tr("写入文件失败: %1").arg(file.errorString()));
            file.close();
            return;
        }
        file.close();
        m_statusLabel->setText(tr("已导出 %1 帧").arg(m_frames.size()));
        ++m_totalExports;  ///< 统计: CSV导出
    });
}

/**
 * @brief 初始化右键菜单
 *
 * 创建包含"复制行"、"复制原始数据"、分隔线、"导出JSON"、分隔线、"清除"的上下文菜单，
 * 并连接表格的customContextMenuRequested信号。
 */
void ProtocolView::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    m_contextMenu->setObjectName("protocolContextMenu");
    m_copyRowAction = new QAction(tr("复制行"), this);
    m_copyRowAction->setObjectName("protocolCopyRowAction");
    connect(m_copyRowAction, &QAction::triggered, this, &ProtocolView::copyRow);
    m_contextMenu->addAction(m_copyRowAction);
    m_copyRawAction = new QAction(tr("复制原始数据"), this);
    m_copyRawAction->setObjectName("protocolCopyRawAction");
    connect(m_copyRawAction, &QAction::triggered, this, &ProtocolView::copyRaw);
    m_contextMenu->addAction(m_copyRawAction);
    m_contextMenu->addSeparator();
    m_exportJsonAction = new QAction(tr("导出JSON"), this);
    m_exportJsonAction->setObjectName("protocolExportJsonAction");
    connect(m_exportJsonAction, &QAction::triggered, this, &ProtocolView::exportJson);
    m_contextMenu->addAction(m_exportJsonAction);
    m_contextMenu->addSeparator();
    m_clearAction = new QAction(tr("清除"), this);
    m_clearAction->setObjectName("protocolClearAction");
    connect(m_clearAction, &QAction::triggered, this, &ProtocolView::clear);
    m_contextMenu->addAction(m_clearAction);
    connect(m_table, &QTableView::customContextMenuRequested,
            this, &ProtocolView::onCustomContextMenu);
}

// ============================================================
// 右键菜单
// ============================================================

/** @brief 右键菜单弹出回调，根据选中状态启用/禁用菜单项 @param pos 点击位置 */
void ProtocolView::onCustomContextMenu(const QPoint& pos)
{
    if (!m_table->selectionModel()) return;
    bool hasSelection = m_table->selectionModel()->hasSelection();
    m_copyRowAction->setEnabled(hasSelection);
    m_copyRawAction->setEnabled(hasSelection);
    m_exportJsonAction->setEnabled(!m_frames.isEmpty());
    ++m_totalContextMenuActions;  ///< 统计: 右键菜单弹出
    m_contextMenu->popup(m_table->viewport()->mapToGlobal(pos));
}

/**
 * @brief 复制选中行文本到剪贴板
 *
 * 以制表符分隔各列值，写入系统剪贴板。
 */
void ProtocolView::copyRow()
{
    if (!m_table->selectionModel()) return;
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;
    QStringList cols;
    cols << m_model->data(m_model->index(row, 0)).toString();
    cols << m_model->data(m_model->index(row, 1)).toString();
    for (int i = 0; i < m_fieldNames.size(); ++i)
        cols << m_model->data(m_model->index(row, kFixedColumns + i)).toString();
    QApplication::clipboard()->setText(cols.join("\t"));
    m_statusLabel->setText(tr("已复制行"));
}

/**
 * @brief 复制选中帧的原始HEX数据到剪贴板
 *
 * 优先使用RawData字段，无RawData时拼接所有字段值。
 */
void ProtocolView::copyRaw()
{
    if (!m_table->selectionModel()) return;
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;
    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;
    const QVariantMap& frame = m_frames[row];
    QString rawData = frame.value("RawData").toString();
    if (rawData.isEmpty()) {
        QStringList parts;
        for (const auto& name : m_fieldNames) parts << frame.value(name).toString();
        rawData = parts.join(" ");
    }
    QApplication::clipboard()->setText(rawData);
    m_statusLabel->setText(tr("已复制原始数据"));
}

/**
 * @brief 导出所有帧为JSON文件
 *
 * 弹出文件保存对话框，将所有帧数据序列化为格式化JSON写入文件，
 * JSON包含frames数组、导出时间、总帧数和总错误数。
 */
void ProtocolView::exportJson()
{
    if (m_frames.isEmpty()) {
        EdDialog::error(this, tr("导出JSON"), tr("无数据可导出"));
        return;
    }
    QString filePath = QFileDialog::getSaveFileName(
        this, tr("导出JSON"), QString(), tr("JSON 文件 (*.json)"));
    if (filePath.isEmpty()) return;

    QJsonObject root;
    QJsonArray framesArray;
    for (int i = 0; i < m_frames.size(); ++i) {
        const auto& frame = m_frames[i];
        QJsonObject frameObj;
        frameObj["#"] = i + 1;
        frameObj["Time"] = frame.value("_frameTime").toString();
        for (const auto& name : m_fieldNames) {
            QString value = frame.value(name).toString();
            bool ok = false;
            double numVal = value.toDouble(&ok);
            if (ok) frameObj[name] = numVal; else frameObj[name] = value;
        }
        framesArray.append(frameObj);
    }
    root["frames"] = framesArray;
    root["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["totalFrames"] = static_cast<qint64>(m_totalFrames);
    root["totalErrors"] = static_cast<qint64>(m_totalErrors);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        EdDialog::error(this, tr("导出JSON"), tr("无法写入文件"));
        return;
    }
    QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(json) != json.size()) {
        EdDialog::error(this, tr("导出JSON"), tr("写入文件失败，磁盘可能已满"));
        return;
    }
    file.close();
    m_statusLabel->setText(tr("已导出 %1 帧到JSON").arg(m_frames.size()));
    ++m_totalExports;  ///< 统计: JSON导出
}

// ============================================================
// 数据管理
// ============================================================

/**
 * @brief 添加帧数据到表格
 *
 * 流程: 更新列头 -> 创建着色单元格 -> 保存帧 -> 超限移除 -> 自动调整列宽。
 * 自动提取字段名创建新列，每50帧或前3帧触发列宽自适应。
 *
 * @param fields 解析后的字段映射，可含"_frameTime"特殊字段
 */
void ProtocolView::addFrame(const QVariantMap& fields)
{
    updateColumnHeaders(fields);
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
    // 字段值(带着色)
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QString value = fields.value(m_fieldNames[i]).toString();
        auto* item = createColoredItem(m_fieldNames[i], value);
        item->setTextAlignment(Qt::AlignCenter);
        m_model->setItem(row, kFixedColumns + i, item);
    }

    m_frames.append(fields);
    while (m_model->rowCount() > m_maxRows) {
        m_model->removeRow(0);
        if (!m_frames.isEmpty()) m_frames.removeFirst();
    }
    m_table->scrollToBottom();
    // 自动调整列宽(每50帧或前3帧)
    if (m_totalFrames % 50 == 0 || m_totalFrames <= 3) autoResizeColumns();
    m_statusLabel->setText(tr("帧数: %1 | 错误: %2").arg(m_totalFrames).arg(m_totalErrors));
    ++m_totalFramesDisplayed;  ///< 统计: 帧展示计数
}

/**
 * @brief 清除表格所有行并重置帧计数器
 */
void ProtocolView::clear()
{
    m_model->removeRows(0, m_model->rowCount());
    m_frames.clear();
    m_totalFrames = 0;
    m_totalErrors = 0;
    m_statusLabel->setText(tr("暂无数据"));
}

/** @brief 设置最大显示行数 @param max 行数上限 */
void ProtocolView::setMaxRows(int max) { m_maxRows = max; }

/** @brief 返回当前表格行数 @return 行数 */
int ProtocolView::rowCount() const { return m_model->rowCount(); }

/** @brief 获取所有已解析帧的原始数据列表 @return QVariantMap列表 */
QList<QVariantMap> ProtocolView::allFrames() const { return m_frames; }

/**
 * @brief 设置字段颜色范围配置
 *
 * 配置后，对应字段的数值将按范围着色：正常区间默认色、警告区间黄色、错误区间红色。
 *
 * @param fieldName 目标字段名
 * @param range 颜色范围配置（含normalLow/normalHigh/warnLow/warnHigh）
 */
void ProtocolView::setFieldColorRange(const QString& fieldName, const FieldColorRange& range)
{
    m_colorRanges[fieldName] = range;
}

/**
 * @brief 帧解析成功回调：添加到表格
 * @param fields 解析后的字段映射
 * @param rawFrame 原始帧数据（当前未使用）
 */
void ProtocolView::onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame)
{
    Q_UNUSED(rawFrame);
    addFrame(fields);
}

/**
 * @brief 帧解析错误回调：添加错误行(红色高亮)
 *
 * 动态添加Error/RawData列（如不存在），错误行整行使用ThemeManager::Error色标红。
 *
 * @param reason 错误原因描述
 * @param rawFrame 导致错误的原始帧数据
 */
void ProtocolView::onFrameError(const QString& reason, const QByteArray& rawFrame)
{
    m_totalErrors++;
    QVariantMap errorFields;
    errorFields["_frameTime"] = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    errorFields["Error"] = reason;
    errorFields["RawData"] = HexConverter::toHexString(rawFrame);

    // 动态添加Error/RawData列
    if (!m_fieldNames.contains("Error")) {
        m_fieldNames.append("Error");
        m_model->setHorizontalHeaderItem(kFixedColumns + m_fieldNames.indexOf("Error"),
                                          new QStandardItem(tr("错误")));
    }
    if (!m_fieldNames.contains("RawData")) {
        m_fieldNames.append("RawData");
        m_model->setHorizontalHeaderItem(kFixedColumns + m_fieldNames.indexOf("RawData"),
                                          new QStandardItem(tr("原始数据")));
    }

    int row = m_model->rowCount();
    m_totalFrames++;
    auto* idxItem = new QStandardItem(QString::number(m_totalFrames));
    idxItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 0, idxItem);
    auto* timeItem = new QStandardItem(errorFields["_frameTime"].toString());
    timeItem->setTextAlignment(Qt::AlignCenter);
    m_model->setItem(row, 1, timeItem);

    // 整行标红
    QColor errorColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        QString value = errorFields.value(m_fieldNames[i]).toString();
        auto* item = new QStandardItem(value);
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(errorColor);
        m_model->setItem(row, kFixedColumns + i, item);
    }
    m_frames.append(errorFields);
    while (m_model->rowCount() > m_maxRows) {
        m_model->removeRow(0);
        if (!m_frames.isEmpty()) m_frames.removeFirst();
    }
    m_table->scrollToBottom();
    m_statusLabel->setText(tr("帧数: %1 | 错误: %2").arg(m_totalFrames).arg(m_totalErrors));
}

/** @brief 重置协议视图统计计数器 */
void ProtocolView::resetViewStatistics()
{
    m_totalFramesDisplayed = 0;
    m_totalExports = 0;
    m_totalContextMenuActions = 0;
}

// ============================================================
// 动态列管理
// ============================================================

/**
 * @brief 动态更新列头
 *
 * 遍历fields中的字段，遇到新字段名时自动追加列并设置100px初始宽度。
 * 忽略以下划线开头的内部字段。
 *
 * @param fields 当前帧的字段映射
 */
void ProtocolView::updateColumnHeaders(const QVariantMap& fields)
{
    for (auto it = fields.constBegin(); it != fields.constEnd(); ++it) {
        if (it.key().startsWith('_')) continue;
        if (!m_fieldNames.contains(it.key())) {
            m_fieldNames.append(it.key());
            int col = kFixedColumns + m_fieldNames.size() - 1;
            m_model->setHorizontalHeaderItem(col, new QStandardItem(it.key()));
            m_table->setColumnWidth(col, 100);
        }
    }
}

// ============================================================
// 着色逻辑
// ============================================================

/**
 * @brief 根据字段值创建着色的QStandardItem
 *
 * 着色规则: normalLow~normalHigh为默认前景色，
 * 超出正常但仍在warnLow~warnHigh范围内为警告色，
 * 超出警告范围为错误色。非数值字段不着色。
 *
 * @param fieldName 字段名，用于查找颜色范围配置
 * @param value 字段值的字符串表示
 * @return 带有前景色设置的QStandardItem
 */
QStandardItem* ProtocolView::createColoredItem(const QString& fieldName,
                                                const QString& value)
{
    auto* item = new QStandardItem(value);
    auto it = m_colorRanges.find(fieldName);
    if (it == m_colorRanges.end()) return item;

    bool ok = false;
    double numVal = value.toDouble(&ok);
    if (!ok) return item;

    const FieldColorRange& range = it.value();
    QColor warnColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Warning);
    QColor errColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Error);

    if (numVal >= range.normalLow && numVal <= range.normalHigh) {
        // 正常区间: 默认前景
    } else if ((numVal >= range.warnLow && numVal < range.normalLow) ||
               (numVal > range.normalHigh && numVal <= range.warnHigh)) {
        item->setForeground(warnColor);  // 警告区间
    } else {
        item->setForeground(errColor);   // 错误区间(超出警告范围)
    }
    return item;
}

/**
 * @brief 自动调整所有列宽以适应内容
 *
 * 序号列固定50px，时间列固定100px，数据列按内容自适应(60~200px)。
 */
void ProtocolView::autoResizeColumns()
{
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 100);
    for (int i = 0; i < m_fieldNames.size(); ++i) {
        int col = kFixedColumns + i;
        m_table->resizeColumnToContents(col);
        int w = m_table->columnWidth(col);
        if (w > 200) m_table->setColumnWidth(col, 200);
        else if (w < 60) m_table->setColumnWidth(col, 60);
    }
}
