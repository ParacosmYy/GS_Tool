/**
 * @file FrameVisualEditor.h
 * @brief 帧格式可视化编辑器 - 用户在UI上配置帧格式定义
 *
 * 提供完整的帧格式编辑能力:
 *   - 帧头/帧尾HEX配置
 *   - 长度字段配置(偏移/大小/字节序/调整值)
 *   - 校验配置(类型/偏移/起始)
 *   - 数据字段拖拽排序(行级drag-drop)
 *   - 扩展字段类型: uint8/uint16/uint32/int8/int16/int32/float/string/raw
 *   - 字段属性编辑(名称/偏移/大小/字节序/缩放因子/偏移值)
 *   - 实时预览: 显示当前帧格式的二进制布局图
 *
 * 协作关系:
 *   - FrameDefinition: 数据结构，编辑器读写的目标
 *   - FrameParser: 接收definitionChanged信号更新解析规则
 *   - HexConverter: HEX字符串与字节数组的相互转换
 *
 * 设计模式: 观察者模式(Qt信号/槽)
 *   编辑器发出definitionChanged信号，FrameParser监听并更新
 */
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
#include <QLabel>
#include "protocol/parser/FrameDefinition.h"

/** @brief 帧格式可视化编辑器，编辑内容实时同步到FrameParser，支持字段拖拽排序和实时二进制布局预览 */
class FrameVisualEditor : public QWidget {
    Q_OBJECT

public:
    explicit FrameVisualEditor(QWidget* parent = nullptr); ///< 构造帧格式可视化编辑器
    FrameDefinition currentDefinition() const; ///< 获取当前编辑的帧格式定义
    void setDefinition(const FrameDefinition& def); ///< 从外部加载帧格式到编辑器

signals:
    void definitionChanged(const FrameDefinition& def); ///< 帧格式定义变更，通知FrameParser更新

private slots:
    void onHeaderChanged();       ///< 帧头HEX文本变更
    void onFooterChanged();       ///< 帧尾HEX文本变更
    void onLengthConfigChanged(); ///< 长度字段配置变更
    void onChecksumConfigChanged(); ///< 校验配置变更
    void onAddField();            ///< 添加新字段(插入到表格末尾)
    void onRemoveField();         ///< 删除选中的字段行
    void onFieldChanged(int row, int col); ///< 字段表格内容变更
    void onApply();               ///< 应用当前编辑器内容到FrameDefinition

private:
    void setupUI();               ///< 初始化所有UI控件和布局
    QGroupBox* setupHeaderGroup();  ///< 创建帧头/帧尾配置分组
    QGroupBox* setupLengthGroup();  ///< 创建长度字段配置分组
    QGroupBox* setupChecksumGroup(); ///< 创建校验配置分组
    QGroupBox* setupFieldsGroup();  ///< 创建数据字段表格分组(含拖拽排序和操作按钮)
    QGroupBox* setupPreviewGroup(); ///< 创建二进制布局预览分组
    void setupConnections();        ///< 连接所有信号/槽
    void rebuildDefinition();       ///< 从UI控件收集数据重建FrameDefinition
    void updateFieldTable();        ///< 从FrameDefinition填充字段表格
    void updateBinaryPreview();     ///< 更新二进制布局预览图
    void onPreviewTimerTick();      ///< 刷新预览和布局提示
    void onMoveFieldUp();           ///< 将当前选中行上移一行
    void onMoveFieldDown();         ///< 将当前选中行下移一行
    int typeSizeFromIndex(int typeIndex) const; ///< 计算字段类型占用的字节数

    // ---- 帧头/帧尾 ----
    QLineEdit* m_headerEdit;        ///< 帧头HEX输入框
    QLineEdit* m_footerEdit;        ///< 帧尾HEX输入框

    // ---- 长度字段配置 ----
    QSpinBox* m_lengthOffsetSpin;   ///< 长度字段偏移(-1=无)
    QComboBox* m_lengthSizeCombo;   ///< 长度字段大小(1/2字节)
    QCheckBox* m_lengthBEndianCheck;///< 长度字段大小端
    QSpinBox* m_lengthAdjustSpin;   ///< 长度调整值

    // ---- 校验配置 ----
    QComboBox* m_checksumTypeCombo; ///< 校验类型(None/Sum8/CRC8/CRC16/CRC32)
    QSpinBox* m_checksumOffsetSpin; ///< 校验偏移(-1=自动)
    QSpinBox* m_checksumStartSpin;  ///< 校验计算起始偏移

    // ---- 字段表格 ----
    QTableWidget* m_fieldTable;     ///< 字段定义表格(支持拖拽排序)
    QPushButton* m_addFieldBtn;     ///< 添加字段按钮
    QPushButton* m_removeFieldBtn;  ///< 删除字段按钮

    // ---- 预览区 ----
    QLabel* m_previewLabel;         ///< 二进制布局预览标签

    // ---- 操作按钮 ----
    QPushButton* m_applyBtn;        ///< 应用定义按钮

    FrameDefinition m_def;          ///< 当前帧格式定义
    bool m_updating = false;        ///< 防止循环更新标志

    // ---- 统计计数器 ----
    quint64 m_totalFramesBuilt = 0; ///< 帧构建总次数
    quint64 m_totalSends = 0;       ///< 帧发送总次数
    quint64 m_totalEdits = 0;       ///< 编辑操作总次数
    quint64 m_totalFieldAdds = 0;   ///< 字段添加总次数
    quint64 m_totalFieldRemoves = 0; ///< 字段删除总次数
    quint64 m_totalFrameValidations = 0; ///< 帧校验总次数
    quint64 m_totalProtocolLoads = 0; ///< 协议加载总次数
    quint64 m_totalProtocolSaves = 0; ///< 协议保存总次数
    quint64 m_validationErrors = 0; ///< 校验错误总次数

public:
    quint64 totalFramesBuilt() const { return m_totalFramesBuilt; } ///< 帧构建总次数
    quint64 totalSends() const { return m_totalSends; }             ///< 帧发送总次数
    quint64 totalEdits() const { return m_totalEdits; }             ///< 编辑操作总次数
    quint64 totalFieldAdds() const { return m_totalFieldAdds; }     ///< 字段添加总次数
    quint64 totalFieldRemoves() const { return m_totalFieldRemoves; } ///< 字段删除总次数
    quint64 totalFrameValidations() const { return m_totalFrameValidations; } ///< 帧校验总次数
    quint64 totalProtocolLoads() const { return m_totalProtocolLoads; } ///< 协议加载总次数
    quint64 totalProtocolSaves() const { return m_totalProtocolSaves; } ///< 协议保存总次数
    quint64 totalValidationErrors() const { return m_validationErrors; } ///< 校验错误总次数
    void resetEditorStatistics(); ///< 重置帧编辑器统计计数器

    /** @brief 扩展字段类型列表(与ComboBox项对应): 0=UInt8,1=UInt16LE,2=UInt16BE,3=UInt32LE,4=UInt32BE,5=Int8,6=Int16LE,7=Int16BE,8=Float,9=Raw */
    static QStringList fieldTypeNames();
};

#endif // FRAMEVISUALEDITOR_H
