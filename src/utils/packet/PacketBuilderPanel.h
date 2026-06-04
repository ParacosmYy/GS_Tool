/**
 * @file PacketBuilderPanel.h
 * @brief 数据包构建面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供字段编辑表格、添加/删除/构建按钮和十六进制预览的交互面板。
 */

#ifndef PACKETBUILDERPANEL_H
#define PACKETBUILDERPANEL_H

#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "utils/packet/PacketBuilder.h"

/**
 * @class PacketBuilderPanel
 * @brief 数据包构建器 UI 面板
 */
class PacketBuilderPanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit PacketBuilderPanel(QWidget *parent = nullptr);

    /**
     * @brief 设置关联的 PacketBuilder 实例
     * @param builder 数据包构建器指针
     */
    void setBuilder(PacketBuilder *builder);

    /** @brief 获取累计构建数据包次数 */
    quint64 totalPacketsBuilt() const { return m_totalPacketsBuilt; }

    /** @brief 获取累计发送数据包次数 */
    quint64 totalSends() const { return m_totalSends; }

    /** @brief 获取累计字段编辑次数 @return 编辑次数 */
    quint64 totalFieldEdits() const { return m_totalFieldEdits; }

    /** @brief 获取累计模板加载次数 @return 加载次数 */
    quint64 totalTemplateLoads() const { return m_totalTemplateLoads; }

    /** @brief 重置所有统计计数器(构建/发送/字段编辑/模板加载) */
    void resetStatistics();

private slots:
    /**
     * @brief 添加新字段
     */
    void onAddField();

    /**
     * @brief 删除选中字段
     */
    void onRemoveField();

    /**
     * @brief 构建数据包并更新预览
     */
    void onBuild();

    /**
     * @brief 加载模板文件
     */
    void onLoadTemplate();

    /**
     * @brief 保存模板文件
     */
    void onSaveTemplate();

    /**
     * @brief 清除所有字段
     */
    void onClearAll();

    /**
     * @brief 上移选中字段
     */
    void onMoveUp();

    /**
     * @brief 下移选中字段
     */
    void onMoveDown();

private:
    /**
     * @brief 刷新表格内容
     */
    void refreshTable();

    /**
     * @brief 将QByteArray格式化为hex dump
     * @param data 原始数据
     * @return 格式化的hex dump字符串
     */
    QString formatHexDump(const QByteArray &data) const;

    QTableWidget *m_fieldTable;         ///< 字段表格
    QPushButton *m_addFieldBtn;         ///< 添加字段按钮
    QPushButton *m_removeFieldBtn;      ///< 删除字段按钮
    QPushButton *m_buildBtn;            ///< 构建按钮
    QPushButton *m_loadBtn;             ///< 加载模板按钮
    QPushButton *m_saveBtn;             ///< 保存模板按钮
    QPushButton *m_clearAllBtn;         ///< 清除所有字段按钮
    QPushButton *m_moveUpBtn;           ///< 上移字段按钮
    QPushButton *m_moveDownBtn;         ///< 下移字段按钮
    QTextEdit *m_hexPreview;            ///< 十六进制预览区
    PacketBuilder *m_builder = nullptr; ///< 关联的构建器

    // ---- 统计计数器 ----
    quint64 m_totalPacketsBuilt = 0;   ///< 累计构建数据包次数
    quint64 m_totalSends = 0;          ///< 累计发送数据包次数
    quint64 m_totalFieldEdits = 0;     ///< 累计字段编辑次数
    quint64 m_totalTemplateLoads = 0;  ///< 累计模板加载次数
};

#endif // PACKETBUILDERPANEL_H
