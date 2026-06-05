/**
 * @file FrameTemplateLibrary.h
 * @brief 帧模板库 — 可复用协议帧模板的增删查改、帧组装与校验和计算
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 管理用户自定义的协议帧模板，每个模板包含帧头、帧尾、字段列表及
 * 校验和配置。调用 buildFrame() 即可按模板组装出完整的二进制帧，
 * 并自动在指定偏移位置填入校验值。
 *
 * 支持的校验算法: XOR8 / Sum8 / CRC16-Modbus / CRC32。
 * 模板库可整体导出/导入为 JSON 文件，便于团队共享。
 *
 * 依赖: CRC (utils/crypto/CRC.h)
 */

#ifndef FRAMETEMPLATELIBRARY_H
#define FRAMETEMPLATELIBRARY_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @class FrameTemplateLibrary
 * @brief 帧模板库，管理可复用协议帧模板并提供一键帧组装
 *
 * 典型用法:
 * @code
 *   FrameTemplateLibrary lib;
 *   FrameTemplate tmpl;
 *   tmpl.name = "ModbusReadHolding";
 *   tmpl.header = QByteArray::fromHex("01030000");
 *   tmpl.footer = QByteArray();
 *   tmpl.checksumType = "CRC16-Modbus";
 *   tmpl.checksumOffset = 4;
 *   lib.addTemplate(tmpl);
 *   QByteArray frame = lib.buildFrame("ModbusReadHolding");
 * @endcode
 */
class FrameTemplateLibrary : public QObject {
    Q_OBJECT

public:
    /** @brief 字段值结构 — 描述模板中一个可填充字段 */
    struct FieldValue {
        QString name;           ///< 字段名称（如 "寄存器地址"）
        QByteArray value;       ///< 默认值（二进制）
        QString description;    ///< 字段用途说明
    };

    /** @brief 帧模板结构 — 定义一帧的完整组装规则 */
    struct FrameTemplate {
        QString name;               ///< 模板名称（唯一标识）
        QString category;           ///< 所属分类（如 "Modbus"/"自定义"）
        QByteArray header;          ///< 帧头固定字节
        QByteArray footer;          ///< 帧尾固定字节
        QList<FieldValue> fields;   ///< 可变字段列表（按顺序拼接）
        QString checksumType;       ///< 校验算法: "XOR8"/"Sum8"/"CRC16-Modbus"/"CRC32"/""(无)
        int checksumOffset;         ///< 校验值写入位置（相对于帧起始的字节偏移，-1=自动追加尾部）
        QString description;        ///< 模板用途描述
    };

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalTemplates = 0;     ///< 当前模板总数
        quint64 totalFramesBuilt = 0;   ///< 累计构建帧数
        quint64 totalImports = 0;       ///< 累计导入次数
        quint64 totalExports = 0;       ///< 累计导出次数
        quint64 totalCategories = 0;    ///< 当前分类总数
        quint64 maxFieldsInTemplate = 0;///< 单模板最大字段数
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FrameTemplateLibrary(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~FrameTemplateLibrary() override;

    // ---- 模板 CRUD ----

    /**
     * @brief 添加模板
     * @param tmpl 模板定义
     * @return true 添加成功，false 同名模板已存在
     */
    bool addTemplate(const FrameTemplate &tmpl);

    /**
     * @brief 移除模板
     * @param name 模板名称
     * @return true 移除成功，false 模板不存在
     */
    bool removeTemplate(const QString &name);

    /**
     * @brief 按名称查询模板
     * @param name 模板名称
     * @return 模板指针（不存在返回 nullptr），生命周期随本对象
     */
    const FrameTemplate *template_(const QString &name) const;

    /** @brief 获取所有模板列表 @return 模板列表的引用 */
    const QList<FrameTemplate> &allTemplates() const;

    /** @brief 获取所有分类名称 @return 去重后的分类列表 */
    QStringList categories() const;

    /**
     * @brief 按分类筛选模板
     * @param category 分类名称
     * @return 匹配的模板列表
     */
    QList<FrameTemplate> findByCategory(const QString &category) const;

    // ---- 帧组装 ----

    /**
     * @brief 使用模板默认值构建帧
     * @param templateName 模板名称
     * @return 组装后的完整帧数据，失败返回空 QByteArray
     */
    QByteArray buildFrame(const QString &templateName);

    /**
     * @brief 使用自定义字段值构建帧
     * @param templateName 模板名称
     * @param fieldValues 字段值列表（name 匹配模板字段名）
     * @return 组装后的完整帧数据，失败返回空 QByteArray
     */
    QByteArray buildFrameWithValues(const QString &templateName,
                                    const QList<FieldValue> &fieldValues);

    // ---- 导入 / 导出 ----

    /**
     * @brief 将整个模板库导出为 JSON 文件
     * @param filePath 目标文件路径
     * @return true 导出成功
     */
    bool exportLibrary(const QString &filePath) const;

    /**
     * @brief 从 JSON 文件导入模板（追加到现有库）
     * @param filePath JSON 文件路径
     * @return true 导入成功
     */
    bool importLibrary(const QString &filePath);

    // ---- 统计 ----

    /** @brief 获取统计快照 @return Stats 常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 模板已添加 @param name 模板名称 */
    void templateAdded(const QString &name);

    /** @brief 模板已移除 @param name 模板名称 */
    void templateRemoved(const QString &name);

    /** @brief 帧已构建 @param name 使用的模板名称 @param frame 构建结果 */
    void frameBuilt(const QString &name, const QByteArray &frame);

private:
    /**
     * @brief 计算校验和并写入帧数据
     * @param frame 帧数据（会被原地修改）
     * @param checksumType 校验算法名称
     * @param checksumOffset 写入偏移（-1=尾部追加）
     */
    void applyChecksum(QByteArray &frame,
                       const QString &checksumType,
                       int checksumOffset) const;

    /**
     * @brief 计算指定算法的校验和字节数
     * @param checksumType 校验算法名称
     * @return 校验值占用字节数（0=无校验）
     */
    int checksumSize(const QString &checksumType) const;

    /** @brief 刷新内部统计缓存（totalTemplates/totalCategories/maxFieldsInTemplate） */
    void refreshStats();

    // ---- 数据 ----
    QMap<QString, FrameTemplate> m_templates;  ///< 模板名称 → 模板定义
    mutable Stats m_stats;                              ///< 统计计数器
};

#endif // FRAMETEMPLATELIBRARY_H
