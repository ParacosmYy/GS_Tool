/**
 * @file PacketBuilderPanel.h
 * @brief 数据包构建面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供字段编辑表格、添加/删除按钮和十六进制预览的交互面板。
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

private:
    /**
     * @brief 刷新表格内容
     */
    void refreshTable();

    QTableWidget *m_fieldTable;         ///< 字段表格
    QPushButton *m_addFieldBtn;         ///< 添加字段按钮
    QPushButton *m_removeFieldBtn;      ///< 删除字段按钮
    QPushButton *m_buildBtn;            ///< 构建按钮
    QTextEdit *m_hexPreview;            ///< 十六进制预览区
    PacketBuilder *m_builder = nullptr; ///< 关联的构建器
};

#endif // PACKETBUILDERPANEL_H
