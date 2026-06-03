/**
 * @file FrameVisualEditorUI.cpp
 * @brief 帧格式可视化编辑器 — UI构建方法
 *
 * 将UI构建方法从FrameVisualEditor.cpp拆分而来，职责:
 *   - setupUI(): 主布局初始化
 *   - setupHeaderGroup(): 帧头/帧尾配置分组
 *   - setupLengthGroup(): 长度字段配置分组
 *   - setupChecksumGroup(): 校验配置分组
 *   - setupFieldsGroup(): 数据字段表格分组(含拖拽排序按钮)
 *   - setupPreviewGroup(): 二进制布局预览分组
 *   - setupConnections(): 信号/槽连接
 */
#include "protocol/editor/FrameVisualEditor.h"
#include "core/widgets/AnimatedButton.h"
#include "core/theme/Constants.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>

// ============================================================
// UI构建方法
// ============================================================

/**
 * @brief 初始化UI控件和布局
 *
 * 结构:
 *   上半部分(水平): 帧头帧尾 | 长度字段 | 校验
 *   下半部分(垂直): 字段表格(可伸展) | 预览 | 应用按钮
 *
 * 三个配置组水平排列，节省垂直空间，避免控件挤到一起。
 */
void FrameVisualEditor::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ---- 上半部分: 三个配置组水平排列 ----
    auto* configLayout = new QHBoxLayout;
    configLayout->setSpacing(8);
    configLayout->addWidget(setupHeaderGroup());
    configLayout->addWidget(setupLengthGroup());
    configLayout->addWidget(setupChecksumGroup());
    mainLayout->addLayout(configLayout);

    // ---- 下半部分: 字段表格(可伸展) + 预览 + 应用 ----
    mainLayout->addWidget(setupFieldsGroup(), 1);
    mainLayout->addWidget(setupPreviewGroup());

    // ---- 应用按钮 ----
    m_applyBtn = new AnimatedButton(tr("应用定义"));
    m_applyBtn->setObjectName("applyDefBtn");
    m_applyBtn->setMinimumHeight(Layout::kInputHeight);
    mainLayout->addWidget(m_applyBtn);

    setupConnections();
}

/**
 * @brief 创建帧头/帧尾配置分组
 * @return 帧头/帧尾GroupBox(包含帧头和帧尾HEX输入框)
 */
QGroupBox* FrameVisualEditor::setupHeaderGroup()
{
    auto* group = new QGroupBox(tr("帧头/帧尾配置"));
    group->setObjectName("frameHeaderGroup");
    auto* layout = new QFormLayout(group);
    m_headerEdit = new QLineEdit;
    m_headerEdit->setObjectName("frameHeaderEdit");
    m_headerEdit->setPlaceholderText(tr("AA 55"));
    m_headerEdit->setToolTip(tr("帧头HEX字节，如 AA 55"));
    layout->addRow(tr("帧头:"), m_headerEdit);
    m_footerEdit = new QLineEdit;
    m_footerEdit->setObjectName("frameFooterEdit");
    m_footerEdit->setPlaceholderText(tr("0D 0A"));
    m_footerEdit->setToolTip(tr("帧尾HEX字节（可选）"));
    layout->addRow(tr("帧尾:"), m_footerEdit);
    return group;
}

/**
 * @brief 创建长度字段配置分组
 * @return 长度字段GroupBox(包含偏移/大小/字节序/调整值)
 */
QGroupBox* FrameVisualEditor::setupLengthGroup()
{
    auto* group = new QGroupBox(tr("长度字段"));
    group->setObjectName("frameLengthGroup");
    auto* layout = new QFormLayout(group);
    m_lengthOffsetSpin = new QSpinBox;
    m_lengthOffsetSpin->setObjectName("frameLengthOffsetSpin");
    m_lengthOffsetSpin->setRange(-1, 255);
    m_lengthOffsetSpin->setValue(-1);
    m_lengthOffsetSpin->setSpecialValueText(tr("无"));
    layout->addRow(tr("偏移:"), m_lengthOffsetSpin);
    m_lengthSizeCombo = new QComboBox;
    m_lengthSizeCombo->setObjectName("frameLengthSizeCombo");
    m_lengthSizeCombo->addItems({tr("1 byte"), tr("2 bytes")});
    layout->addRow(tr("大小:"), m_lengthSizeCombo);
    m_lengthBEndianCheck = new QCheckBox(tr("大端序"));
    m_lengthBEndianCheck->setObjectName("frameLengthEndianCheck");
    layout->addRow(m_lengthBEndianCheck);
    m_lengthAdjustSpin = new QSpinBox;
    m_lengthAdjustSpin->setObjectName("frameLengthAdjustSpin");
    m_lengthAdjustSpin->setRange(-256, 256);
    m_lengthAdjustSpin->setValue(0);
    m_lengthAdjustSpin->setToolTip(tr("实际负载 = 长度字段值 - 调整值"));
    layout->addRow(tr("调整:"), m_lengthAdjustSpin);
    return group;
}

/**
 * @brief 创建校验配置分组
 * @return 校验GroupBox(包含类型/偏移/起始偏移)
 */
QGroupBox* FrameVisualEditor::setupChecksumGroup()
{
    auto* group = new QGroupBox(tr("校验配置"));
    group->setObjectName("frameChecksumGroup");
    auto* layout = new QFormLayout(group);
    m_checksumTypeCombo = new QComboBox;
    m_checksumTypeCombo->setObjectName("frameChecksumTypeCombo");
    m_checksumTypeCombo->addItems({tr("无校验"), tr("Sum8"), tr("CRC8"), tr("CRC16-CCITT"), tr("CRC16-Modbus"), tr("CRC32")});
    layout->addRow(tr("类型:"), m_checksumTypeCombo);
    m_checksumOffsetSpin = new QSpinBox;
    m_checksumOffsetSpin->setObjectName("frameChecksumOffsetSpin");
    m_checksumOffsetSpin->setRange(-1, 255);
    m_checksumOffsetSpin->setValue(-1);
    m_checksumOffsetSpin->setSpecialValueText(tr("自动"));
    layout->addRow(tr("偏移:"), m_checksumOffsetSpin);
    m_checksumStartSpin = new QSpinBox;
    m_checksumStartSpin->setObjectName("frameChecksumStartSpin");
    m_checksumStartSpin->setRange(0, 255);
    m_checksumStartSpin->setValue(0);
    layout->addRow(tr("起始:"), m_checksumStartSpin);
    return group;
}

/**
 * @brief 创建数据字段表格分组(含拖拽排序和上移/下移按钮)
 * @return 字段GroupBox(包含6列表格: 名称/类型/偏移/大小/字节序/缩放)
 */
QGroupBox* FrameVisualEditor::setupFieldsGroup()
{
    auto* group = new QGroupBox(tr("数据字段"));
    group->setObjectName("frameFieldGroup");
    auto* layout = new QVBoxLayout(group);

    // 字段定义表(支持拖拽排序)
    m_fieldTable = new QTableWidget(0, 6);
    m_fieldTable->setObjectName("frameFieldTable");
    m_fieldTable->setHorizontalHeaderLabels({
        tr("名称"), tr("类型"), tr("偏移"), tr("大小"), tr("字节序"), tr("缩放")
    });
    m_fieldTable->horizontalHeader()->setStretchLastSection(true);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_fieldTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_fieldTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fieldTable->setMinimumHeight(OtaLayout::kFieldTableMinHeight);
    // 启用拖拽排序(行级拖放)
    m_fieldTable->setDragEnabled(true);
    m_fieldTable->setAcceptDrops(true);
    m_fieldTable->setDragDropMode(QAbstractItemView::InternalMove);
    m_fieldTable->setDefaultDropAction(Qt::MoveAction);
    m_fieldTable->setDragDropOverwriteMode(false);
    layout->addWidget(m_fieldTable);

    // 字段操作按钮行
    auto* btnLayout = new QHBoxLayout;
    m_addFieldBtn = new AnimatedButton(tr("添加字段"));
    m_addFieldBtn->setObjectName("frameAddFieldBtn");
    m_removeFieldBtn = new AnimatedButton(tr("删除字段"));
    m_removeFieldBtn->setObjectName("frameRemoveFieldBtn");
    auto* moveUpBtn = new AnimatedButton(tr("上移"));
    moveUpBtn->setObjectName("frameMoveUpBtn");
    moveUpBtn->setFixedWidth(OtaLayout::kTableBtnWidth);
    auto* moveDownBtn = new AnimatedButton(tr("下移"));
    moveDownBtn->setObjectName("frameMoveDownBtn");
    moveDownBtn->setFixedWidth(OtaLayout::kTableBtnWidth);
    btnLayout->addWidget(m_addFieldBtn);
    btnLayout->addWidget(m_removeFieldBtn);
    btnLayout->addWidget(moveUpBtn);
    btnLayout->addWidget(moveDownBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(moveUpBtn, &QPushButton::clicked, this, &FrameVisualEditor::onMoveFieldUp);
    connect(moveDownBtn, &QPushButton::clicked, this, &FrameVisualEditor::onMoveFieldDown);

    return group;
}

/**
 * @brief 创建二进制布局预览分组
 * @return 预览GroupBox(包含Consolas字体的预览标签)
 */
QGroupBox* FrameVisualEditor::setupPreviewGroup()
{
    auto* group = new QGroupBox(tr("二进制布局预览"));
    group->setObjectName("framePreviewGroup");
    auto* layout = new QVBoxLayout(group);
    m_previewLabel = new QLabel(tr("点击\"应用定义\"后显示布局"));
    m_previewLabel->setObjectName("framePreviewLabel");
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_previewLabel->setMinimumHeight(OtaLayout::kPreviewMinHeight);
    m_previewLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_previewLabel->setFont(QFont(TerminalDefaults::kFontFamily, TerminalDefaults::kFontSize));
    layout->addWidget(m_previewLabel);
    return group;
}

/**
 * @brief 连接所有信号/槽(应用/添加/删除/字段变更/实时预览)
 */
void FrameVisualEditor::setupConnections()
{
    connect(m_applyBtn, &QPushButton::clicked, this, &FrameVisualEditor::onApply);
    connect(m_addFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onAddField);
    connect(m_removeFieldBtn, &QPushButton::clicked, this, &FrameVisualEditor::onRemoveField);
    connect(m_fieldTable, &QTableWidget::cellChanged, this, &FrameVisualEditor::onFieldChanged);

    /* 帧头/帧尾实时预览 */
    connect(m_headerEdit, &QLineEdit::textChanged,
            this, &FrameVisualEditor::onHeaderChanged);
    connect(m_footerEdit, &QLineEdit::textChanged,
            this, &FrameVisualEditor::onFooterChanged);

    /* 长度字段配置变更 → 实时预览 */
    connect(m_lengthOffsetSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onLengthConfigChanged);
    connect(m_lengthSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FrameVisualEditor::onLengthConfigChanged);
    connect(m_lengthBEndianCheck, &QCheckBox::checkStateChanged,
            this, &FrameVisualEditor::onLengthConfigChanged);
    connect(m_lengthAdjustSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onLengthConfigChanged);

    /* 校验配置变更 → 实时预览 */
    connect(m_checksumTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FrameVisualEditor::onChecksumConfigChanged);
    connect(m_checksumOffsetSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onChecksumConfigChanged);
    connect(m_checksumStartSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FrameVisualEditor::onChecksumConfigChanged);
}
