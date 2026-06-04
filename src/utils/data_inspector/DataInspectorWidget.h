/**
 * @file DataInspectorWidget.h
 * @brief 字节级数据检查器面板 — 多格式多字节解析视图
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供原始数据在 Hex/ASCII/Binary/Decimal/Octal 多种进制下的逐字节展示，
 * 以及从选中位置出发的多字节类型（int16/uint16/int32/uint32/float）解释。
 */

#ifndef DATAINSPECTORWIDGET_H
#define DATAINSPECTORWIDGET_H

#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QTableWidget>
#include <QWidget>

/**
 * @class DataInspectorWidget
 * @brief 字节级数据检查器控件
 *
 * 上半部分为逐字节明细表（Offset/Hex/Dec/Oct/Bin/ASCII 六列），
 * 下半部分为多字节解释表（int16/uint16/int32/uint32/float 五行），
 * 工具栏提供字节序切换（LittleEndian/BigEndian）和有符号开关。
 */
class DataInspectorWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 字节序枚举 */
    enum ByteOrder { LittleEndian, BigEndian };

    /** @brief 逐字节表列索引 */
    enum DataColumn {
        ColOffset,  ///< 偏移量列
        ColHex,     ///< 十六进制列
        ColDecimal, ///< 十进制列
        ColOctal,   ///< 八进制列
        ColBinary,  ///< 二进制列
        ColAscii,   ///< ASCII 列
        ColCount    ///< 列总数
    };

    /** @brief 检查器统计数据结构 */
    struct Stats {
        quint64 totalInspections    = 0; ///< 累计检查次数
        quint64 totalBytesInspected = 0; ///< 累计检查字节数
        quint64 totalCopies         = 0; ///< 累计复制次数
        quint64 peakBytesPerInspect = 0; ///< 单次最大检查字节数
        quint64 formatChanges       = 0; ///< 累计格式/字节序切换次数
    };

    /** @brief 构造数据检查器面板 @param parent 父控件 */
    explicit DataInspectorWidget(QWidget *parent = nullptr);

    /** @brief 设置待检查的原始数据 @param data 字节数组 */
    void setData(const QByteArray &data);

    /** @brief 获取当前数据 @return 字节数组引用 */
    QByteArray data() const;

    /** @brief 设置多字节解释字节序 @param order 字节序 */
    void setByteOrder(ByteOrder order);

    /** @brief 获取当前字节序 @return 字节序枚举 */
    ByteOrder byteOrder() const;

    /** @brief 获取统计数据 @return Stats 常量引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 数据检查完成信号 @param byteCount 检查字节数 */
    void dataInspected(int byteCount);

    /** @brief 数据已复制到剪贴板信号 @param text 复制内容 */
    void dataCopied(const QString &text);

private slots:
    /** @brief 字节序下拉框变更处理 */
    void onByteOrderChanged(int index);

    /** @brief 字节表单元格点击 — 更新多字节视图 */
    void onCellClicked(int row, int column);

    /** @brief 复制选中单元格内容到剪贴板 */
    void onCopySelected();

private:
    /** @brief 构建 UI 布局、控件、信号连接 */
    void setupUI();

    /** @brief 刷新逐字节明细表 */
    void refreshTable();

    /** @brief 根据选中行刷新多字节解释表 */
    void updateMultiByteView();

    /** @brief 将字节转为可显示 ASCII 字符 @param byte 输入字节 @return 可显示字符或 '.' */
    static char byteToAscii(unsigned char byte);

    /** @brief 将字节转为 8 位二进制字符串 @param byte 输入字节 @return 如 "01000001" */
    static QString byteToBinary(unsigned char byte);

    QTableWidget *m_byteTable;       ///< 逐字节明细表
    QTableWidget *m_multiByteTable;  ///< 多字节解释表
    QComboBox    *m_byteOrderCombo;  ///< 字节序选择下拉框
    QCheckBox    *m_signedCheck;     ///< 有符号开关复选框
    QLabel       *m_summaryLabel;    ///< 底部摘要标签

    QByteArray m_data;               ///< 待检查的原始数据
    ByteOrder  m_byteOrder = LittleEndian; ///< 当前字节序
    bool       m_signed    = false;  ///< 是否按有符号解释

    Stats m_stats;                   ///< 统计数据
};

#endif // DATAINSPECTORWIDGET_H
