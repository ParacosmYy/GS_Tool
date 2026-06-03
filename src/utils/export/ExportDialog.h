/**
 * @file ExportDialog.h
 * @brief 导出对话框，提供导出格式选择和文件路径配置 UI
 *
 * 作为 F3 多通道数据导出子系统的用户交互入口，
 * 用户通过此对话框选择导出格式（CSV/PNG/SVG）和目标文件路径。
 */

#ifndef EXPORT_DIALOG_H
#define EXPORT_DIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QPushButton;

/**
 * @class ExportDialog
 * @brief 导出配置对话框
 *
 * 提供格式选择下拉框、文件路径输入框和导出按钮，
 * 用户确认后通过 exportRequested 信号传递导出参数。
 */
class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件指针
     */
    explicit ExportDialog(QWidget* parent = nullptr);

    /**
     * @brief 获取用户选择的文件路径
     * @return 文件路径字符串
     */
    QString selectedPath() const;

    /**
     * @brief 获取用户选择的导出格式索引
     * @return 格式索引（0=CSV, 1=JSON, 2=PNG, 3=SVG）
     */
    int selectedFormat() const;

signals:
    /**
     * @brief 用户请求导出信号
     * @param filePath 导出文件路径
     * @param format 导出格式索引
     */
    void exportRequested(const QString& filePath, int format);

private:
    /**
     * @brief 初始化界面布局和控件
     */
    void setupUI();

    QComboBox*   m_formatCombo;  ///< 导出格式选择下拉框
    QLineEdit*   m_pathEdit;     ///< 文件路径输入框
    QPushButton* m_exportBtn;    ///< 导出按钮

    // ---- 统计计数器 ----
    quint64 m_totalExports = 0;  ///< 累计导出确认次数
    quint64 m_totalFormatChanges = 0; ///< 累计格式切换次数
public:
    /** @brief 获取累计导出确认次数 @return 导出次数 */
    quint64 totalExports() const { return m_totalExports; }
    /** @brief 获取累计格式切换次数 @return 切换次数 */
    quint64 totalFormatChanges() const { return m_totalFormatChanges; }
    /** @brief 重置导出对话框统计计数器(导出次数/格式切换次数) */
    void resetExportDialogStatistics() { m_totalExports = 0; m_totalFormatChanges = 0; }
};

#endif // EXPORT_DIALOG_H
