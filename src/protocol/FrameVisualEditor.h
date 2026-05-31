#ifndef FRAMEVISUALEDITOR_H
#define FRAMEVISUALEDITOR_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include "protocol/FrameDefinition.h"

// 帧格式可视化编辑器 - 用户在UI上配置帧格式定义
// 编辑内容实时同步到FrameParser
class FrameVisualEditor : public QWidget {
    Q_OBJECT

public:
    explicit FrameVisualEditor(QWidget* parent = nullptr);

    // 获取当前编辑的帧格式定义
    FrameDefinition currentDefinition() const;

    // 从外部加载帧格式到编辑器
    void setDefinition(const FrameDefinition& def);

signals:
    // 帧格式定义变更，通知FrameParser更新
    void definitionChanged(const FrameDefinition& def);

private slots:
    void onHeaderChanged();
    void onFooterChanged();
    void onLengthConfigChanged();
    void onChecksumConfigChanged();
    void onAddField();
    void onRemoveField();
    void onFieldChanged(int row, int col);
    void onApply();

private:
    void setupUI();
    void rebuildDefinition();
    void updateFieldTable();

    // 帧头/帧尾
    QLineEdit* m_headerEdit;
    QLineEdit* m_footerEdit;

    // 长度字段配置
    QSpinBox* m_lengthOffsetSpin;
    QComboBox* m_lengthSizeCombo;
    QCheckBox* m_lengthBEndianCheck;
    QSpinBox* m_lengthAdjustSpin;

    // 校验配置
    QComboBox* m_checksumTypeCombo;
    QSpinBox* m_checksumOffsetSpin;
    QSpinBox* m_checksumStartSpin;

    // 字段表
    QTableWidget* m_fieldTable;
    QPushButton* m_addFieldBtn;
    QPushButton* m_removeFieldBtn;

    // 应用按钮
    QPushButton* m_applyBtn;

    FrameDefinition m_def;
    bool m_updating = false; // 防止循环更新
};

#endif // FRAMEVISUALEDITOR_H
