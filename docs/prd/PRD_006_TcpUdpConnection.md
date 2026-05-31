# PRD-006: TCP Client/Server + UDP 网络连接

## 背景
目前只支持串口连接，需要支持TCP和UDP网络通信，用于调试远程设备或网络协议。所有连接类型必须复用IConnection抽象接口。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | TcpConnection TCP Client模式（连接远程设备） | P0 | connection/ |
| R2 | TcpConnection TCP Server模式（监听端口等待连接） | P0 | connection/ |
| R3 | UdpConnection UDP收发（广播/单播） | P1 | connection/ |
| R4 | ConnectionFactory扩展支持TCP/UDP创建 | P0 | core/ |
| R5 | 导航树增加TCP/UDP连接节点 | P1 | core/ |
| R6 | 网络连接配置面板（IP/端口/模式） | P1 | serial/ 或 core/ |

## 接口设计

### TcpConnection
```cpp
class TcpConnection : public IConnection {
    Q_OBJECT
public:
    // Client模式: connectToHost(host, port)
    // Server模式: listen(port)
    void configure(const QVariantMap& params) override;
    bool open() override;
    void close() override;
    qint64 write(const QByteArray& data) override;
    // params: mode=client/server, host, port
};
```

### UdpConnection
```cpp
class UdpConnection : public IConnection {
    Q_OBJECT
public:
    void configure(const QVariantMap& params) override;
    // params: localPort, remoteHost, remotePort, broadcast
};
```

## 设计模式
- **策略模式 (Strategy)**: TcpConnection和UdpConnection是IConnection策略的不同实现
- **工厂模式 (Factory)**: ConnectionFactory根据ConnectionType创建具体实例

## 依赖的公共组件
- `IConnection` — 连接抽象接口
- `ConnectionFactory` — 工厂模式创建连接
- `ConnectionManager` — 连接生命周期管理
- `SettingsManager` — 网络配置持久化

## 验收标准
1. TCP Client能连接远程服务器，收发数据
2. TCP Server能监听端口，接受客户端连接
3. UDP能发送和接收数据报
4. 通过IConnection接口操作，MainWindow无强转
5. ConnectionFactory根据类型创建正确的连接实例
