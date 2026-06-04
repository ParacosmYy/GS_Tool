/**
 * @file DataAnnotationWidget.cpp
 * @brief 数据标注控件实现 — 数据流标注管理
 */

#include "utils/annotation/DataAnnotationWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QDateTime>
#include <QColorDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QTableWidget>

/** @brief 构造函数 @param parent 父控件 */
DataAnnotationWidget::DataAnnotationWidget(QWidget* parent)
    : QWidget(parent)
    , m_nextId(1)
    , m_selectedId(-1)
{
    setupUI();
}

/** @brief 初始化UI布局 */
void DataAnnotationWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    /* 工具栏 */
    auto* toolbar = new QHBoxLayout();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName(QStringLiteral("annotSearchEdit"));
    m_searchEdit->setPlaceholderText(tr("搜索标注..."));

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setObjectName(QStringLiteral("annotCategoryCombo"));
    m_categoryCombo->addItem(tr("全部分类"), -1);
    m_categoryCombo->addItem(tr("信息"), 0);
    m_categoryCombo->addItem(tr("警告"), 1);
    m_categoryCombo->addItem(tr("错误"), 2);
    m_categoryCombo->addItem(tr("自定义"), 3);

    m_addBtn = new QPushButton(tr("添加"), this);
    m_addBtn->setObjectName(QStringLiteral("annotAddBtn"));

    m_removeBtn = new QPushButton(tr("删除"), this);
    m_removeBtn->setObjectName(QStringLiteral("annotRemoveBtn"));

    m_editBtn = new QPushButton(tr("编辑"), this);
    m_editBtn->setObjectName(QStringLiteral("annotEditBtn"));

    m_exportBtn = new QPushButton(tr("导出"), this);
    m_exportBtn->setObjectName(QStringLiteral("annotExportBtn"));

    m_importBtn = new QPushButton(tr("导入"), this);
    m_importBtn->setObjectName(QStringLiteral("annotImportBtn"));

    m_countLabel = new QLabel(tr("0 条标注"), this);
    m_countLabel->setObjectName(QStringLiteral("annotCountLabel"));

    toolbar->addWidget(m_searchEdit, 3);
    toolbar->addWidget(m_categoryCombo);
    toolbar->addWidget(m_addBtn);
    toolbar->addWidget(m_removeBtn);
    toolbar->addWidget(m_editBtn);
    toolbar->addWidget(m_exportBtn);
    toolbar->addWidget(m_importBtn);
    toolbar->addWidget(m_countLabel);
    mainLayout->addLayout(toolbar);

    /* 标注表格 */
    m_table = new QTableWidget(0, 5, this);
    m_table->setObjectName(QStringLiteral("annotTable"));
    m_table->setHorizontalHeaderLabels({
        tr("时间"), tr("分类"), tr("标题"), tr("偏移"), tr("备注")
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    mainLayout->addWidget(m_table, 1);

    /* 详情区 */
    m_detailEdit = new QTextEdit(this);
    m_detailEdit->setObjectName(QStringLiteral("annotDetailEdit"));
    m_detailEdit->setReadOnly(true);
    m_detailEdit->setMaximumHeight(80);
    m_detailEdit->setVisible(false);
    mainLayout->addWidget(m_detailEdit);

    /* 信号连接 */
    connect(m_addBtn, &QPushButton::clicked, this, &DataAnnotationWidget::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &DataAnnotationWidget::onRemoveClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &DataAnnotationWidget::onEditClicked);
    connect(m_exportBtn, &QPushButton::clicked, this, &DataAnnotationWidget::onExportClicked);
    connect(m_importBtn, &QPushButton::clicked, this, &DataAnnotationWidget::onImportClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DataAnnotationWidget::onSearchChanged);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataAnnotationWidget::onCategoryFilterChanged);
    connect(m_table, &QTableWidget::cellClicked,
            this, &DataAnnotationWidget::onRowClicked);
}

/** @brief 添加标注并刷新表格 @param annotation 标注数据 @return 标注ID */
int DataAnnotationWidget::addAnnotation(const DataAnnotation& annotation)
{
    DataAnnotation a = annotation;
    a.id = nextId();
    if (a.timestamp <= 0) {
        a.timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    if (!a.color.isValid()) {
        a.color = categoryColor(a.category);
    }
    m_annotations.append(a);

    ++m_stats.totalAnnotations;
    m_stats.activeAnnotations = m_annotations.size();
    if (m_annotations.size() > m_stats.peakAnnotations) {
        m_stats.peakAnnotations = m_annotations.size();
    }

    refreshTable(m_annotations);
    m_countLabel->setText(tr("%1 条标注").arg(m_annotations.size()));
    emit annotationAdded(a.id);
    return a.id;
}

/** @brief 移除标注 @param id 标注ID @return 是否成功 */
bool DataAnnotationWidget::removeAnnotation(int id)
{
    for (int i = 0; i < m_annotations.size(); ++i) {
        if (m_annotations.at(i).id == id) {
            m_annotations.removeAt(i);
            m_stats.activeAnnotations = m_annotations.size();
            refreshTable(m_annotations);
            m_countLabel->setText(tr("%1 条标注").arg(m_annotations.size()));
            emit annotationRemoved(id);
            return true;
        }
    }
    return false;
}

/** @brief 更新标注 @param id 标注ID @param annotation 新数据 @return 是否成功 */
bool DataAnnotationWidget::updateAnnotation(int id, const DataAnnotation& annotation)
{
    for (int i = 0; i < m_annotations.size(); ++i) {
        if (m_annotations.at(i).id == id) {
            DataAnnotation updated = annotation;
            updated.id = id;
            m_annotations[i] = updated;
            ++m_stats.totalEdits;
            refreshTable(m_annotations);
            emit annotationUpdated(id);
            return true;
        }
    }
    return false;
}

/** @brief 获取标注 @param id 标注ID @return 标注数据 */
DataAnnotation DataAnnotationWidget::annotation(int id) const
{
    for (const auto& a : m_annotations) {
        if (a.id == id) return a;
    }
    return DataAnnotation{};
}

/** @brief 获取所有标注 @return 标注列表 */
QList<DataAnnotation> DataAnnotationWidget::allAnnotations() const
{
    return m_annotations;
}

/** @brief 搜索标注 @param keyword 关键字 @return 匹配标注列表 */
QList<DataAnnotation> DataAnnotationWidget::search(const QString& keyword) const
{
    QList<DataAnnotation> results;
    for (const auto& a : m_annotations) {
        if (a.title.contains(keyword, Qt::CaseInsensitive)
            || a.note.contains(keyword, Qt::CaseInsensitive)) {
            results.append(a);
        }
    }
    return results;
}

/** @brief 按分类过滤 @param category 分类 @return 过滤后的标注列表 */
QList<DataAnnotation> DataAnnotationWidget::filterByCategory(int category) const
{
    QList<DataAnnotation> results;
    for (const auto& a : m_annotations) {
        if (category < 0 || a.category == category) {
            results.append(a);
        }
    }
    return results;
}

/** @brief 导出标注到JSON @param filePath 目标路径 @return 是否成功 */
bool DataAnnotationWidget::exportToJson(const QString& filePath)
{
    QJsonArray arr;
    for (const auto& a : m_annotations) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = a.id;
        obj[QStringLiteral("byteOffset")] = static_cast<qint64>(a.byteOffset);
        obj[QStringLiteral("timestamp")] = static_cast<qint64>(a.timestamp);
        obj[QStringLiteral("title")] = a.title;
        obj[QStringLiteral("note")] = a.note;
        obj[QStringLiteral("color")] = a.color.name();
        obj[QStringLiteral("category")] = a.category;
        obj[QStringLiteral("contextData")] = QString::fromUtf8(a.contextData.toHex());
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_stats.totalExports;
    return true;
}

/** @brief 从JSON导入标注 @param filePath JSON文件路径 @return 是否成功 */
bool DataAnnotationWidget::importFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray()) return false;

    const QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        DataAnnotation a;
        a.id = nextId();
        a.byteOffset = obj[QStringLiteral("byteOffset")].toInteger();
        a.timestamp = obj[QStringLiteral("timestamp")].toInteger();
        a.title = obj[QStringLiteral("title")].toString();
        a.note = obj[QStringLiteral("note")].toString();
        a.color = QColor(obj[QStringLiteral("color")].toString());
        a.category = obj[QStringLiteral("category")].toInt();
        a.contextData = QByteArray::fromHex(
            obj[QStringLiteral("contextData")].toString().toUtf8());
        m_annotations.append(a);
    }

    ++m_stats.totalImports;
    m_stats.activeAnnotations = m_annotations.size();
    if (m_annotations.size() > m_stats.peakAnnotations) {
        m_stats.peakAnnotations = m_annotations.size();
    }
    refreshTable(m_annotations);
    m_countLabel->setText(tr("%1 条标注").arg(m_annotations.size()));
    return true;
}

/** @brief 清除所有标注 */
void DataAnnotationWidget::clear()
{
    m_annotations.clear();
    m_selectedId = -1;
    m_stats.activeAnnotations = 0;
    refreshTable(m_annotations);
    m_countLabel->setText(tr("0 条标注"));
    m_detailEdit->setVisible(false);
}

/** @brief 刷新表格 @param annotations 要显示的标注列表 */
void DataAnnotationWidget::refreshTable(const QList<DataAnnotation>& annotations)
{
    m_table->setRowCount(0);
    m_table->setUpdatesEnabled(false);

    for (const auto& a : annotations) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* timeItem = new QTableWidgetItem(formatTimestamp(a.timestamp));
        auto* catItem = new QTableWidgetItem(categoryName(a.category));
        catItem->setForeground(QBrush(categoryColor(a.category)));
        auto* titleItem = new QTableWidgetItem(a.title);
        auto* offsetItem = new QTableWidgetItem(
            QStringLiteral("0x%1").arg(static_cast<quint64>(a.byteOffset), 4, 16, QLatin1Char('0')).toUpper());
        auto* noteItem = new QTableWidgetItem(a.note.left(50));

        m_table->setItem(row, 0, timeItem);
        m_table->setItem(row, 1, catItem);
        m_table->setItem(row, 2, titleItem);
        m_table->setItem(row, 3, offsetItem);
        m_table->setItem(row, 4, noteItem);

        /* 行背景色 */
        QColor bg = a.color;
        bg.setAlpha(30);
        for (int c = 0; c < m_table->columnCount(); ++c) {
            if (m_table->item(row, c)) {
                m_table->item(row, c)->setBackground(QBrush(bg));
            }
        }
    }

    m_table->setUpdatesEnabled(true);
}

/** @brief 格式化时间戳 @param ms 毫秒时间戳 @return 格式化字符串 */
QString DataAnnotationWidget::formatTimestamp(qint64 ms) const
{
    return QDateTime::fromMSecsSinceEpoch(ms).toString(QStringLiteral("HH:mm:ss.zzz"));
}

/** @brief 获取分类颜色 @param category 分类 @return 颜色 */
QColor DataAnnotationWidget::categoryColor(int category) const
{
    switch (category) {
    case Info:    return QColor(52, 152, 219);   // 蓝
    case Warning: return QColor(241, 196, 15);   // 黄
    case Error:   return QColor(231, 76, 60);    // 红
    case Custom:  return QColor(46, 204, 113);   // 绿
    default:      return QColor(149, 165, 166);  // 灰
    }
}

/** @brief 获取分类名称 @param category 分类 @return 名称 */
QString DataAnnotationWidget::categoryName(int category) const
{
    switch (category) {
    case Info:    return tr("信息");
    case Warning: return tr("警告");
    case Error:   return tr("错误");
    case Custom:  return tr("自定义");
    default:      return tr("未知");
    }
}

/** @brief 生成下一个标注ID @return 递增的ID */
int DataAnnotationWidget::nextId()
{
    return m_nextId++;
}

/** @brief 添加按钮点击 — 弹出对话框创建新标注 */
void DataAnnotationWidget::onAddClicked()
{
    DataAnnotation a;
    a.timestamp = QDateTime::currentMSecsSinceEpoch();
    a.category = Info;
    a.title = tr("标注 %1").arg(m_nextId);
    a.color = categoryColor(Info);
    addAnnotation(a);
}

/** @brief 删除按钮点击 */
void DataAnnotationWidget::onRemoveClicked()
{
    if (m_selectedId < 0) return;
    removeAnnotation(m_selectedId);
    m_selectedId = -1;
    m_detailEdit->setVisible(false);
}

/** @brief 编辑按钮点击 */
void DataAnnotationWidget::onEditClicked()
{
    if (m_selectedId < 0) return;
    DataAnnotation a = annotation(m_selectedId);
    /* 切换分类 */
    a.category = (a.category + 1) % 4;
    a.color = categoryColor(a.category);
    updateAnnotation(m_selectedId, a);
}

/** @brief 导出按钮点击 */
void DataAnnotationWidget::onExportClicked()
{
    QString path = QFileDialog::getSaveFileName(this,
        tr("导出标注"), QString(), tr("JSON文件 (*.json)"));
    if (!path.isEmpty()) {
        exportToJson(path);
    }
}

/** @brief 导入按钮点击 */
void DataAnnotationWidget::onImportClicked()
{
    QString path = QFileDialog::getOpenFileName(this,
        tr("导入标注"), QString(), tr("JSON文件 (*.json)"));
    if (!path.isEmpty()) {
        importFromJson(path);
    }
}

/** @brief 搜索文本变化 */
void DataAnnotationWidget::onSearchChanged(const QString& text)
{
    ++m_stats.totalSearches;
    if (text.isEmpty()) {
        refreshTable(m_annotations);
    } else {
        refreshTable(search(text));
    }
}

/** @brief 分类过滤变化 */
void DataAnnotationWidget::onCategoryFilterChanged(int index)
{
    int cat = m_categoryCombo->itemData(index).toInt();
    refreshTable(filterByCategory(cat));
}

/** @brief 表格行点击 — 选中标注并显示详情 */
void DataAnnotationWidget::onRowClicked(int row, int col)
{
    Q_UNUSED(col);
    if (row < 0 || row >= m_annotations.size()) return;

    const auto& a = m_annotations.at(row);
    m_selectedId = a.id;

    QString detail = tr("时间: %1\n偏移: 0x%2\n分类: %3\n标题: %4\n备注: %5")
        .arg(formatTimestamp(a.timestamp))
        .arg(static_cast<quint64>(a.byteOffset), 4, 16, QLatin1Char('0')).toUpper()
        .arg(categoryName(a.category))
        .arg(a.title)
        .arg(a.note);

    if (!a.contextData.isEmpty()) {
        detail += tr("\n上下文: %1").arg(QString::fromUtf8(a.contextData.toHex(' ')));
    }

    m_detailEdit->setPlainText(detail);
    m_detailEdit->setVisible(true);
    emit annotationSelected(a.id);
}
