/**
 * @file ExportDialogSlots.cpp
 * @brief 导出对话框 — 信号连接与交互逻辑实现
 *
 * 从 ExportDialog.cpp 拆分而来，包含:
 *   - setupUI() 中的信号连接(浏览/导出/取消/格式切换)
 *     独立为 setupConnections() 方法
 *
 * 构造函数和公共访问器见 ExportDialog.cpp。
 */

#include "utils/export/ExportDialog.h"

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>

// ---------------------------------------------------------------------------
// 默认文件扩展名表（与 selectedFormat() 索引对齐）
// ---------------------------------------------------------------------------
static const char* s_extTable[] = {
    ".csv",    ///< 0 = CSV
    ".json",   ///< 1 = JSON
    ".png",    ///< 2 = PNG
    ".svg"     ///< 3 = SVG
};

/** @brief 初始化信号连接：浏览按钮弹出文件对话框，导出按钮关闭对话框，格式切换更新扩展名 */
void ExportDialog::setupConnections()
{
    // 浏览按钮：弹出文件保存对话框，根据当前格式更新默认扩展名
    connect(m_browseBtn, &QPushButton::clicked, this, [this]() {
        int idx = m_formatCombo->currentIndex();
        QString ext;
        QString filter;

        switch (idx) {
        case 0:  // CSV
            ext    = "csv";
            filter = tr("CSV 文件 (*.csv)");
            break;
        case 1:  // JSON
            ext    = "json";
            filter = tr("JSON 文件 (*.json)");
            break;
        case 2:  // PNG
            ext    = "png";
            filter = tr("PNG 图片 (*.png)");
            break;
        case 3:  // SVG
            ext    = "svg";
            filter = tr("SVG 文件 (*.svg)");
            break;
        default:
            ext    = "csv";
            filter = tr("所有文件 (*)");
            break;
        }

        QString path = QFileDialog::getSaveFileName(
            this, tr("选择导出文件"), m_pathEdit->text(), filter);

        if (!path.isEmpty()) {
            // 自动追加默认扩展名
            if (!path.endsWith(QString(".%1").arg(ext), Qt::CaseInsensitive)) {
                path += QString(".%1").arg(ext);
            }
            m_pathEdit->setText(path);
        }
    });

    // 导出按钮：关闭对话框并发出 exportRequested 信号
    connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
        ++m_totalExports;
        emit exportRequested(m_pathEdit->text(), m_formatCombo->currentIndex());
        accept();
    });

    // 取消按钮：关闭对话框
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // 格式切换：更新路径中的文件扩展名
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        ++m_totalFormatChanges;
        QString path = m_pathEdit->text();
        if (path.isEmpty()) {
            return;
        }

        // 移除旧扩展名，追加新的
        for (const auto* oldExt : s_extTable) {
            if (path.endsWith(QString(oldExt), Qt::CaseInsensitive)) {
                path.chop(static_cast<int>(qstrlen(oldExt)));
                break;
            }
        }

        if (index >= 0 && index < 4) {
            path += QString(s_extTable[index]);
        }

        m_pathEdit->setText(path);
    });
}
