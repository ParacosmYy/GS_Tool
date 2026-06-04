# interfaces

> 纯虚接口与跨模块契约层。

## 说明

- 放 `IConnection`、`IPanelProvider`、`IDataSink`、`IProtocolParser` 等契约。
- 这里不放具体实现，只放抽象边界。
- 新能力如果要跨模块协作，优先先定义契约，再实现功能。
