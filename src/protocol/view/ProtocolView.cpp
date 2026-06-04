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

/** @brief 初始化UI布局(工具栏+表格+信号连接) */
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

/** @brief 初始化右键菜单(复制行/复制原始数据/导出JSON/清除) */
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

// 右键菜单和导出功能见 ProtocolViewMenu.cpp

// addFrame/clear/onFrameParsed/onFrameError/resetViewStatistics见 ProtocolViewData.cpp

/** @brief 设置最大显示行数 @param max 行数上限 */
void ProtocolView::setMaxRows(int max) { m_maxRows = max; }

/** @brief 返回当前表格行数 @return 行数 */
int ProtocolView::rowCount() const { return m_model->rowCount(); }

/** @brief 获取所有已解析帧的原始数据列表 @return QVariantMap列表 */
QList<QVariantMap> ProtocolView::allFrames() const { return m_frames; }

/** @brief 设置字段颜色范围配置 @param fieldName 目标字段名 @param range 颜色范围配置 */
void ProtocolView::setFieldColorRange(const QString& fieldName, const FieldColorRange& range)
{
    m_colorRanges[fieldName] = range;
}

// 动态列管理/着色逻辑/列宽自适应见 ProtocolViewDisplay.cpp
