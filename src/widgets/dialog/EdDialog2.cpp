/**
 * @file EdDialog2.cpp
 * @brief 通用对话框v2实现 — 自定义标题/消息/按钮/记住选项
 */
#include "widgets/dialog/EdDialog2.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>

/** @brief 构造函数 @param parent 父Widget */
EdDialog::EdDialog(QWidget *parent) : QDialog(parent) { setObjectName("EdDialog2"); ++s_totalOpens; setupUi(); }
/** @brief 析构函数 */
EdDialog::~EdDialog() = default;
/** @brief 初始化UI布局 */
void EdDialog::setupUi() { auto *l = new QVBoxLayout(this); l->setContentsMargins(16,16,16,16); setLayout(l); }
/** @brief 设置对话框标题 @param t 标题文本 */
void EdDialog::setTitle(const QString &t) { setWindowTitle(t); }
/** @brief 设置对话框消息内容 @param m 消息文本 */
void EdDialog::setMessage(const QString &m) { auto *l = qobject_cast<QVBoxLayout*>(layout()); if (l) { auto *lbl = new QLabel(m, this); lbl->setObjectName("edDialogMessageLabel"); l->insertWidget(0, lbl); } }
/** @brief 添加标准按钮 @param b 标准按钮类型 */
void EdDialog::addButton(StandardButton b) { Q_UNUSED(b); }
/** @brief 添加自定义按钮 @param label 按钮文本 @param role 按钮角色 */
void EdDialog::addCustomButton(const QString &label, int role) { Q_UNUSED(label); Q_UNUSED(role); ++s_totalButtonPresses; m_resultRole = role; }
/** @brief 设置对话框图标 @param name 图标名称 */
void EdDialog::setIcon(const QString &name) { Q_UNUSED(name); }
/** @brief 设置自定义内容Widget @param w 内容Widget */
void EdDialog::setContentWidget(QWidget *w) { auto *l = layout(); if (l) l->addWidget(w); }
/** @brief 获取对话框结果角色 @return 角色值 */
int EdDialog::resultRole() const { return m_resultRole; }
/** @brief 设置"记住选择"选项 @param key 选项键 @param label 显示文本 */
void EdDialog::setRememberOption(const QString &key, const QString &label) { m_remember[key] = {key, label, false}; ++s_totalRememberSets; }
/** @brief 检查"记住选择"是否被勾选 @param key 选项键 @return 已勾选返回true */
bool EdDialog::isRememberChecked(const QString &key) const { return m_remember.value(key).checked; }

/** @brief 对话框关闭时累计关闭计数 @param result 对话框结果码 */
void EdDialog::done(int result) { ++s_totalCloses; QDialog::done(result); }
