/**
 * @file CanBusMonitor.cpp
 * @brief CAN总线监控面板实现 — 表格显示、颜色编码、自动滚动、信号解码、统计面板
 *
 * 支持帧列表显示(含过滤)、DBC信号解码面板、帧类型统计面板。
 * 颜色编码: 标准帧白色、扩展帧浅蓝、RTR帧黄色、CAN-FD帧浅绿、错误帧红色。
 */

#include "connection/can/CanBusMonitor.h"
#include "protocol/can/DbcParser.h"
#include "core/theme/ThemeManager.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTime>
#include <QBrush>
#include <QColor>

/** @brief 构造CAN总线监控面板，初始化表格/信号面板/统计面板 @param parent 父控件 */
CanBusMonitor::CanBusMonitor(QWidget* parent)
    : QWidget(parent)
    , m_frameTable(new QTableWidget(this))
    , m_signalTable(new QTableWidget(this))
    , m_countLabel(new QLabel(tr("帧数: 0"), this))
    , m_clearBtn(new QPushButton(tr("清空"), this))
    , m_autoScrollCheck(new QCheckBox(tr("自动滚动"), this))
    , m_filterEdit(new QLineEdit(this))
    , m_typeFilterCombo(new QComboBox(this))
    , m_statsLabel(new QLabel(this))
{
    setObjectName("CanBusMonitor");

    /* ── 帧列表表格 ── */
    m_frameTable->setObjectName("canFrameTable");
    m_frameTable->setColumnCount(7);
    m_frameTable->setHorizontalHeaderLabels({
        tr("时间"), tr("帧ID"), tr("DLC"), tr("数据"),
        tr("扩展帧"), tr("RTR"), tr("计数")
    });
    m_frameTable->horizontalHeader()->setStretchLastSection(true);
    m_frameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_frameTable->setAlternatingRowColors(true);

    /* ── 信号解码面板 ── */
    m_signalTable->setObjectName("canSignalTable");
    m_signalTable->setColumnCount(4);
    m_signalTable->setHorizontalHeaderLabels({
        tr("信号名"), tr("物理值"), tr("单位"), tr("描述")
    });
    m_signalTable->horizontalHeader()->setStretchLastSection(true);
    m_signalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_signalTable->setMaximumHeight(200);

    /* ── 工具栏 ── */
    m_countLabel->setObjectName("canCountLabel");
    m_clearBtn->setObjectName("canClearBtn");
    m_autoScrollCheck->setObjectName("canAutoScrollCheck");
    m_autoScrollCheck->setChecked(true);

    m_filterEdit->setObjectName("canFilterEdit");
    m_filterEdit->setPlaceholderText(tr("帧ID过滤(如 0x123)"));
    m_filterEdit->setMaximumWidth(160);

    m_typeFilterCombo->setObjectName("canTypeFilterCombo");
    m_typeFilterCombo->addItem(tr("全部"), QVariant::fromValue(0));
    m_typeFilterCombo->addItem(tr("标准帧"), QVariant::fromValue(1));
    m_typeFilterCombo->addItem(tr("扩展帧"), QVariant::fromValue(2));
    m_typeFilterCombo->addItem(tr("RTR帧"), QVariant::fromValue(3));
    m_typeFilterCombo->addItem(tr("CAN-FD帧"), QVariant::fromValue(4));
    m_typeFilterCombo->setMaximumWidth(100);

    auto toolbar = new QHBoxLayout();
    toolbar->addWidget(m_countLabel, 1);
    toolbar->addWidget(m_filterEdit);
    toolbar->addWidget(m_typeFilterCombo);
    toolbar->addWidget(m_autoScrollCheck);
    toolbar->addWidget(m_clearBtn);

    /* ── 统计面板 ── */
    m_statsLabel->setObjectName("canStatsLabel");
    m_statsLabel->setWordWrap(true);
    m_statsLabel->setFrameShape(QFrame::StyledPanel);

    /* ── 使用分割器布局 ── */
    auto splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("canMonitorSplitter");
    splitter->addWidget(m_frameTable);
    splitter->addWidget(m_signalTable);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(toolbar);
    layout->addWidget(splitter, 1);
    layout->addWidget(m_statsLabel);

    m_rateTimer.start();

    /* ── 信号连接 ── */
    connect(m_clearBtn, &QPushButton::clicked, this, &CanBusMonitor::clearFrames);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &CanBusMonitor::setFrameIdFilter);

    /* 帧类型过滤: 联合ID过滤 */
    connect(m_typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { updateStatsDisplay(); });
}

/** @brief 获取当前帧总数 @return 已记录的帧数量 */
int CanBusMonitor::frameCount() const
{
    return m_frameCount;
}

/** @brief 获取唯一帧ID数量 @return 不同帧ID的数量 */
int CanBusMonitor::uniqueFrameIdCount() const
{
    return m_idFrequency.size();
}

/** @brief 设置帧ID过滤器 @param filterText 帧ID过滤文本(如"0x123")，空字符串清除过滤 */
void CanBusMonitor::setFrameIdFilter(const QString& filterText)
{
    m_frameIdFilter = filterText.trimmed();
}

/** @brief 设置DBC解析器用于信号解码显示 @param parser DBC解析器指针 */
void CanBusMonitor::setDbcParser(DbcParser* parser)
{
    m_dbcParser = parser;
}

/** @brief 获取累计监控帧总数 */
quint64 CanBusMonitor::totalFramesMonitored() const
{
    return m_totalFramesMonitored;
}

/** @brief 获取累计错误次数 */
quint64 CanBusMonitor::totalErrors() const
{
    return m_totalErrors;
}
