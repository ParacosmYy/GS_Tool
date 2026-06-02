/**
 * @file ExportDialog.cpp
 * @brief 导出对话框实现
 */

#include "utils/export/ExportDialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent)
    , m_formatCombo(nullptr)
    , m_pathEdit(nullptr)
    , m_exportBtn(nullptr)
{
    setObjectName("ExportDialog");
    setWindowTitle(tr("导出数据"));
    setupUI();
}

QString ExportDialog::selectedPath() const
{
    return m_pathEdit ? m_pathEdit->text() : QString();
}

int ExportDialog::selectedFormat() const
{
    return m_formatCombo ? m_formatCombo->currentIndex() : 0;
}

void ExportDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 格式选择行
    auto* formatLayout = new QHBoxLayout();
    auto* formatLabel = new QLabel(tr("导出格式:"), this);
    m_formatCombo = new QComboBox(this);
    m_formatCombo->setObjectName("formatCombo");
    m_formatCombo->addItem(tr("CSV 数据文件"));
    m_formatCombo->addItem(tr("PNG 图片"));
    m_formatCombo->addItem(tr("SVG 矢量图"));
    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(m_formatCombo, 1);
    mainLayout->addLayout(formatLayout);

    // 路径选择行
    auto* pathLayout = new QHBoxLayout();
    auto* pathLabel = new QLabel(tr("文件路径:"), this);
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setObjectName("pathEdit");
    m_pathEdit->setPlaceholderText(tr("请选择导出文件路径"));
    auto* browseBtn = new QPushButton(tr("浏览..."), this);
    browseBtn->setObjectName("browseBtn");
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(m_pathEdit, 1);
    pathLayout->addWidget(browseBtn);
    mainLayout->addLayout(pathLayout);

    // 导出按钮行
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_exportBtn = new QPushButton(tr("导出"), this);
    m_exportBtn->setObjectName("exportBtn");
    btnLayout->addWidget(m_exportBtn);
    mainLayout->addLayout(btnLayout);

    // TODO: 连接 browseBtn → 文件对话框
    // TODO: 连接 m_exportBtn → emit exportRequested(...)
}
