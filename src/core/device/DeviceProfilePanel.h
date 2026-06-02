/**
 * @file DeviceProfilePanel.h
 * @brief 设备配置选择面板 UI
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 提供设备列表、新建/编辑/删除/导入/导出按钮的交互面板。
 */

#ifndef DEVICEPROFILEPANEL_H
#define DEVICEPROFILEPANEL_H

#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "core/device/DeviceProfile.h"
#include "core/device/DeviceRegistry.h"

/**
 * @class DeviceProfilePanel
 * @brief 设备配置选择 UI 面板
 */
class DeviceProfilePanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit DeviceProfilePanel(QWidget *parent = nullptr);

    /**
     * @brief 设置关联的注册表实例
     * @param registry 设备注册表指针
     */
    void setRegistry(DeviceRegistry *registry);

    /**
     * @brief 获取当前选中的设备配置
     * @return 设备配置（未选中时 name 为空）
     */
    DeviceProfile selectedProfile() const;

signals:
    /**
     * @brief 设备配置选中信号
     * @param profile 选中的配置
     */
    void profileSelected(const DeviceProfile &profile);

    /**
     * @brief 请求新建配置信号
     */
    void newProfileRequested();

private slots:
    /**
     * @brief 处理列表选择变更
     */
    void onSelectionChanged();

    /**
     * @brief 新建设备配置
     */
    void onNewProfile();

    /**
     * @brief 编辑选中的配置
     */
    void onEditProfile();

    /**
     * @brief 删除选中的配置
     */
    void onDeleteProfile();

    /**
     * @brief 导入配置文件
     */
    void onImportProfiles();

    /**
     * @brief 导出配置文件
     */
    void onExportProfiles();

    /**
     * @brief 刷新设备列表
     */
    void refreshList();

private:
    QListWidget *m_profileList;         ///< 设备列表
    QPushButton *m_newBtn;              ///< 新建按钮
    QPushButton *m_editBtn;             ///< 编辑按钮
    QPushButton *m_deleteBtn;           ///< 删除按钮
    QPushButton *m_importBtn;           ///< 导入按钮
    QPushButton *m_exportBtn;           ///< 导出按钮
    DeviceRegistry *m_registry = nullptr; ///< 关联的注册表
};

#endif // DEVICEPROFILEPANEL_H
