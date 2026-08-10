/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Immutable cross-platform models for the Android data boundary.
 * Module: Android data / domain-facing contracts.
 */
package com.aitokentracker.data

/** Public account projection returned by Windows `/api/v1`. */
data class UserProfile(
    val id: Long,
    val username: String,
    val role: String,
)

/** Short-lived access and rotating refresh material kept behind the repository. */
internal data class TokenPair(
    val accessToken: String,
    val refreshToken: String,
    val expiresInSeconds: Long,
    val user: UserProfile,
)

/** Encrypted session snapshot used by the secure local store. */
internal data class SessionSnapshot(
    val accessToken: String,
    val refreshToken: String,
    val expiresAtEpochMillis: Long,
    val user: UserProfile,
)

/** Aggregate token totals for a selected local-time period. */
data class UsageTotals(
    val calls: Long,
    val inputTokens: Long,
    val outputTokens: Long,
    val totalTokens: Long,
)

/** Token totals grouped by the model returned by the Windows service. */
data class ModelUsage(
    val model: String,
    val calls: Long,
    val inputTokens: Long,
    val outputTokens: Long,
    val totalTokens: Long,
)

/** Daily token totals used by the mobile trend surface. */
data class DailyUsage(
    val day: String,
    val inputTokens: Long,
    val outputTokens: Long,
    val totalTokens: Long,
)

/** A persisted token record; raw provider payloads are intentionally absent. */
data class UsageRecord(
    val id: Long,
    val model: String,
    val inputTokens: Long,
    val outputTokens: Long,
    val totalTokens: Long,
    val timestamp: String,
    val note: String,
    val source: String,
)

/** User-entered usage payload; timestamp is server-normalized when omitted. */
data class UsageRecordDraft(
    val model: String,
    val inputTokens: Long,
    val outputTokens: Long,
    val note: String = "",
)

/** Sanitized model-discovery result; provider keys never cross this model. */
data class ProviderModels(
    val provider: String,
    val models: List<String>,
)

/** Sanitized result of one automatic provider call. */
data class ProviderCallResult(
    val provider: String,
    val assistantText: String,
    val usageSummary: String,
    val record: UsageRecord?,
    val recorded: Boolean,
    val replayed: Boolean,
    val warning: String?,
)

/** One summary response from `GET /api/v1/me/summary`. */
data class UsageSummary(
    val period: String,
    val from: String,
    val to: String,
    val totals: UsageTotals,
    val byModel: List<ModelUsage>,
    val trend: List<DailyUsage>,
    val records: List<UsageRecord>,
)

/** Input for a structured work event; no raw prompt or API key is permitted. */
data class WorkEventDraft(
    val direction: String,
    val outcome: String,
    val durationMs: Long? = null,
    val efficiencyScore: Int? = null,
    val resultCode: String? = null,
    val errorCode: String? = null,
    val project: String = "",
    val taskType: String = "",
    val note: String = "",
)

/** Input for a bounded diagnostic log accepted by the Windows service. */
data class LogDraft(
    val level: String = "info",
    val eventType: String,
    val message: String,
    val errorCode: String? = null,
    val metadata: Map<String, String> = emptyMap(),
)

/** Server pagination metadata for future history and log screens. */
data class PageInfo(
    val limit: Int,
    val offset: Int,
    val total: Int,
)

/** A bounded personal token-record page returned by the Windows service. */
data class UsageRecordsPage(
    val period: String,
    val from: String,
    val to: String,
    val records: List<UsageRecord>,
    val page: PageInfo,
)

/** A sanitized work event projection returned by the service. */
data class WorkEvent(
    val id: Long,
    val direction: String,
    val outcome: String,
    val durationMs: Long?,
    val efficiencyScore: Int?,
    val resultCode: String?,
    val errorCode: String?,
    val project: String,
    val taskType: String,
    val note: String,
    val createdAt: String,
)

/** A sanitized diagnostic log projection returned by the service. */
data class AppLog(
    val id: Long,
    val requestId: String,
    val level: String,
    val eventType: String,
    val message: String,
    val errorCode: String?,
    val createdAt: String,
)
