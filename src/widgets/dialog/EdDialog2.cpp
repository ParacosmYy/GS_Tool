/**
 * @file EdDialog2.cpp
 * @brief 通用对话框v2实现 — 自定义标题/消息/按钮/图标/记住选项
 *
 * 完整实现: 标准按钮创建/自定义按钮添加/图标显示/记住选择复选框。
 * 替代QMessageBox，统一应用内对话框风格。
 */
#include "widgets/dialog/EdDialog2.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

/** @brief 标准按钮文本映射表 */
static QString standardButtonText(EdDialog::StandardButton btn)
{
    switch (btn) {
    case EdDialog::Ok:     return QStringLiteral("OK");
    case EdDialog::Cancel: return QStringLiteral("Cancel");
    case EdDialog::Yes:    return QStringLiteral("Yes");
    case EdDialog::No:     return QStringLiteral("No");
    case EdDialog::Apply:  return QStringLiteral("Apply");
    case EdDialog::Close:  return QStringLiteral("Close");
    }
    return QStringLiteral("OK");
}

/** @brief 标准按钮到QDialogButtonBox按钮类型的映射 */
static QDialogButtonBox::StandardButton toQDialogButton(EdDialog::StandardButton btn)
{
    switch (btn) {
    case EdDialog::Ok:     return QDialogButtonBox::Ok;
    case EdDialog::Cancel: return QDialogButtonBox::Cancel;
    case EdDialog::Yes:    return QDialogButtonBox::Yes;
    case EdDialog::No:     return QDialogButtonBox::No;
    case EdDialog::Apply:  return QDialogButtonBox::Apply;
    case EdDialog::Close:  return QDialogButtonBox::Close;
    }
    return QDialogButtonBox::Ok;
}

/** @brief 构造对话框，初始化布局和控件 @param parent 父Widget */
EdDialog::EdDialog(QWidget *parent)
    : QDialog(parent)
    , m_iconLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_messageLabel(nullptr)
    , m_buttonBox(nullptr)
    , m_rememberCheck(nullptr)
{
    setObjectName("EdDialog2");
    ++s_totalOpens;
    setupUi();
}

/** @brief 析构函数 */
EdDialog::~EdDialog() = default;

/** @brief 初始化UI布局 — 顶部图标+标题/消息/按钮/记住选择 */
void EdDialog::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(12);

    /* 顶部区域: 图标 + 标题 */
    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(12);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("edDialogIcon");
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->hide();
    topLayout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("edDialogTitle");
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    m_titleLabel->setFont(titleFont);
    topLayout->addWidget(m_titleLabel, 1);

    mainLayout->addLayout(topLayout);

    /* 消息区域 */
    m_messageLabel = new QLabel(this);
    m_messageLabel->setObjectName("edDialogMessage");
    m_messageLabel->setWordWrap(true);
    mainLayout->addWidget(m_messageLabel);

    /* 内容区域占位 */
    m_contentArea = new QVBoxLayout();
    mainLayout->addLayout(m_contentArea);

    /* 按钮区域 */
    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setObjectName("edDialogButtonBox");
    mainLayout->addWidget(m_buttonBox);

    /* 连接按钮盒信号 */
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* btn) {
        ++s_totalButtonPresses;
        /* 查找按钮角色 */
        QDialogButtonBox::ButtonRole role = m_buttonBox->buttonRole(btn);
        /* 检查是否为自定义按钮 */
        if (m_customButtons.contains(btn)) {
            m_resultRole = m_customButtons.value(btn);
        } else {
            m_resultRole = static_cast<int>(role);
        }
        emit buttonClicked(m_resultRole);
    });
}

/** @brief 设置对话框标题 @param t 标题文本 */
void EdDialog::setTitle(const QString &t)
{
    if (m_titleLabel) {
        m_titleLabel->setText(t);
    }
    setWindowTitle(t);
}

/** @brief 设置对话框消息内容 @param m 消息文本 */
void EdDialog::setMessage(const QString &m)
{
    if (m_messageLabel) {
        m_messageLabel->setText(m);
    }
}

/** @brief 添加标准按钮到按钮盒 @param b 标准按钮类型 */
void EdDialog::addButton(StandardButton b)
{
    if (!m_buttonBox) return;
    m_buttonBox->addButton(toQDialogButton(b));
}

/** @brief 添加自定义按钮到按钮盒 @param label 按钮文本 @param role 按钮角色标识 */
void EdDialog::addCustomButton(const QString &label, int role)
{
    if (!m_buttonBox) return;
    QPushButton* btn = m_buttonBox->addButton(label, QDialogButtonBox::AcceptRole);
    m_customButtons.insert(btn, role);
    m_resultRole = role;
}

/** @brief 设置对话框图标(通过IconManager名称或颜色圆点) @param name 图标名称 */
void EdDialog::setIcon(const QString &name)
{
    if (!m_iconLabel || name.isEmpty()) return;

    /* 根据名称生成不同颜色的图标圆点 */
    QColor iconColor;
    if (name.contains("warning", Qt::CaseInsensitive) || name.contains("warn", Qt::CaseInsensitive)) {
        iconColor = QColor(245, 158, 11);
    } else if (name.contains("error", Qt::CaseInsensitive) || name.contains("danger", Qt::CaseInsensitive)) {
        iconColor = QColor(239, 68, 68);
    } else if (name.contains("info", Qt::CaseInsensitive) || name.contains("information", Qt::CaseInsensitive)) {
        iconColor = QColor(59, 130, 246);
    } else if (name.contains("confirm", Qt::CaseInsensitive) || name.contains("success", Qt::CaseInsensitive)) {
        iconColor = QColor(34, 197, 94);
    } else if (name.contains("question", Qt::CaseInsensitive) || name.contains("help", Qt::CaseInsensitive)) {
        iconColor = QColor(168, 85, 247);
    } else {
        /* 默认信息蓝 */
        iconColor = QColor(59, 130, 246);
    }

    /* 绘制圆角图标 */
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(iconColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(2, 2, 28, 28, 6, 6);

    /* 绘制图标符号(i/!/✓/?) */
    painter.setPen(Qt::white);
    QFont symbolFont("Arial", 16, QFont::Bold);
    painter.setFont(symbolFont);
    QString symbol;
    if (name.contains("warning") || name.contains("warn")) symbol = "!";
    else if (name.contains("error") || name.contains("danger")) symbol = "X";
    else if (name.contains("confirm") || name.contains("success")) symbol = QString(QChar(0x2713));
    else if (name.contains("question") || name.contains("help")) symbol = "?";
    else symbol = "i";
    painter.drawText(pixmap.rect(), Qt::AlignCenter, symbol);

    m_iconLabel->setPixmap(pixmap);
    m_iconLabel->show();
}

/** @brief 设置自定义内容Widget(嵌入到消息区域下方) @param w 内容Widget指针 */
void EdDialog::setContentWidget(QWidget *w)
{
    if (m_contentArea && w) {
        m_contentArea->addWidget(w);
    }
}

/** @brief 获取对话框结果角色 @return 角色值 */
int EdDialog::resultRole() const
{
    return m_resultRole;
}

/** @brief 设置"记住选择"复选框 @param key 持久化键名 @param label 显示文本 */
void EdDialog::setRememberOption(const QString &key, const QString &label)
{
    m_remember[key] = {key, label, false};
    ++s_totalRememberSets;

    /* 延迟创建复选框(仅首次) */
    if (!m_rememberCheck) {
        auto* layout = qobject_cast<QVBoxLayout*>(this->layout());
        if (layout && m_buttonBox) {
            m_rememberCheck = new QCheckBox(this);
            m_rememberCheck->setObjectName("edDialogRememberCheck");
            layout->insertWidget(layout->indexOf(m_buttonBox), m_rememberCheck);
        }
    }

    if (m_rememberCheck) {
        m_rememberCheck->setText(label);
        m_rememberCheck->setChecked(false);
    }
}

/** @brief 检查"记住选择"是否被勾选 @param key 选项键 @return 已勾选返回true */
bool EdDialog::isRememberChecked(const QString &key) const
{
    /* 如果有复选框，优先使用复选框状态 */
    if (m_rememberCheck && m_rememberCheck->isVisible()) {
        return m_rememberCheck->isChecked();
    }
    return m_remember.value(key).checked;
}

/** @brief 对话框关闭时保存记住状态并累计关闭计数 @param result 对话框结果码 */
void EdDialog::done(int result)
{
    /* 保存记住选择状态 */
    if (m_rememberCheck && m_rememberCheck->isVisible()) {
        for (auto it = m_remember.begin(); it != m_remember.end(); ++it) {
            it.value().checked = m_rememberCheck->isChecked();
        }
    }
    ++s_totalCloses;
    QDialog::done(result);
}
