/**
 * @file DeviceSimulatorPanel.h
 * @brief 设备模拟器配置面板 -- 可视化管理命令-响应规则
 *
 * 提供 QTableWidget 规则表、模式/延迟/策略编辑器、导入导出 JSON 等功能。
 * 所有 QWidget 子对象均设置了 objectName（QSS 依赖）。
 */
#ifndef DEVICESIMULATORPANEL_H
#define DEVICESIMULATORPANEL_H

#include <QWidget>
#include <QSharedPointer>

#include "utils/simulator/SimulatorTypes.h"

class QTableWidget;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class DeviceSimulator;

/**
 * @brief 设备模拟器配置面板 -- 管理命令-响应规则的 QWidget
 *
 * 面板布局:
 *   顶部: 全局配置栏(回显开关、噪声率、默认延迟)
 *   中部: QTableWidget 规则表(命令模式、匹配策略、响应模式、延迟范围、启用)
 *   底部: 按钮栏(添加/删除/清空/导入JSON/导出JSON)
 */
class DeviceSimulatorPanel : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造配置面板 @param simulator 关联的模拟器实例(可为nullptr) @param parent 父Widget */
    explicit DeviceSimulatorPanel(QSharedPointer<DeviceSimulator> simulator,
                                  QWidget* parent = nullptr);

    /** @brief 关联模拟器实例(替换当前关联) */
    void setSimulator(QSharedPointer<DeviceSimulator> simulator);
    /** @brief 获取关联的模拟器实例 */
    QSharedPointer<DeviceSimulator> simulator() const;

    // ── JSON 导入/导出 ──

    /** @brief 将当前规则导出为 JSON 文件 @param filePath 目标文件路径 @return true成功 */
    bool exportToJson(const QString& filePath) const;
    /** @brief 从 JSON 文件导入规则 @param filePath 源文件路径 @return true成功 */
    bool importFromJson(const QString& filePath);

signals:
    /** @brief 规则表内容已变更 */
    void rulesChanged();
    /** @brief 模拟器配置已变更(回显/噪声/延迟) */
    void configChanged();

private slots:
    /** @brief 添加一条空规则 */
    void onAddRule();
    /** @brief 删除选中行 */
    void onRemoveRule();
    /** @brief 清空所有规则 */
    void onClearRules();
    /** @brief 导出按钮点击 */
    void onExportClicked();
    /** @brief 导入按钮点击 */
    void onImportClicked();
    /** @brief 规则表单元格变更 @param row 行号 @param col 列号 */
    void onCellChanged(int row, int col);

private:
    /** @brief 构建 UI 布局 */
    void setupUI();
    /** @brief 从模拟器加载规则到表格 */
    void loadFromSimulator();
    /** @brief 将表格当前内容同步到模拟器 */
    void syncToSimulator();
    /** @brief 从表格行构造 SimResponse */
    SimResponse buildRuleFromRow(int row) const;
    /** @brief 填充一行表格 @param row 行号 @param rule 规则数据 */
    void fillRow(int row, const SimResponse& rule);
    /** @brief 响应模式 → 显示文本 */
    QString modeToString(ResponseMode mode) const;
    /** @brief 匹配策略 → 显示文本 */
    QString strategyToString(MatchStrategy s) const;

    QSharedPointer<DeviceSimulator> m_simulator;  ///< 关联的模拟器实例
    QTableWidget* m_table = nullptr;               ///< 规则表
    QPushButton* m_addBtn = nullptr;               ///< 添加按钮
    QPushButton* m_removeBtn = nullptr;            ///< 删除按钮
    QPushButton* m_clearBtn = nullptr;             ///< 清空按钮
    QPushButton* m_exportBtn = nullptr;            ///< 导出按钮
    QPushButton* m_importBtn = nullptr;            ///< 导入按钮
    QCheckBox* m_echoCheck = nullptr;              ///< 回显开关
    QDoubleSpinBox* m_noiseSpin = nullptr;         ///< 噪声率
    QSpinBox* m_minDelaySpin = nullptr;            ///< 默认最小延迟
    QSpinBox* m_maxDelaySpin = nullptr;            ///< 默认最大延迟
    QLabel* m_statsLabel = nullptr;                ///< 统计信息标签
    bool m_syncing = false;                        ///< 防止循环同步标志
};

#endif // DEVICESIMULATORPANEL_H
