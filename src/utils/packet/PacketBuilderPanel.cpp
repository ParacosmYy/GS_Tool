/**
 * @file PacketBuilderPanel.cpp
 * @brief 数据包构建面板 UI 实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/packet/PacketBuilderPanel.h"

#include <QFileDialog>
#include <QHeaderView>

/**
 * @brief 构造函数，初始化面板布局
 */
PacketBuilderPanel::PacketBuilderPanel(QWidget *parent)
    : QWidget(parent)
    , m_fieldTable(new QTableWidget(this))
    , m_addFieldBtn(new QPushButton(tr("添加字段"), this))
    , m_removeFieldBtn(new QPushButton(tr("删除字段"), this))
    , m_buildBtn(new QPushButton(tr("构建数据包"), this))
    , m_loadBtn(new QPushButton(tr("加载模板"), this))
    , m_saveBtn(new QPushButton(tr("保存模板"), this))
    , m_clearAllBtn(new QPushButton(tr("清除全部"), this))
    , m_moveUpBtn(new QPushButton(tr("上移"), this))
    , m_moveDownBtn(new QPushButton(tr("下移"), this))
    , m_hexPreview(new QTextEdit(this))
{
    setObjectName(QStringLiteral("PacketBuilderPanel"));

    auto *mainLayout = new QVBoxLayout(this);

    // 按钮行1：字段操作
    auto *fieldBtnLayout = new QHBoxLayout();
    fieldBtnLayout->addWidget(m_addFieldBtn);
    fieldBtnLayout->addWidget(m_removeFieldBtn);
    fieldBtnLayout->addStretch();
    fieldBtnLayout->addWidget(m_buildBtn);

    // 按钮行2：模板操作 + 排序 + 清除
    auto *tmplBtnLayout = new QHBoxLayout();
    tmplBtnLayout->addWidget(m_loadBtn);
    tmplBtnLayout->addWidget(m_saveBtn);
    tmplBtnLayout->addStretch();
    tmplBtnLayout->addWidget(m_moveUpBtn);
    tmplBtnLayout->addWidget(m_moveDownBtn);
    tmplBtnLayout->addWidget(m_clearAllBtn);

    // 字段表格
    m_fieldTable->setColumnCount(5);
    m_fieldTable->setHorizontalHeaderLabels({
        tr("名称"), tr("偏移"), tr("大小"), tr("类型"), tr("值")
    });
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 十六进制预览
    m_hexPreview->setReadOnly(true);
    m_hexPreview->setMaximumHeight(80);
    m_hexPreview->setPlaceholderText(
        tr("构建后的数据包将以十六进制显示..."));

    mainLayout->addLayout(fieldBtnLayout);
    mainLayout->addLayout(tmplBtnLayout);
    mainLayout->addWidget(m_fieldTable);
    mainLayout->addWidget(m_hexPreview);

    // 连接信号
    connect(m_addFieldBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onRemoveField);
    connect(m_buildBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onBuild);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onLoadTemplate);
    connect(m_saveBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onSaveTemplate);
    connect(m_clearAllBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onClearAll);
    connect(m_moveUpBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onMoveUp);
    connect(m_moveDownBtn, &QPushButton::clicked,
            this, &PacketBuilderPanel::onMoveDown);
}

/**
 * @brief 设置关联的构建器实例
 */
void PacketBuilderPanel::setBuilder(PacketBuilder *builder)
{
    m_builder = builder;
    refreshTable();
}

/**
 * @brief 添加新字段行
 */
void PacketBuilderPanel::onAddField()
{
    if (!m_builder) { return; }

    PacketField field;
    field.name = tr("字段%1").arg(m_builder->fields().size() + 1);
    field.offset = 0;
    field.size = 1;
    field.dataType = QStringLiteral("uint8");
    field.value = 0;

    m_builder->addField(field);
    refreshTable();
}

/**
 * @brief 删除选中行对应的字段
 */
void PacketBuilderPanel::onRemoveField()
{
    if (!m_builder) { return; }

    int row = m_fieldTable->currentRow();
    if (row >= 0) {
        m_builder->removeField(row);
        refreshTable();
    }
}

/**
 * @brief 构建数据包并更新预览 — hex dump格式
 */
void PacketBuilderPanel::onBuild()
{
    if (!m_builder) { return; }

    QByteArray packet = m_builder->buildPacket();
    m_hexPreview->setPlainText(formatHexDump(packet));
}

/**
 * @brief 加载模板文件
 */
void PacketBuilderPanel::onLoadTemplate()
{
    if (!m_builder) { return; }

    QString path = QFileDialog::getOpenFileName(
        this, tr("加载模板"), QString(),
        tr("JSON模板 (*.json);;所有文件 (*)"));
    if (path.isEmpty()) { return; }

    if (m_builder->loadTemplate(path)) {
        refreshTable();
        m_hexPreview->setPlainText(tr("模板已加载: %1").arg(path));
    }
}

/**
 * @brief 保存模板文件
 */
void PacketBuilderPanel::onSaveTemplate()
{
    if (!m_builder) { return; }

    QString path = QFileDialog::getSaveFileName(
        this, tr("保存模板"), QString(),
        tr("JSON模板 (*.json);;所有文件 (*)"));
    if (path.isEmpty()) { return; }

    if (m_builder->saveTemplate(path)) {
        m_hexPreview->setPlainText(tr("模板已保存: %1").arg(path));
    }
}

/**
 * @brief 刷新表格以同步构建器字段
 */
void PacketBuilderPanel::refreshTable()
{
    if (!m_builder) { return; }

    auto fields = m_builder->fields();
    m_fieldTable->setRowCount(fields.size());

    for (int i = 0; i < fields.size(); ++i) {
        const auto &f = fields[i];
        m_fieldTable->setItem(i, 0, new QTableWidgetItem(f.name));
        m_fieldTable->setItem(i, 1, new QTableWidgetItem(
            QString::number(f.offset)));
        m_fieldTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(f.size)));
        m_fieldTable->setItem(i, 3, new QTableWidgetItem(f.dataType));
        m_fieldTable->setItem(i, 4, new QTableWidgetItem(
            f.value.toString()));
    }
}

/**
 * @brief 格式化hex dump输出
 */
QString PacketBuilderPanel::formatHexDump(const QByteArray &data) const
{
    if (data.isEmpty()) { return tr("(空数据包)"); }

    QString result;
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) {
            result += (i % 16 == 0) ? "\n" : " ";
        }
        result += QString("%1").arg(
            static_cast<quint8>(data[i]), 2, 16, QChar('0')).toUpper();
    }

    result += QString("\n\n%1 bytes").arg(data.size());
    return result;
}

/**
 * @brief 清除所有字段
 */
void PacketBuilderPanel::onClearAll()
{
    if (!m_builder) { return; }

    auto fields = m_builder->fields();
    for (int i = fields.size() - 1; i >= 0; --i) {
        m_builder->removeField(i);
    }
    refreshTable();
    m_hexPreview->clear();
}

/**
 * @brief 上移选中字段
 */
void PacketBuilderPanel::onMoveUp()
{
    if (!m_builder) { return; }

    int row = m_fieldTable->currentRow();
    if (row <= 0) { return; }

    auto fields = m_builder->fields();
    std::swap(fields[row], fields[row - 1]);

    /* 重建字段列表 */
    for (int i = fields.size() - 1; i >= 0; --i) {
        m_builder->removeField(i);
    }
    for (auto& f : fields) {
        m_builder->addField(f);
    }

    refreshTable();
    m_fieldTable->selectRow(row - 1);
}

/**
 * @brief 下移选中字段
 */
void PacketBuilderPanel::onMoveDown()
{
    if (!m_builder) { return; }

    int row = m_fieldTable->currentRow();
    auto fields = m_builder->fields();
    if (row < 0 || row >= fields.size() - 1) { return; }

    std::swap(fields[row], fields[row + 1]);

    /* 重建字段列表 */
    for (int i = fields.size() - 1; i >= 0; --i) {
        m_builder->removeField(i);
    }
    for (auto& f : fields) {
        m_builder->addField(f);
    }

    refreshTable();
    m_fieldTable->selectRow(row + 1);
}
