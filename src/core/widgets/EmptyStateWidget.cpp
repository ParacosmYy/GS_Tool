/**
 * @file EmptyStateWidget.cpp
 * @brief 空状态组件实现 — 居中排列图标+标题+描述+操作按钮
 *
 * 实现细节:
 *   - 图标占位 48x48, 标题 bold 14px, 描述 12px
 *   - 最大宽度 320px, 文字自动换行
 *   - 操作按钮可选, 默认隐藏
 *   - 所有颜色通过 ThemeManager 语义色获取
 */

#include "core/widgets/EmptyStateWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造空状态组件 @param title 标题文字 @param description 描述文字 @param parent 父控件指针 */
EmptyStateWidget::EmptyStateWidget(const QString& title,
                                     const QString& description,
                                     QWidget* parent)
    : QWidget(parent)
{
    setObjectName("emptyStateWidget");
    setupUI(title, description);
}

// ============================================================================
// 公开接口
// ============================================================================

/** @brief 设置图标名称，显示首字符作为占位 @param name 图标名称字符串 */
void EmptyStateWidget::setIconName(const QString& name)
{
    if (m_iconLabel) {
        // 占位: 显示首字符，后续接入IconManager替换
        m_iconLabel->setText(name.isEmpty() ? QString() : QString(name.at(0)));
        ++m_totalIconChanges;
    }
}

/** @brief 设置标题文字 @param title 新的标题文字 */
void EmptyStateWidget::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
        ++m_totalStateChanges;
    }
}

/** @brief 设置描述文字，为空时自动隐藏 @param description 描述文字内容 */
void EmptyStateWidget::setDescription(const QString& description)
{
    if (m_descLabel) {
        m_descLabel->setText(description);
        m_descLabel->setVisible(!description.isEmpty());
        ++m_totalStateChanges;
    }
}

/** @brief 设置操作按钮的文字与回调，文字为空时隐藏按钮 @param text 按钮文字 @param callback 点击回调函数 */
void EmptyStateWidget::setActionButton(const QString& text,
                                         std::function<void()> callback)
{
    if (!m_actionBtn) return;

    if (text.isEmpty()) {
        m_actionBtn->hide();
        return;
    }

    m_actionBtn->setText(text);
    m_actionBtn->show();

    // 断开之前的所有连接，避免重复
    m_actionBtn->disconnect();
    if (callback) {
        connect(m_actionBtn, &QPushButton::clicked, this, [cb = std::move(callback)]() {
            cb();
        });
    }
}

// ============================================================================
// 私有方法
// ============================================================================

/** @brief 初始化UI布局，创建图标/标题/描述/按钮控件 @param title 标题文字 @param description 描述文字 */
void EmptyStateWidget::setupUI(const QString& title, const QString& description)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(16, 16, 16, 16);

    // 图标占位 (48x48)
    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("emptyStateIcon");
    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    QFont iconFont = m_iconLabel->font();
    iconFont.setPointSize(24);
    m_iconLabel->setFont(iconFont);
    m_iconLabel->hide(); // 默认隐藏，setIconName后显示
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);

    // 标题 (bold 14px)
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName("emptyStateTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumWidth(320);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    m_titleLabel->setFont(titleFont);
    m_mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

    // 描述 (12px, 次要文字色)
    m_descLabel = new QLabel(description, this);
    m_descLabel->setObjectName("emptyStateDescription");
    m_descLabel->setAlignment(Qt::AlignCenter);
    m_descLabel->setWordWrap(true);
    m_descLabel->setMaximumWidth(320);
    QFont descFont = m_descLabel->font();
    descFont.setPointSize(12);
    m_descLabel->setFont(descFont);
    m_descLabel->setVisible(!description.isEmpty());
    m_mainLayout->addWidget(m_descLabel, 0, Qt::AlignCenter);

    // 操作按钮 (可选)
    m_actionBtn = new QPushButton(this);
    m_actionBtn->setObjectName("emptyStateAction");
    m_actionBtn->setFlat(true);
    m_actionBtn->setMaximumWidth(320);
    m_actionBtn->hide(); // 默认隐藏，setActionButton后显示
    m_mainLayout->addWidget(m_actionBtn, 0, Qt::AlignCenter);
}

// ============================================================================
// 统计重置
// ============================================================================

/** @brief 重置空状态组件的统计计数器(状态变更次数和图标变更次数) */
void EmptyStateWidget::resetEmptyStateStatistics()
{
    m_totalStateChanges = 0;
    m_totalIconChanges = 0;
}
