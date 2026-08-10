# ADR-052：Provider 响应投影边界

## 状态

已接受（2026-08-10）。

## 背景

上游 OpenAI-compatible 响应可能包含隐藏推理、工具参数、供应商 metadata、回显内容和
未冻结的扩展字段。将完整 JSON 通过 Web 或 Android 返回，会把第三方响应结构误当成
本项目公共契约，也扩大敏感内容的传播范围。

## 决策

- 新增 `provider_projection.py`，在 Application provider 用例返回客户端之前执行有界投影。
- `response` 只保留可选 `id`、`model`、第一个 choice 的助手文本和 `finish_reason`。
- 助手文本最多 20,000 字符；模型/响应 ID 有单独长度上限；content-part 数组只提取
  `text` 字段。
- `usage`、`recorded`、`replayed`、`record`、`warning` 继续作为本项目自己的稳定字段，
  不从上游响应原样透传。
- Web 和 Android 继续读取 `choices[0].message.content`，因此此次收敛不改变当前 UI
  和移动端的公开读取路径；供应商私有字段不属于兼容契约。

## 验证边界

- 使用无网络、无真实 Key 的内存对象 smoke 检查字符串、content parts、超长文本和非对象
  响应；不创建用户、记录或测试专用资产。
- Python 编译、源文件 1000 行门禁、API 契约引用和只读发布审计必须通过。
