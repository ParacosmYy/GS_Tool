/**
 * @file PacketTemplateLib.cpp
 * @brief 报文模板库控件实现 — UI构建、参数编辑、报文构建、搜索过滤
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/packet_lib/PacketTemplateLib.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "utils/crypto/CRC.h"

// ============================================================
// 构造 / UI
// ============================================================

/**
 * @brief 构造函数 — 初始化界面并加载内置模板
 */
PacketTemplateLib::PacketTemplateLib(QWidget *parent)
    : QWidget(parent)
    , m_splitter(new QSplitter(Qt::Horizontal, this))
    , m_templateTree(new QTreeWidget(this))
    , m_paramTable(new QTableWidget(this))
    , m_previewEdit(new QTextEdit(this))
    , m_sendBtn(new QPushButton(tr("发送"), this))
    , m_copyBtn(new QPushButton(tr("复制Hex"), this))
    , m_categoryCombo(new QComboBox(this))
    , m_searchEdit(new QLineEdit(this))
    , m_statusLabel(new QLabel(this))
{
    setupUI();
    loadBuiltinTemplates();
}

/**
 * @brief 初始化界面布局 — 左侧模板树 + 右侧参数/预览/按钮
 */
void PacketTemplateLib::setupUI()
{
    setObjectName(QStringLiteral("PacketTemplateLib"));

    // ---- 左侧面板 ----
    auto *leftWidget = new QWidget(this);
    leftWidget->setObjectName("pktLibLeftPanel");
    auto *leftLayout = new QVBoxLayout(leftWidget);

    // 搜索栏
    m_searchEdit->setObjectName("pktLibSearchEdit");
    m_searchEdit->setPlaceholderText(tr("搜索模板..."));
    leftLayout->addWidget(m_searchEdit);

    // 分类过滤
    m_categoryCombo->setObjectName("pktLibCategoryCombo");
    m_categoryCombo->addItem(tr("全部分类"), -1);
    m_categoryCombo->addItem(tr("Modbus RTU"), static_cast<int>(TemplateCategory::ModbusRtu));
    m_categoryCombo->addItem(tr("SPI 命令"), static_cast<int>(TemplateCategory::Spi));
    m_categoryCombo->addItem(tr("CAN 帧"), static_cast<int>(TemplateCategory::Can));
    m_categoryCombo->addItem(tr("UART AT"), static_cast<int>(TemplateCategory::UartAt));
    m_categoryCombo->addItem(tr("自定义"), static_cast<int>(TemplateCategory::Custom));
    leftLayout->addWidget(m_categoryCombo);

    // 模板树
    m_templateTree->setObjectName("pktLibTemplateTree");
    m_templateTree->setHeaderLabel(tr("报文模板"));
    m_templateTree->setAlternatingRowColors(true);
    leftLayout->addWidget(m_templateTree);

    // ---- 右侧面板 ----
    auto *rightWidget = new QWidget(this);
    rightWidget->setObjectName("pktLibRightPanel");
    auto *rightLayout = new QVBoxLayout(rightWidget);

    // 参数表格
    m_paramTable->setObjectName("pktLibParamTable");
    m_paramTable->setColumnCount(4);
    m_paramTable->setHorizontalHeaderLabels({
        tr("参数名"), tr("值"), tr("偏移"), tr("长度")
    });
    m_paramTable->horizontalHeader()->setStretchLastSection(true);
    m_paramTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    rightLayout->addWidget(m_paramTable);

    // Hex 预览
    m_previewEdit->setObjectName("pktLibPreviewEdit");
    m_previewEdit->setReadOnly(true);
    m_previewEdit->setMaximumHeight(100);
    m_previewEdit->setPlaceholderText(tr("报文预览（十六进制）..."));
    rightLayout->addWidget(m_previewEdit);

    // 按钮行
    auto *btnLayout = new QHBoxLayout();
    m_sendBtn->setObjectName("pktLibSendBtn");
    m_copyBtn->setObjectName("pktLibCopyBtn");
    btnLayout->addWidget(m_sendBtn);
    btnLayout->addWidget(m_copyBtn);
    btnLayout->addStretch();
    rightLayout->addLayout(btnLayout);

    // 状态标签
    m_statusLabel->setObjectName("pktLibStatusLabel");
    m_statusLabel->setText(tr("就绪"));
    rightLayout->addWidget(m_statusLabel);

    // 分割器
    m_splitter->setObjectName("pktLibSplitter");
    m_splitter->addWidget(leftWidget);
    m_splitter->addWidget(rightWidget);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_splitter);

    // ---- 信号连接 ----
    connect(m_templateTree, &QTreeWidget::itemClicked,
            this, &PacketTemplateLib::onTemplateClicked);
    connect(m_paramTable, &QTableWidget::cellChanged,
            this, &PacketTemplateLib::onParamChanged);
    connect(m_sendBtn, &QPushButton::clicked,
            this, &PacketTemplateLib::onSendClicked);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &PacketTemplateLib::onCopyClicked);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &PacketTemplateLib::onSearchChanged);
    connect(m_categoryCombo, &QComboBox::currentIndexChanged,
            this, &PacketTemplateLib::onCategoryFilterChanged);
}

// ============================================================
// 模板加载 / 树填充
// ============================================================

/**
 * @brief 加载所有内置模板并填充分类树
 */
void PacketTemplateLib::loadBuiltinTemplates()
{
    createBuiltinModbusTemplates();
    createBuiltinSpiTemplates();
    createBuiltinCanTemplates();
    createBuiltinAtTemplates();

    m_stats.totalTemplatesLoaded = static_cast<quint64>(m_templates.size());

    // 计算单分类峰值
    QMap<TemplateCategory, int> catCount;
    for (const auto &t : m_templates) {
        catCount[t.category]++;
    }
    for (auto it = catCount.constBegin(); it != catCount.constEnd(); ++it) {
        m_stats.peakTemplatesPerCategory = qMax(m_stats.peakTemplatesPerCategory, it.value());
    }

    populateTree();
}

/**
 * @brief 填充模板分类树 — 分类为顶层节点，模板为子节点
 */
void PacketTemplateLib::populateTree()
{
    m_templateTree->clear();

    QMap<TemplateCategory, QString> catNames = {
        {TemplateCategory::ModbusRtu, tr("Modbus RTU")},
        {TemplateCategory::Spi,       tr("SPI 命令")},
        {TemplateCategory::Can,       tr("CAN 帧")},
        {TemplateCategory::UartAt,    tr("UART AT")},
        {TemplateCategory::Custom,    tr("自定义")}
    };

    QMap<TemplateCategory, QTreeWidgetItem *> catItems;
    for (auto it = catNames.constBegin(); it != catNames.constEnd(); ++it) {
        auto *catItem = new QTreeWidgetItem(m_templateTree, {it.value()});
        catItem->setData(0, Qt::UserRole, static_cast<int>(it.key()));
        catItem->setExpanded(true);
        catItems.insert(it.key(), catItem);
    }

    for (int i = 0; i < m_templates.size(); ++i) {
        const auto &tmpl = m_templates.at(i);
        auto *parentItem = catItems.value(tmpl.category, nullptr);
        if (!parentItem) { continue; }

        auto *item = new QTreeWidgetItem(parentItem, {tmpl.name});
        item->setData(0, Qt::UserRole, i);
        item->setToolTip(0, tmpl.description);
    }
    m_templateTree->expandAll();
}

// ============================================================
// 模板点击 / 参数加载
// ============================================================

/** @brief 模板树节点被点击 — 加载参数并更新预览 */
void PacketTemplateLib::onTemplateClicked()
{
    auto *current = m_templateTree->currentItem();
    if (!current) { return; }

    bool ok = false;
    int idx = current->data(0, Qt::UserRole).toInt(&ok);
    if (!ok || idx < 0 || idx >= m_templates.size()) { return; }

    m_currentTemplateIndex = idx;
    const auto &tmpl = m_templates.at(idx);
    loadTemplateParams(tmpl);

    emit templateSelected(tmpl.name);
    m_statusLabel->setText(tr("已选择: %1").arg(tmpl.name));
}

/**
 * @brief 将模板参数加载到参数编辑表格
 */
void PacketTemplateLib::loadTemplateParams(const PacketTemplate &tmpl)
{
    m_paramTable->blockSignals(true);
    m_paramTable->setRowCount(tmpl.params.size());

    for (int i = 0; i < tmpl.params.size(); ++i) {
        const auto &p = tmpl.params.at(i);

        auto *nameItem = new QTableWidgetItem(p.name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_paramTable->setItem(i, 0, nameItem);

        m_paramTable->setItem(i, 1, new QTableWidgetItem(p.value));

        auto *offsetItem = new QTableWidgetItem(QString::number(p.byteOffset));
        offsetItem->setFlags(offsetItem->flags() & ~Qt::ItemIsEditable);
        m_paramTable->setItem(i, 2, offsetItem);

        auto *lenItem = new QTableWidgetItem(QString::number(p.byteLength));
        lenItem->setFlags(lenItem->flags() & ~Qt::ItemIsEditable);
        m_paramTable->setItem(i, 3, lenItem);
    }
    m_paramTable->blockSignals(false);
    updatePreview();
}

// ============================================================
// 报文构建 / 校验和
// ============================================================

/**
 * @brief 根据当前参数构建报文，自动计算校验和
 * @return 构建后的字节数组（空数组表示未选中模板）
 */
QByteArray PacketTemplateLib::buildPacket()
{
    if (m_currentTemplateIndex < 0 ||
        m_currentTemplateIndex >= m_templates.size()) {
        return {};
    }

    const auto &tmpl = m_templates.at(m_currentTemplateIndex);
    QByteArray frame = tmpl.frameTemplate;

    // 将参数值写入帧模板对应偏移位置
    for (int i = 0; i < tmpl.params.size() && i < m_paramTable->rowCount(); ++i) {
        const auto &p = tmpl.params.at(i);
        QString valStr = m_paramTable->item(i, 1) ? m_paramTable->item(i, 1)->text() : p.value;

        QByteArray valBytes = p.isHex
            ? QByteArray::fromHex(valStr.toUtf8())
            : valStr.toUtf8();

        for (int b = 0; b < p.byteLength && b < valBytes.size(); ++b) {
            int pos = p.byteOffset + b;
            if (pos >= 0 && pos < frame.size()) {
                frame[pos] = valBytes[b];
            }
        }
    }

    // 计算并写入校验和
    if (tmpl.hasChecksum && tmpl.category == TemplateCategory::ModbusRtu) {
        uint16_t crc = CRC::crc16Modbus(
            reinterpret_cast<const uint8_t*>(frame.constData()),
            frame.size() - 2);
        frame[frame.size() - 2] = static_cast<char>(crc & 0xFF);
        frame[frame.size() - 1] = static_cast<char>((crc >> 8) & 0xFF);
    } else if (tmpl.hasChecksum) {
        uint8_t sum = CRC::checksum(
            reinterpret_cast<const uint8_t*>(frame.constData()),
            frame.size() - 1);
        frame[frame.size() - 1] = static_cast<char>(sum);
    }

    return frame;
}

/**
 * @brief 计算校验和 — 根据当前模板的协议类型选择算法
 * @param data 报文数据
 * @param offset 预留偏移参数（当前未使用）
 * @return 校验和字节数组
 */
QByteArray PacketTemplateLib::calculateChecksum(const QByteArray &data, int offset) const
{
    Q_UNUSED(offset)
    if (data.isEmpty()) { return {}; }
    if (m_currentTemplateIndex < 0 ||
        m_currentTemplateIndex >= m_templates.size()) {
        return {};
    }

    const auto &tmpl = m_templates.at(m_currentTemplateIndex);
    if (tmpl.category == TemplateCategory::ModbusRtu) {
        uint16_t crc = CRC::crc16Modbus(data);
        return QByteArray(1, static_cast<char>(crc & 0xFF))
               + QByteArray(1, static_cast<char>((crc >> 8) & 0xFF));
    }

    uint8_t sum = CRC::checksum(data);
    return QByteArray(1, static_cast<char>(sum));
}

// ============================================================
// 槽函数
// ============================================================

/** @brief 参数被编辑 — 递增编辑计数并刷新预览 */
void PacketTemplateLib::onParamChanged()
{
    ++m_stats.totalEdits;
    updatePreview();
}

/** @brief 刷新 Hex 预览区 — 空格分隔的大写十六进制 */
void PacketTemplateLib::updatePreview()
{
    QByteArray packet = buildPacket();
    if (packet.isEmpty()) {
        m_previewEdit->clear();
        return;
    }
    QStringList hexParts;
    for (char b : packet) {
        hexParts << QString("%1").arg(static_cast<uint8_t>(b), 2, 16, QChar('0')).toUpper();
    }
    m_previewEdit->setPlainText(hexParts.join(' '));
}

/** @brief 发送按钮 — 构建报文并发出 packetSendRequested 信号 */
void PacketTemplateLib::onSendClicked()
{
    QByteArray packet = buildPacket();
    if (packet.isEmpty()) {
        m_statusLabel->setText(tr("错误: 未选择模板或报文为空"));
        return;
    }
    ++m_stats.totalSends;
    m_stats.totalBytesSent += static_cast<quint64>(packet.size());
    emit packetSendRequested(packet);
    m_statusLabel->setText(tr("已发送 %1 字节").arg(packet.size()));
}

/** @brief 复制 Hex 到剪贴板 */
void PacketTemplateLib::onCopyClicked()
{
    QString hex = m_previewEdit->toPlainText();
    if (hex.isEmpty()) { return; }
    QApplication::clipboard()->setText(hex);
    m_statusLabel->setText(tr("已复制到剪贴板"));
}

/** @brief 搜索过滤 — 按关键词隐藏/显示模板树节点 */
void PacketTemplateLib::onSearchChanged()
{
    QString keyword = m_searchEdit->text().trimmed().toLower();
    for (int i = 0; i < m_templateTree->topLevelItemCount(); ++i) {
        auto *catItem = m_templateTree->topLevelItem(i);
        bool catVisible = false;
        for (int j = 0; j < catItem->childCount(); ++j) {
            auto *child = catItem->child(j);
            bool match = keyword.isEmpty()
                         || child->text(0).toLower().contains(keyword);
            child->setHidden(!match);
            if (match) { catVisible = true; }
        }
        catItem->setHidden(!catVisible);
    }
}

/** @brief 分类过滤切换 — 隐藏不匹配的分类 */
void PacketTemplateLib::onCategoryFilterChanged()
{
    int filterCat = m_categoryCombo->currentData().toInt();
    for (int i = 0; i < m_templateTree->topLevelItemCount(); ++i) {
        auto *catItem = m_templateTree->topLevelItem(i);
        int cat = catItem->data(0, Qt::UserRole).toInt();
        catItem->setHidden(filterCat >= 0 && cat != filterCat);
    }
}
