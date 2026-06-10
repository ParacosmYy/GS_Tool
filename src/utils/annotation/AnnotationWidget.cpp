/**
 * @file AnnotationWidget.cpp
 * @brief 数据标注工具控件实现 — 时间轴绘制 + 列表管理 + 工具栏交互
 */

#include "utils/annotation/AnnotationWidget.h"
#include "utils/annotation/AnnotationManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

/** @brief 时间轴区域在控件顶部的固定高度 */
static constexpr int kTimelineHeight = 40;

/* ── 构造 / UI ── */

/** @brief 构造控件 @param manager 标注管理器 @param parent 父控件 */
AnnotationWidget::AnnotationWidget(AnnotationManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager)
{
    setObjectName(QStringLiteral("annotationWidget"));
    setupUI();

    /* 连接管理器信号 */
    connect(m_manager, &AnnotationManager::annotationAdded,
            this, &AnnotationWidget::onAnnotationAdded);
    connect(m_manager, &AnnotationManager::annotationRemoved,
            this, &AnnotationWidget::onAnnotationRemoved);
    connect(m_manager, &AnnotationManager::annotationUpdated,
            this, &AnnotationWidget::onAnnotationUpdated);
}

/** @brief 初始化 UI 布局 */
void AnnotationWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    /* 时间轴占位 — 实际由 paintEvent 绘制 */
    setMinimumHeight(kTimelineHeight + 200);
    setMouseTracking(true);

    /* 工具栏 */
    auto* toolbar = new QHBoxLayout();
    toolbar->setSpacing(4);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName(QStringLiteral("annotToolSearchEdit"));
    m_searchEdit->setPlaceholderText(tr("搜索标签/分类..."));

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setObjectName(QStringLiteral("annotToolFilterCombo"));
    m_filterCombo->addItem(tr("全部类型"), -1);
    m_filterCombo->addItem(tr("标记"),   static_cast<int>(AnnotationKind::Marker));
    m_filterCombo->addItem(tr("区域"),   static_cast<int>(AnnotationKind::Region));
    m_filterCombo->addItem(tr("事件"),   static_cast<int>(AnnotationKind::Event));
    m_filterCombo->addItem(tr("测量"),   static_cast<int>(AnnotationKind::Measurement));

    m_addBtn = new QPushButton(tr("添加"), this);
    m_addBtn->setObjectName(QStringLiteral("annotToolAddBtn"));

    m_editBtn = new QPushButton(tr("编辑"), this);
    m_editBtn->setObjectName(QStringLiteral("annotToolEditBtn"));

    m_removeBtn = new QPushButton(tr("删除"), this);
    m_removeBtn->setObjectName(QStringLiteral("annotToolRemoveBtn"));

    m_undoBtn = new QPushButton(tr("撤销"), this);
    m_undoBtn->setObjectName(QStringLiteral("annotToolUndoBtn"));

    m_redoBtn = new QPushButton(tr("重做"), this);
    m_redoBtn->setObjectName(QStringLiteral("annotToolRedoBtn"));

    m_exportBtn = new QPushButton(tr("导出"), this);
    m_exportBtn->setObjectName(QStringLiteral("annotToolExportBtn"));

    m_countLabel = new QLabel(tr("0 条标注"), this);
    m_countLabel->setObjectName(QStringLiteral("annotToolCountLabel"));

    toolbar->addWidget(m_searchEdit, 3);
    toolbar->addWidget(m_filterCombo);
    toolbar->addWidget(m_addBtn);
    toolbar->addWidget(m_editBtn);
    toolbar->addWidget(m_removeBtn);
    toolbar->addWidget(m_undoBtn);
    toolbar->addWidget(m_redoBtn);
    toolbar->addWidget(m_exportBtn);
    toolbar->addWidget(m_countLabel);

    /* 标注表格 */
    m_table = new QTableWidget(0, 5, this);
    m_table->setObjectName(QStringLiteral("annotToolTable"));
    m_table->setHorizontalHeaderLabels({
        tr("ID"), tr("类型"), tr("位置"), tr("标签"), tr("分类")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    /* 隐藏 ID 列（太长），但保留数据供程序访问 */
    m_table->setColumnHidden(0, true);

    layout->addLayout(toolbar, 0);
    layout->addWidget(m_table, 1);

    /* 信号连接 */
    connect(m_addBtn, &QPushButton::clicked, this, &AnnotationWidget::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &AnnotationWidget::onEditClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &AnnotationWidget::onRemoveClicked);
    connect(m_undoBtn, &QPushButton::clicked, this, &AnnotationWidget::onUndoClicked);
    connect(m_redoBtn, &QPushButton::clicked, this, &AnnotationWidget::onRedoClicked);
    connect(m_exportBtn, &QPushButton::clicked, this, &AnnotationWidget::onExportClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AnnotationWidget::onSearchChanged);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnnotationWidget::onFilterChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, &AnnotationWidget::onTableDoubleClicked);

    refreshTable();
}

/* ── 时间轴绘制 ── */

/** @brief 绘制事件 — 时间轴 + 标注标记 */
void AnnotationWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = kTimelineHeight;
    const qint64 range = m_timelineMax - m_timelineMin;

    /* 背景 */
    p.fillRect(0, 0, w, h, QColor(30, 30, 30));
    p.setPen(QColor(60, 60, 60));
    p.drawLine(0, h - 1, w, h - 1);

    if (range <= 0) return;

    /* 绘制标注标记 */
    const auto annotations = m_manager->allAnnotations();
    for (const auto& a : annotations) {
        QColor c = a.color;
        c.setAlpha(180);

        if (a.isRegion()) {
            /* 区域标注 — 色块 */
            qreal x1 = static_cast<qreal>(a.startPos - m_timelineMin) / range * w;
            qreal x2 = static_cast<qreal>(a.endPos - m_timelineMin) / range * w;
            p.fillRect(QRectF(qMin(x1, x2), 4, qAbs(x2 - x1), h - 8), c);
        } else {
            /* 单点标注 — 竖线 + 三角 */
            qreal x = static_cast<qreal>(a.startPos - m_timelineMin) / range * w;
            p.setPen(QPen(c, 2));
            p.drawLine(static_cast<int>(x), 4, static_cast<int>(x), h - 4);
            /* 顶部三角 */
            QPolygonF tri;
            tri << QPointF(x - 4, 4) << QPointF(x + 4, 4) << QPointF(x, 10);
            p.setBrush(c);
            p.drawPolygon(tri);
        }
    }
}

/** @brief 鼠标点击 — 时间轴区域转换为位置坐标 */
void AnnotationWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->y() < kTimelineHeight) {
        qint64 range = m_timelineMax - m_timelineMin;
        if (range > 0) {
            qreal ratio = static_cast<qreal>(event->x()) / width();
            qint64 pos = m_timelineMin + static_cast<qint64>(ratio * range);
            ++m_stats.totalTimelineClicks;

            AnnotationEntry a;
            a.startPos = pos;
            a.endPos   = pos;
            a.type     = AnnotationKind::Marker;
            a.label    = tr("标注 @ %1").arg(pos);
            m_manager->addAnnotation(a);
            emit timelineClicked(pos);
        }
    }
    QWidget::mousePressEvent(event);
}

/* ── 公共接口 ── */

/** @brief 设置时间轴范围 @param minPos 起始 @param maxPos 结束 */
void AnnotationWidget::setDataRange(qint64 minPos, qint64 maxPos)
{
    m_timelineMin = minPos;
    m_timelineMax = maxPos;
    update();
}

/* ── 槽函数 ── */

/** @brief 添加按钮 — 在时间轴中点创建标注 */
void AnnotationWidget::onAddClicked()
{
    ++m_stats.totalAdds;
    AnnotationEntry a;
    a.startPos = (m_timelineMin + m_timelineMax) / 2;
    a.endPos   = a.startPos;
    a.label    = tr("新标注");
    m_manager->addAnnotation(a);
}

/** @brief 编辑按钮 — 切换选中标注的类型 */
void AnnotationWidget::onEditClicked()
{
    if (m_selectedId.isEmpty()) return;
    ++m_stats.totalEdits;
    AnnotationEntry a = m_manager->annotation(m_selectedId);
    if (!a.id.isEmpty()) {
        int nextType = (static_cast<int>(a.type) + 1) % 4;
        a.type = static_cast<AnnotationKind>(nextType);
        m_manager->updateAnnotation(m_selectedId, a);
    }
}

/** @brief 删除按钮 */
void AnnotationWidget::onRemoveClicked()
{
    if (m_selectedId.isEmpty()) return;
    ++m_stats.totalDeletes;
    m_manager->removeAnnotation(m_selectedId);
    m_selectedId.clear();
}

/** @brief 撤销按钮 */
void AnnotationWidget::onUndoClicked()
{
    m_manager->undo();
    refreshTable();
}

/** @brief 重做按钮 */
void AnnotationWidget::onRedoClicked()
{
    m_manager->redo();
    refreshTable();
}

/** @brief 导出按钮 */
void AnnotationWidget::onExportClicked()
{
    QString path = QFileDialog::getSaveFileName(this,
        tr("导出标注"), QString(),
        tr("JSON 文件 (*.json);;CSV 文件 (*.csv)"));
    if (path.isEmpty()) return;
    if (path.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        m_manager->exportToCsv(path);
    } else {
        m_manager->saveToJson(path);
    }
}

/** @brief 搜索文本变化 */
void AnnotationWidget::onSearchChanged(const QString& text)
{
    Q_UNUSED(text);
    refreshTable();
}

/** @brief 类型过滤变化 */
void AnnotationWidget::onFilterChanged(int index)
{
    Q_UNUSED(index);
    refreshTable();
}

/** @brief 表格双击 — 编辑标注 */
void AnnotationWidget::onTableDoubleClicked(int row, int col)
{
    Q_UNUSED(col);
    ++m_stats.totalTableDblClicks;
    auto* idItem = m_table->item(row, 0);
    if (!idItem) return;
    m_selectedId = idItem->text();
    onEditClicked();
    emit annotationClicked(m_selectedId);
}

/** @brief 管理器标注添加 — 增量刷新 */
void AnnotationWidget::onAnnotationAdded(const AnnotationEntry& a)
{
    addTableRow(a);
    m_countLabel->setText(tr("%1 条标注").arg(m_table->rowCount()));
    update();
}

/** @brief 管理器标注移除 — 刷新整个表格 */
void AnnotationWidget::onAnnotationRemoved(const QString& id)
{
    int row = findRowById(id);
    if (row >= 0) m_table->removeRow(row);
    m_countLabel->setText(tr("%1 条标注").arg(m_table->rowCount()));
    update();
}

/** @brief 管理器标注更新 — 更新对应行 */
void AnnotationWidget::onAnnotationUpdated(const AnnotationEntry& a)
{
    int row = findRowById(a.id);
    if (row >= 0) updateTableRow(row, a);
    update();
}

/* ── 内部方法 ── */

/** @brief 完整刷新表格（根据搜索和过滤条件） */
void AnnotationWidget::refreshTable()
{
    ++m_stats.totalRefreshes;
    m_table->setRowCount(0);
    m_table->setUpdatesEnabled(false);

    QList<AnnotationEntry> list = m_manager->allAnnotations();

    /* 类型过滤 */
    int typeFilter = m_filterCombo->currentData().toInt();
    if (typeFilter >= 0) {
        auto filtered = m_manager->filterByType(
            static_cast<AnnotationKind>(typeFilter));
        list = filtered;
    }

    /* 关键字搜索 */
    QString keyword = m_searchEdit->text().trimmed();
    if (!keyword.isEmpty()) {
        QList<AnnotationEntry> filtered;
        for (const auto& a : list) {
            if (a.label.contains(keyword, Qt::CaseInsensitive)
                || a.category.contains(keyword, Qt::CaseInsensitive)) {
                filtered.append(a);
            }
        }
        list = filtered;
    }

    for (const auto& a : list) {
        addTableRow(a);
    }

    m_table->setUpdatesEnabled(true);
    m_countLabel->setText(tr("%1 条标注").arg(m_table->rowCount()));
}

/** @brief 添加表格行 @param a 标注数据 */
void AnnotationWidget::addTableRow(const AnnotationEntry& a)
{
    int row = m_table->rowCount();
    m_table->insertRow(row);
    updateTableRow(row, a);
}

/** @brief 更新表格行 @param row 行号 @param a 标注数据 */
void AnnotationWidget::updateTableRow(int row, const AnnotationEntry& a)
{
    auto* idItem     = new QTableWidgetItem(a.id);
    auto* typeItem   = new QTableWidgetItem(typeDisplayName(a.type));
    auto* posItem    = new QTableWidgetItem(
        a.isRegion()
            ? QStringLiteral("%1 ~ %2").arg(a.startPos).arg(a.endPos)
            : QString::number(a.startPos));
    auto* labelItem  = new QTableWidgetItem(a.label);
    auto* catItem    = new QTableWidgetItem(a.category);

    typeItem->setForeground(QBrush(a.color));
    m_table->setItem(row, 0, idItem);
    m_table->setItem(row, 1, typeItem);
    m_table->setItem(row, 2, posItem);
    m_table->setItem(row, 3, labelItem);
    m_table->setItem(row, 4, catItem);

    /* 行背景色 */
    QColor bg = a.color;
    bg.setAlpha(25);
    for (int c = 0; c < m_table->columnCount(); ++c) {
        if (auto* item = m_table->item(row, c)) {
            item->setBackground(QBrush(bg));
        }
    }
}

/** @brief 按 ID 查找行号 @param id 标注 ID @return 行号（-1 表示未找到） */
int AnnotationWidget::findRowById(const QString& id) const
{
    for (int r = 0; r < m_table->rowCount(); ++r) {
        if (auto* item = m_table->item(r, 0)) {
            if (item->text() == id) return r;
        }
    }
    return -1;
}

/** @brief 标注类型的显示名称 @param type 标注类型 @return 经过 tr() 的名称 */
QString AnnotationWidget::typeDisplayName(AnnotationKind type) const
{
    switch (type) {
    case AnnotationKind::Marker:      return tr("标记");
    case AnnotationKind::Region:      return tr("区域");
    case AnnotationKind::Event:       return tr("事件");
    case AnnotationKind::Measurement: return tr("测量");
    }
    return tr("未知");
}
