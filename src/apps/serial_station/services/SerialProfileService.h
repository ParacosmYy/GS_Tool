#ifndef SERIAL_PROFILE_SERVICE_H
#define SERIAL_PROFILE_SERVICE_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/SerialStationModels.h"

namespace serial_station {

/**
 * @brief Serial Station 工站配置档案。
 *
 * 该值对象聚合 UART 参数、协议选择和常用命令，便于 JSON 导入导出和后续 UI 一键应用。
 */
struct SerialStationProfile {
    int schemaVersion = 1;          ///< JSON schema 版本，用于后续兼容迁移
    QString name;                   ///< 档案名称，必填
    QString description;            ///< 档案描述
    QStringList tags;               ///< 标签列表，用于后续筛选
    SerialPortConfig port;          ///< UART 配置，复用既有值对象
    QString protocolName = QStringLiteral("ascii_text"); ///< 默认协议名
    QString sendMode = QStringLiteral("ascii");          ///< 默认发送模式
    QVector<SerialProfileCommand> commands;              ///< 常用命令列表
};

/**
 * @brief Serial Station 档案读取/解析结果。
 */
struct SerialProfileResult {
    bool ok = false;                 ///< 解析是否成功
    SerialStationProfile profile;    ///< 成功时的档案
    QString errorMessage;            ///< 失败时的诊断信息
};

/**
 * @brief Serial Station 档案写入结果。
 */
struct SerialProfileWriteResult {
    bool ok = false;        ///< 写入是否成功
    QString filePath;       ///< 目标文件路径
    qint64 bytesWritten = 0; ///< 成功写入的字节数
    QString errorMessage;   ///< 失败时的诊断信息
};

/**
 * @brief Serial Station 服务层配置档案序列化器。
 *
 * 只负责档案校验、JSON 序列化和文件读写，不触碰 QWidget、串口线程或具体协议实现。
 */
class SerialProfileService {
public:
    /**
     * @brief 将档案序列化为 JSON 文本。
     * @param profile 输入档案
     * @param errorMessage 可选错误输出
     * @return 成功时返回缩进 JSON；失败时返回空字符串
     */
    QString toJson(const SerialStationProfile& profile, QString* errorMessage = nullptr) const;

    /**
     * @brief 从 JSON 文本解析档案。
     * @param json JSON 文本
     * @return 解析结果
     */
    SerialProfileResult fromJson(const QString& json) const;

    /**
     * @brief 保存档案到文件。
     * @param profile 输入档案
     * @param filePath 目标文件路径
     * @return 写入结果
     */
    SerialProfileWriteResult saveToFile(const SerialStationProfile& profile,
                                        const QString& filePath) const;

    /**
     * @brief 从文件加载档案。
     * @param filePath 源文件路径
     * @return 读取和解析结果
     */
    SerialProfileResult loadFromFile(const QString& filePath) const;

    /**
     * @brief 校验档案并返回错误原因。
     * @param profile 输入档案
     * @return 合法时为空，否则为可读错误描述
     */
    QString validationError(const SerialStationProfile& profile) const;

private:
    static QString normalizedProtocolName(const QString& protocolName);
    static QString normalizedSendMode(const QString& mode);
    static QString normalizedCommandMode(const QString& mode);
    static bool isSupportedProtocol(const QString& protocolName);
    static bool isSupportedMode(const QString& mode);
    static QString portParityName(QSerialPort::Parity parity);
    static QString portStopBitsName(QSerialPort::StopBits stopBits);
    static QString portFlowControlName(QSerialPort::FlowControl flowControl);
    static bool parseParity(const QString& text, QSerialPort::Parity& parity);
    static bool parseStopBits(const QString& text, QSerialPort::StopBits& stopBits);
    static bool parseFlowControl(const QString& text, QSerialPort::FlowControl& flowControl);
    static QJsonObject portToJson(const SerialPortConfig& port);
    static bool portFromJson(const QJsonObject& object,
                             SerialPortConfig& port,
                             QString& errorMessage);
    static QJsonObject commandToJson(const SerialProfileCommand& command);
    static bool commandFromJson(const QJsonObject& object,
                                int index,
                                SerialProfileCommand& command,
                                QString& errorMessage);
    static QJsonObject profileToJsonObject(const SerialStationProfile& profile);
    static bool profileFromJsonObject(const QJsonObject& object,
                                      SerialStationProfile& profile,
                                      QString& errorMessage);
    static QString makeError(const QString& message);
    static void setError(QString* target, const QString& message);
};

} // namespace serial_station

#endif // SERIAL_PROFILE_SERVICE_H
