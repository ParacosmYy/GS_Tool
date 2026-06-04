/**
 * @file ExportDialog.cpp
 * @brief 导出对话框实现
 *
 * 提供导出格式选择（CSV/JSON/PNG/SVG）和文件路径配置界面。
 * 用户点击"导出"按钮后以 Accepted 状态关闭对话框，
 * 调用方通过 selectedPath() / selectedFormat() 获取结果。
 */

#include "utils/export/ExportDialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>

// ---------------------------------------------------------------------------
// 默认文件扩展名表（与 selectedFormat() 索引对齐）
// ---------------------------------------------------------------------------
static const char* s_defaultExtensions[] = {
    ".csv",    ///< 0 = CSV
    ".json",   ///< 1 = JSON
    ".png",    ///< 2 = PNG
    ".svg"     ///< 3 = SVG
};

// ---------------------------------------------------------------------------
// 构造函数
// ---------------------------------------------------------------------------

/**
 * @brief 构造导出对话框
 *
 * 初始化所有子控件、布局和信号连接。
 *
 * @param parent 父控件指针
 */
ExportDialog::ExportDialog(QWidget* parent)
    : QDialog(parent)
    , m_formatCombo(nullptr)
    , m_pathEdit(nullptr)
    , m_exportBtn(nullptr)
    , m_browseBtn(nullptr)
    , m_cancelBtn(nullptr)
{
    setObjectName("ExportDialog");
    setWindowTitle(tr("导出数据"));
    setupUI();
    setupConnections();
}

// ---------------------------------------------------------------------------
// 公共访问器
// ---------------------------------------------------------------------------

/**
 * @brief 获取用户输入的文件路径
 * @return 文件路径文本，可能为空
 */
QString ExportDialog::selectedPath() const
{
    return m_pathEdit ? m_pathEdit->text() : QString();
}

/**
 * @brief 获取用户选择的导出格式索引
 * @return 0=CSV, 1=JSON, 2=PNG, 3=SVG
 */
int ExportDialog::selectedFormat() const
{
    return m_formatCombo ? m_formatCombo->currentIndex() : 0;
}

// ---------------------------------------------------------------------------
// 界面初始化
// ---------------------------------------------------------------------------

/**
 * @brief 构建对话框界面布局和信号连接
 *
 * 布局结构：
 * - QFormLayout：格式下拉框 + 路径输入（含浏览按钮）
 * - QHBoxLayout：导出 / 取消按钮
 */
void ExportDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    /* --- 表单区域 --- */
    auto* formLayout = new QFormLayout();

    // 格式下拉框
    m_formatCombo = new QComboBox(this);
    m_formatCombo->setObjectName("formatCombo");
    m_formatCombo->addItem(tr("CSV 数据文件"));
    m_formatCombo->addItem(tr("JSON 数据文件"));
    m_formatCombo->addItem(tr("PNG 图片 (截图)"));
    m_formatCombo->addItem(tr("SVG 矢量图"));
    formLayout->addRow(tr("导出格式:"), m_formatCombo);

    // 路径输入 + 浏览按钮
    auto* pathRow = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setObjectName("pathEdit");
    m_pathEdit->setPlaceholderText(tr("请选择导出文件路径"));

    auto* browseBtn = new QPushButton(tr("浏览..."), this);
    browseBtn->setObjectName("browseBtn");
    m_browseBtn = browseBtn;
    pathRow->addWidget(m_pathEdit, 1);
    pathRow->addWidget(m_browseBtn);

    formLayout->addRow(tr("文件路径:"), pathRow);
    mainLayout->addLayout(formLayout);

    /* --- 按钮区域 --- */
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_exportBtn = new QPushButton(tr("导出"), this);
    m_exportBtn->setObjectName("exportBtn");
    m_exportBtn->setDefault(true);

    auto* cancelBtn = new QPushButton(tr("取消"), this);
    cancelBtn->setObjectName("cancelBtn");
    m_cancelBtn = cancelBtn;

    btnLayout->addWidget(m_exportBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    /* --- 信号连接见 setupConnections() (ExportDialogSlots.cpp) --- */
}
