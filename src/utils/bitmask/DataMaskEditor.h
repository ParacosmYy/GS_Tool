/**
 * @file DataMaskEditor.h
 * @brief 数据掩码编辑器 -- 可视化位域定义与掩码编辑
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 8/16/32/64 位宽度的逐位可视化编辑，支持命名位域定义、
 * 实时 hex/binary/decimal 显示、字段提取和 JSON 导入导出。
 * 统计查询与重置见：@see DataMaskEditorStats.cpp
 */

#ifndef DATAMASKEDITOR_H
#define DATAMASKEDITOR_H

#include <QColor>
#include <QComboBox>
#include <QList>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVector>
#include <QWidget>
#include <QLineEdit>

/**
 * @class DataMaskEditor
 * @brief 数据掩码可视化编辑器，支持位域定义与掩码操作
 *
 * 典型用途：嵌入式协议分析中定义寄存器位域、解析二进制帧字段。
 * 用户可点击单个 bit 切换其状态，也可定义带名称和颜色的位域范围。
 */
class DataMaskEditor : public QWidget {
    Q_OBJECT

public:
    /** @brief 位域定义结构体 */
    struct BitField {
        int startBit = 0;       ///< 起始位号(低位)
        int endBit = 0;         ///< 结束位号(高位)
        QString name;           ///< 字段名称
        QColor color;           ///< 显示颜色
        uint64_t value = 0;     ///< 字段当前值
    };

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalMaskChanges = 0;   ///< 掩码值变更次数
        quint64 totalFieldEdits = 0;    ///< 字段编辑次数
        quint64 totalFieldAdds = 0;     ///< 字段添加次数
        quint64 totalFieldRemoves = 0;  ///< 字段删除次数
        quint64 totalExports = 0;       ///< 导出次数
        int peakFields = 0;             ///< 峰值字段数
        int activeFields = 0;           ///< 当前活跃字段数
    };

    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit DataMaskEditor(QWidget *parent = nullptr);

    /** @brief 设置位宽度(8/16/32/64) @param bits 位宽 */
    void setBitWidth(int bits);

    /** @brief 获取当前位宽度 @return 位宽 */
    int bitWidth() const;

    /** @brief 设置掩码值 @param value 新的掩码值 */
    void setMaskValue(uint64_t value);

    /** @brief 获取当前掩码值 @return 掩码值 */
    uint64_t maskValue() const;

    /** @brief 添加位域定义 @param field 位域定义 */
    void addField(const BitField &field);

    /** @brief 移除指定位域 @param index 位域索引 */
    void removeField(int index);

    /** @brief 更新指定位域 @param index 位域索引 @param field 新的位域定义 */
    void updateField(int index, const BitField &field);

    /** @brief 获取所有位域定义 @return 位域列表 */
    QList<BitField> fields() const;

    /**
     * @brief 从给定数据中提取指定位域的值
     * @param index 位域索引
     * @param data 输入数据
     * @return 提取出的值(右对齐)
     */
    uint64_t extractField(int index, uint64_t data) const;

    /** @brief 导出位域定义到JSON文件 @param filePath 文件路径 @return true导出成功 */
    bool exportToJson(const QString &filePath);

    /** @brief 从JSON文件导入位域定义 @param filePath 文件路径 @return true导入成功 */
    bool importFromJson(const QString &filePath);

    /** @brief 获取统计信息 @return 统计快照 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 掩码值变更信号 @param value 新的掩码值 */
    void maskChanged(uint64_t value);

    /** @brief 字段添加信号 @param index 添加位置 */
    void fieldAdded(int index);

    /** @brief 字段移除信号 @param index 移除位置 */
    void fieldRemoved(int index);

    /** @brief 字段更新信号 @param index 更新位置 */
    void fieldUpdated(int index);

    /** @brief 单个bit切换信号 @param bit 位号 @param on 新状态 */
    void bitToggled(int bit, bool on);

private slots:
    void onBitClicked();        ///< 位按钮点击槽
    void onBitWidthChanged();   ///< 位宽切换槽
    void onAddField();          ///< 添加字段槽
    void onRemoveField();       ///< 删除字段槽
    void onExport();            ///< 导出槽
    void onImport();            ///< 导入槽
    void onFieldCellChanged(int row, int col); ///< 字段表格编辑槽

private:
    void setupUI();             ///< 初始化界面布局
    void rebuildBitButtons();   ///< 重建位按钮网格
    void refreshBitDisplay();   ///< 刷新位按钮显示状态
    void refreshValueDisplay(); ///< 刷新hex/binary/decimal显示
    void refreshFieldTable();   ///< 刷新字段表格
    void colorBitButtons();     ///< 根据字段定义给位按钮着色

    // ---- 数据成员 ----
    int m_bitWidth = 8;             ///< 当前位宽度
    uint64_t m_maskValue = 0;       ///< 当前掩码值
    QList<BitField> m_fields;       ///< 位域定义列表
    QVector<bool> m_bits;           ///< 各位状态
    Stats m_stats;                  ///< 统计计数器

    // ---- UI 控件 ----
    QComboBox *m_bitWidthCombo;     ///< 位宽选择器
    QLineEdit *m_hexValueEdit;      ///< 十六进制值显示
    QLineEdit *m_binValueEdit;      ///< 二进制值显示
    QLineEdit *m_decValueEdit;      ///< 十进制值显示
    QTableWidget *m_fieldTable;     ///< 字段定义表格
    QPushButton *m_addFieldBtn;     ///< 添加字段按钮
    QPushButton *m_removeFieldBtn;  ///< 删除字段按钮
    QPushButton *m_exportBtn;       ///< 导出按钮
    QPushButton *m_importBtn;       ///< 导入按钮
    QList<QPushButton*> m_bitButtons; ///< 位按钮列表
};

#endif // DATAMASKEDITOR_H
