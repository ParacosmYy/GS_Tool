/**
 * @file YModemTransferHandlers.cpp
 * @brief YMODEM协议状态处理器 - 转发文件
 *
 * 原始实现已拆分为两个文件:
 *   - YModemTransferHandlersEarly.cpp: 前4个状态(WaitingStart/SendingBlock0/SendingData/SendingEOT)
 *   - YModemTransferHandlersFinal.cpp: 后3个状态(WaitBlock0Ack/WaitFinalC/SendingFinalBlock0)
 *
 * 保留本文件以保持向后兼容的文件索引。所有实现代码已迁移至上述两个文件。
 */

// 所有状态处理器实现见:
//   YModemTransferHandlersEarly.cpp — 数据传输阶段
//   YModemTransferHandlersFinal.cpp — 批量切换与会话结束阶段
