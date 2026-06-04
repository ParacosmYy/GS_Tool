/**
 * @file WorkspaceManager.h
 * @brief 工作区管理器，管理面板布局的保存、加载和导入导出
 */
// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QElapsedTimer>

/**
 * @brief 工作区布局数据结构，保存面板状态和窗口几何信息
 */
struct WorkspaceLayout {
    QString name;                 ///< 工作区名称
    QVariantMap panelStates;      ///< 各面板的状态信息
    QStringList visiblePanels;    ///< 可见面板列表
    QString mainWindowState;      ///< 主窗口状态数据
    QString geometry;             ///< 窗口几何信息
};

/**
 * @class WorkspaceManager
 * @brief 工作区管理器，支持多工作区的保存/加载/删除和文件导入导出
 */
class WorkspaceManager : public QObject {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父对象指针 */
    explicit WorkspaceManager(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~WorkspaceManager() override;

    /** @brief 保存工作区布局 @param layout 布局数据 */
    void saveWorkspace(const WorkspaceLayout &layout);
    /** @brief 加载指定名称的工作区 @param name 工作区名称 @return 布局数据 */
    WorkspaceLayout loadWorkspace(const QString &name) const;
    /** @brief 删除指定工作区 @param name 工作区名称 */
    void deleteWorkspace(const QString &name);
    /** @brief 获取所有工作区名称 @return 名称列表 */
    QStringList workspaceNames() const;
    /** @brief 查询工作区是否存在 @param name 工作区名称 @return 是否存在 */
    bool exists(const QString &name) const;
    /** @brief 设置当前激活的工作区 @param name 工作区名称 */
    void setActiveWorkspace(const QString &name);
    /** @brief 获取当前激活的工作区名称 @return 工作区名称 */
    QString activeWorkspace() const;

    /** @brief 将工作区导出到文件 @param name 工作区名称 @param filePath 目标文件路径 */
    void exportToFile(const QString &name, const QString &filePath) const;
    /** @brief 从文件导入工作区 @param filePath 源文件路径 @return 是否导入成功 */
    bool importFromFile(const QString &filePath);

signals:
    /** @brief 工作区保存完成时发射 @param name 工作区名称 */
    void workspaceSaved(const QString &name);
    /** @brief 工作区加载完成时发射 @param name 工作区名称 */
    void workspaceLoaded(const QString &name);
    /** @brief 工作区被删除时发射 @param name 工作区名称 */
    void workspaceDeleted(const QString &name);
    /** @brief 当前激活工作区切换时发射 @param name 新激活的工作区名称 */
    void activeWorkspaceChanged(const QString &name);

private:
    QMap<QString, WorkspaceLayout> m_workspaces;  ///< 工作区名称到布局的映射
    QString m_activeWorkspace;                      ///< 当前激活的工作区名称

    // ---- 统计计数器 ----
    quint64 m_totalSaves = 0;                ///< 总保存次数
    mutable quint64 m_totalLoads = 0;        ///< 总加载次数
    quint64 m_totalDeletions = 0;            ///< 总删除次数
    quint64 m_totalSwitches = 0;             ///< 总切换次数
    mutable quint64 m_totalExportFiles = 0;  ///< 总导出文件次数
    mutable quint64 m_totalImportFiles = 0;  ///< 总导入文件次数
    quint64 m_totalWorkspacesCreated = 0;    ///< 总新建工作区次数
    quint64 m_totalWorkspacesDeleted = 0;    ///< 总删除工作区次数
    quint64 m_activeWorkspaceTimeMs = 0;     ///< 当前激活工作区累计活跃时长(毫秒)
    mutable quint64 m_totalExportErrors = 0;  ///< 累计导出失败次数(在const方法中更新)
    quint64 m_totalImportErrors = 0;          ///< 累计导入失败次数
    mutable quint64 m_totalLoadMisses = 0;    ///< 累计加载不存在工作区次数
    quint64 m_totalOverwriteSaves = 0;        ///< 累计覆盖已存在工作区的保存次数
    QElapsedTimer m_activeTimer;             ///< 当前激活工作区计时器

public:
    /** @brief 获取总保存次数 @return 累计保存次数 */
    quint64 totalSaves() const { return m_totalSaves; }
    /** @brief 获取总加载次数 @return 累计加载次数 */
    quint64 totalLoads() const { return m_totalLoads; }
    /** @brief 获取总删除次数 @return 累计删除次数 */
    quint64 totalDeletions() const { return m_totalDeletions; }
    /** @brief 获取总切换次数 @return 累计切换次数 */
    quint64 totalSwitches() const { return m_totalSwitches; }
    /** @brief 获取总导出文件次数 @return 累计导出次数 */
    quint64 totalExportFiles() const { return m_totalExportFiles; }
    /** @brief 获取总导入文件次数 @return 累计导入次数 */
    quint64 totalImportFiles() const { return m_totalImportFiles; }
    /** @brief 获取总新建工作区次数 @return 累计新建次数 */
    quint64 totalWorkspacesCreated() const { return m_totalWorkspacesCreated; }
    /** @brief 获取总删除工作区次数 @return 累计删除次数 */
    quint64 totalWorkspacesDeleted() const { return m_totalWorkspacesDeleted; }
    /** @brief 获取当前激活工作区累计活跃时长(毫秒) @return 活跃时长毫秒数 */
    quint64 activeWorkspaceTimeMs() const;

    /** @brief 获取累计导出失败次数(文件写入错误) @return 导出错误总数 */
    quint64 totalExportErrors() const { return m_totalExportErrors; }

    /** @brief 获取累计导入失败次数(文件读取/解析错误) @return 导入错误总数 */
    quint64 totalImportErrors() const { return m_totalImportErrors; }

    /** @brief 获取累计加载不存在工作区次数 @return 加载未命中次数 */
    quint64 totalLoadMisses() const { return m_totalLoadMisses; }

    /** @brief 获取累计覆盖已存在工作区的保存次数 @return 覆盖保存次数 */
    quint64 totalOverwriteSaves() const { return m_totalOverwriteSaves; }

    /** @brief 重置工作区统计计数器(包含所有计数器归零) */
    void resetWorkspaceStatistics();
};
