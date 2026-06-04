/**
 * @file PacketTemplateLib.h
 * @brief 报文模板库控件 -- 常用协议报文模板管理与一键发送
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 内置 Modbus RTU / SPI / CAN / UART AT 等常用协议报文模板，
 * 支持参数自定义、校验和自动计算、一键发送。
 * 依赖: CRC (utils/crypto/CRC.h)
 */

#ifndef PACKETTEMPLATELIB_H
#define PACKETTEMPLATELIB_H

#include <QByteArray>
#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QWidget>

/**
 * @class PacketTemplateLib
 * @brief 报文模板库控件，提供内置协议模板的浏览、参数编辑和发送
 *
 * 面板左侧为分类树+搜索，右侧为参数表+Hex预览+操作按钮。
 * 通过 packetSendRequested 信号通知外部发送数据。
 */
class PacketTemplateLib : public QWidget {
    Q_OBJECT

public:
    /** @brief 模板分类枚举 */
    enum class TemplateCategory {
        ModbusRtu,  ///< Modbus RTU
        Spi,        ///< SPI 命令
        Can,        ///< CAN 帧
        UartAt,     ///< UART AT 命令
        Custom      ///< 自定义
    };
    Q_ENUM(TemplateCategory)

    /** @brief 模板参数结构 */
    struct TemplateParam {
        QString name;       ///< 参数名称
        QString value;      ///< 默认值（十六进制字符串）
        int byteOffset;     ///< 在帧模板中的字节偏移
        int byteLength;     ///< 参数占用的字节长度
        bool isHex;         ///< 是否为十六进制格式
    };

    /** @brief 报文模板结构 */
    struct PacketTemplate {
        QString name;               ///< 模板名称
        QString description;        ///< 模板描述
        TemplateCategory category;  ///< 所属分类
        QByteArray frameTemplate;   ///< 帧模板（0xFF = 参数占位符）
        QList<TemplateParam> params;///< 可编辑参数列表
        bool hasChecksum;           ///< 是否需要计算校验和
        int checksumOffset;         ///< 校验和在帧中的起始偏移
    };

    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalTemplatesLoaded = 0;  ///< 累计加载模板数
        quint64 totalSends = 0;            ///< 累计发送次数
        quint64 totalBytesSent = 0;        ///< 累计发送字节数
        quint64 totalEdits = 0;            ///< 累计参数编辑次数
        int peakTemplatesPerCategory = 0;  ///< 单分类最大模板数
    };

    /**
     * @brief 构造函数
     * @param parent 父控件
     */
    explicit PacketTemplateLib(QWidget *parent = nullptr);

    /** @brief 加载内置协议模板（Modbus/SPI/CAN/AT） */
    void loadBuiltinTemplates();

    /**
     * @brief 从 JSON 文件加载自定义模板
     * @param filePath JSON 文件路径
     * @return true 加载成功
     */
    bool loadFromJson(const QString &filePath);

    /**
     * @brief 根据当前参数值构建报文字节数组
     * @return 构建后的报文数据
     */
    QByteArray buildPacket();

    /** @brief 获取统计信息 @return 统计快照 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 请求发送报文 @param data 发送的字节数据 */
    void packetSendRequested(const QByteArray &data);

    /** @brief 模板被选中 @param name 模板名称 */
    void templateSelected(const QString &name);

private slots:
    void onTemplateClicked();   ///< 模板树节点被点击
    void onParamChanged();      ///< 参数表格内容变化
    void onSendClicked();       ///< 发送按钮点击
    void onCopyClicked();       ///< 复制按钮点击
    void onSearchChanged();     ///< 搜索框内容变化
    void onCategoryFilterChanged(); ///< 分类过滤切换

private:
    void setupUI();             ///< 初始化界面布局
    void populateTree();        ///< 填充模板分类树
    void loadTemplateParams(const PacketTemplate &tmpl); ///< 加载模板参数到表格
    QByteArray calculateChecksum(const QByteArray &data, int offset) const; ///< 计算校验和
    void updatePreview();       ///< 刷新 Hex 预览
    void createBuiltinModbusTemplates();  ///< 创建 Modbus 内置模板
    void createBuiltinSpiTemplates();     ///< 创建 SPI 内置模板
    void createBuiltinCanTemplates();     ///< 创建 CAN 内置模板
    void createBuiltinAtTemplates();      ///< 创建 AT 内置模板

    // ---- UI 控件 ----
    QSplitter *m_splitter;          ///< 左右分割器
    QTreeWidget *m_templateTree;    ///< 模板分类树
    QTableWidget *m_paramTable;     ///< 参数编辑表格
    QTextEdit *m_previewEdit;       ///< Hex 预览区
    QPushButton *m_sendBtn;         ///< 发送按钮
    QPushButton *m_copyBtn;         ///< 复制按钮
    QComboBox *m_categoryCombo;     ///< 分类过滤下拉框
    QLineEdit *m_searchEdit;        ///< 搜索输入框
    QLabel *m_statusLabel;          ///< 状态提示标签

    // ---- 数据 ----
    QList<PacketTemplate> m_templates;      ///< 所有模板列表
    int m_currentTemplateIndex = -1;        ///< 当前选中模板索引

    Stats m_stats;                          ///< 统计计数器
};

#endif // PACKETTEMPLATELIB_H
