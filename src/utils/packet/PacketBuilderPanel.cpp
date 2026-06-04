/**
 * @file PacketBuilderPanel.cpp
 * @brief 数据包构建面板 UI 实现 — 面板构造、模板加载/保存、数据包构建
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

    m_fieldTable->setObjectName("packetFieldTable");
    m_addFieldBtn->setObjectName("packetAddFieldBtn");
    m_removeFieldBtn->setObjectName("packetRemoveFieldBtn");
    m_buildBtn->setObjectName("packetBuildBtn");
    m_loadBtn->setObjectName("packetLoadBtn");
    m_saveBtn->setObjectName("packetSaveBtn");
    m_clearAllBtn->setObjectName("packetClearAllBtn");
    m_moveUpBtn->setObjectName("packetMoveUpBtn");
    m_moveDownBtn->setObjectName("packetMoveDownBtn");
    m_hexPreview->setObjectName("packetHexPreview");

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
 * @brief 构建数据包并更新预览 — hex dump格式
 */
void PacketBuilderPanel::onBuild()
{
    if (!m_builder) { return; }

    ++m_totalPacketsBuilt;
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
        ++m_totalTemplateLoads;
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
