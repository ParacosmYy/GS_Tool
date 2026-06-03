/**
 * @file MqttConnection.h
 * @brief MQTT瀹㈡埛绔繛鎺ュ疄鐜?鈥?鍩轰簬QTcpSocket瀹炵幇MQTT v3.1.1绾垮崗璁? *
 * 鑱岃矗: MQTT杩炴帴绠＄悊銆佹秷鎭彂甯?璁㈤槄銆並eepAlive蹇冭烦锛? * 閫氳繃IConnection缁熶竴鎺ュ彛渚涗笂灞備娇鐢ㄣ€? */
#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include "connection/interface/IConnection.h"
#include <QTcpSocket>
#include <QTimer>
#include <QStringList>

/**
 * @brief MQTT瀹㈡埛绔繛鎺ュ疄鐜? *
 * 灏佽MQTT v3.1.1鍗忚閫氫俊锛屽熀浜嶲TcpSocket瀹炵幇鍘熷绾垮崗璁€? * 鏀寔CONNECT/PUBLISH/SUBSCRIBE/UNSUBSCRIBE/PINGREQ绛夊熀鏈姤鏂囩被鍨嬨€? */
class MqttConnection : public IConnection {
    Q_OBJECT

public:
    /**
     * @brief 鏋勯€燤QTT杩炴帴
     * @param parent 鐖跺璞?     */
    explicit MqttConnection(QObject* parent = nullptr);

    /** @brief 鏋愭瀯鍑芥暟锛岃嚜鍔ㄦ柇寮€杩炴帴 */
    ~MqttConnection() override;

    // ---- IConnection 鎺ュ彛瀹炵幇 ----

    /** @brief 杩斿洖杩炴帴绫诲瀷 */
    ConnectionType type() const override;

    /** @brief 杩斿洖MQTT鏈嶅姟鍣ㄥ湴鍧€ */
    QString name() const override;

    /** @brief 杩斿洖褰撳墠杩炴帴鐘舵€?*/
    ConnectionState state() const override;

    /** @brief 鎵撳紑MQTT杩炴帴 */
    bool open() override;

    /** @brief 鍏抽棴MQTT杩炴帴 */
    void close() override;

    /**
     * @brief 鍐欏叆(鍙戝竷)鏁版嵁鍒伴粯璁や富棰?     * @param data 寰呭彂甯冪殑鏁版嵁
     * @return 瀹為檯鍐欏叆瀛楄妭鏁帮紝-1琛ㄧず澶辫触
     */
    qint64 write(const QByteArray& data) override;

    /**
     * @brief 閫氳繃鍙傛暟鏄犲皠閰嶇疆MQTT杩炴帴
     * @param params 鏀寔鐨刱ey: host, port, clientId, username, password, keepAlive, cleanSession
     */
    void configure(const QVariantMap& params) override;

    // ---- MQTT涓撶敤鎺ュ彛 ----

    /**
     * @brief 杩炴帴鍒癕QTT鏈嶅姟鍣?     * @param host 鏈嶅姟鍣ㄥ湴鍧€
     * @param port 绔彛鍙?     */
    void connectToHost(const QString& host, int port);

    /** @brief 鏂紑MQTT鏈嶅姟鍣ㄨ繛鎺?*/
    void disconnectFromHost();

    /**
     * @brief 鍙戝竷娑堟伅鍒版寚瀹氫富棰?     * @param topic 鐩爣涓婚
     * @param payload 娑堟伅璐熻浇
     * @param qos 鏈嶅姟璐ㄩ噺绛夌骇(0/1/2)
     * @return true=鍙戝竷鎴愬姛
     */
    bool publish(const QString& topic, const QByteArray& payload, int qos = 0);

    /**
     * @brief 璁㈤槄涓婚
     * @param topic 璁㈤槄涓婚(鏀寔閫氶厤绗?鍜?)
     * @param qos 鏈嶅姟璐ㄩ噺绛夌骇
     * @return true=璁㈤槄璇锋眰宸插彂閫?     */
    bool subscribe(const QString& topic, int qos = 0);

    /**
     * @brief 鍙栨秷璁㈤槄涓婚
     * @param topic 瑕佸彇娑堢殑涓婚
     */
    void unsubscribe(const QString& topic);

    /** @brief 鑾峰彇宸插彂甯冩秷鎭鏁?*/
    quint64 publishCount() const;

    /** @brief 鑾峰彇宸叉帴鏀舵秷鎭鏁?*/
    quint64 receivedCount() const;

    /** @brief 鑾峰彇宸茶闃呬富棰樻暟閲?*/
    int subscriptionCount() const;

    /** @brief 閲嶇疆娑堟伅璁℃暟缁熻 */
    void resetStatistics();

signals:
    /**
     * @brief 鏀跺埌MQTT娑堟伅
     * @param topic 娑堟伅涓婚
     * @param payload 娑堟伅璐熻浇
     */
    void messageReceived(const QString& topic, const QByteArray& payload);

    /** @brief MQTT杩炴帴宸插缓绔?*/
    void connected();

    /** @brief MQTT杩炴帴宸叉柇寮€ */
    void disconnected();

private slots:
    /** @brief TCP socket鏁版嵁鍒拌揪澶勭悊 */
    void onSocketReadyRead();

    /** @brief TCP杩炴帴寤虹珛鍚庡彂閫丮QTT CONNECT */
    void onSocketConnected();

    /** @brief TCP杩炴帴鏂紑澶勭悊 */
    void onSocketDisconnected();

    /** @brief 鍙戦€丳INGREQ淇濇椿 */
    void onKeepAlive();

private:
    /**
     * @brief 鏋勫缓MQTT鍥哄畾澶?     * @param packetType 鎶ユ枃绫诲瀷(1=CONNECT, 3=PUBLISH, 8=SUBSCRIBE, ...)
     * @param payload 鍙橀暱澶?璐熻浇
     * @return 瀹屾暣MQTT鎶ユ枃
     */
    QByteArray buildMqttPacket(quint8 packetType, const QByteArray& payload);

    /** @brief 缂栫爜鍓╀綑闀垮害瀛楁 */
    QByteArray encodeRemainingLength(int length);

    /** @brief 瑙ｆ瀽鏀跺埌鐨凪QTT鎶ユ枃骞跺垎鍙?*/
    void parseIncomingPacket();

    /** @brief 鍙戦€丮QTT CONNECT鎶ユ枃 */
    void sendConnect();

    /** @brief 澶勭悊CONNACK鎶ユ枃 */
    void handleConnack(const QByteArray& data);

    /** @brief 澶勭悊PUBLISH鎶ユ枃(鏈嶅姟鍣ㄦ帹閫? */
    void handlePublish(const QByteArray& data, quint8 flags);

    /** @brief 澶勭悊SUBACK鎶ユ枃 */
    void handleSuback(const QByteArray& data);

    /** @brief 鐢熸垚鑷姩瀹㈡埛绔疘D */
    QString generateClientId();

    /** @brief MQTT鏈嶅姟鍣ㄥ湴鍧€ */
    QString m_host;

    /** @brief MQTT鏈嶅姟鍣ㄧ鍙ｏ紝榛樿1883 */
    int m_port = 1883;

    /** @brief 瀹㈡埛绔疘D */
    QString m_clientId;

    /** @brief 璁よ瘉鐢ㄦ埛鍚?*/
    QString m_username;

    /** @brief 璁よ瘉瀵嗙爜 */
    QString m_password;

    /** @brief KeepAlive闂撮殧(绉?锛岄粯璁?0 */
    int m_keepAliveInterval = 60;

    /** @brief Clean Session鏍囧織 */
    bool m_cleanSession = true;

    /** @brief 鎶ユ枃鏍囪瘑绗﹁鏁板櫒(鐢ㄤ簬SUB/UNSUB) */
    quint16 m_packetId = 0;

    /** @brief TCP socket */
    QTcpSocket* m_socket;

    /** @brief KeepAlive蹇冭烦瀹氭椂鍣?*/
    QTimer* m_keepAlive;

    /** @brief 褰撳墠杩炴帴鐘舵€?*/
    ConnectionState m_state = ConnectionState::Disconnected;

    /** @brief 鎺ユ敹缂撳啿鍖?*/
    QByteArray m_rxBuffer;

    /** @brief 鏈熸湜鐨勫墿浣欓暱搴?瑙ｆ瀽涓棿鐘舵€? */
    int m_expectedLength = -1;

    /** @brief 宸插彂甯冩秷鎭鏁?*/
    quint64 m_publishCount = 0;
    /** @brief 宸叉帴鏀舵秷鎭鏁?*/
    quint64 m_receivedCount = 0;
    /** @brief 宸茶闃呬富棰橀泦鍚?*/
    QStringList m_subscriptions;
};

#endif // MQTTCONNECTION_H
