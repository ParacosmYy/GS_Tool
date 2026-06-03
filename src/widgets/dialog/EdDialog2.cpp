#include "widgets/dialog/EdDialog2.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>
EdDialog::EdDialog(QWidget *parent) : QDialog(parent) { setObjectName("EdDialog2"); setupUi(); }
EdDialog::~EdDialog() = default;
void EdDialog::setupUi() { auto *l = new QVBoxLayout(this); l->setContentsMargins(16,16,16,16); setLayout(l); }
void EdDialog::setTitle(const QString &t) { setWindowTitle(t); }
void EdDialog::setMessage(const QString &m) { auto *l = qobject_cast<QVBoxLayout*>(layout()); if (l) { auto *lbl = new QLabel(m, this); l->insertWidget(0, lbl); } }
void EdDialog::addButton(StandardButton b) { Q_UNUSED(b); }
void EdDialog::addCustomButton(const QString &label, int role) { Q_UNUSED(label); Q_UNUSED(role); }
void EdDialog::setIcon(const QString &name) { Q_UNUSED(name); }
void EdDialog::setContentWidget(QWidget *w) { auto *l = layout(); if (l) l->addWidget(w); }
int EdDialog::resultRole() const { return m_resultRole; }
void EdDialog::setRememberOption(const QString &key, const QString &label) { m_remember[key] = {key, label, false}; }
bool EdDialog::isRememberChecked(const QString &key) const { return m_remember.value(key).checked; }
