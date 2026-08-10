/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Stable network boundary between repository logic and HTTP details.
 * Module: Android data / remote source.
 */
package com.aitokentracker.data.remote

import com.aitokentracker.data.AppLog
import com.aitokentracker.data.DailyUsage
import com.aitokentracker.data.LogDraft
import com.aitokentracker.data.ModelUsage
import com.aitokentracker.data.PageInfo
import com.aitokentracker.data.TokenPair
import com.aitokentracker.data.UsageRecord
import com.aitokentracker.data.UsageRecordDraft
import com.aitokentracker.data.UsageTotals
import com.aitokentracker.data.WorkEvent
import com.aitokentracker.data.WorkEventDraft

/**
 * HTTP operations required by the first cross-device slice.
 * Implementations must be main-safe when called from the repository executor.
 */
internal interface TrackerRemoteDataSource {
    /** Change the server endpoint before the next request is issued. */
    fun configureBaseUrl(baseUrl: String)

    /** Authenticate one account; callers must keep the password transient. */
    fun login(username: String, password: String): TokenPair

    /** Rotate a refresh token and return the replacement bearer pair. */
    fun refresh(refreshToken: String): TokenPair

    /** Revoke the presented access token; network failure is surfaced to the repository. */
    fun logout(accessToken: String)

    /** Read the authenticated user's server-owned summary projection. */
    fun summary(accessToken: String, period: String): RemoteSummary

    /** Discover allowlisted provider models without persisting the submitted key. */
    fun providerModels(accessToken: String, provider: String, baseUrl: String, apiKey: String): RemoteProviderModels

    /** Proxy one non-streaming provider call and return only its bounded projection. */
    fun proxyChat(
        accessToken: String,
        provider: String,
        baseUrl: String,
        apiKey: String,
        model: String,
        prompt: String,
        note: String,
        idempotencyKey: String,
    ): RemoteProviderCall

    fun listUsageRecords(
        accessToken: String,
        period: String,
        limit: Int,
        offset: Int,
    ): RemoteUsageRecords

    /** Write one usage record with a stable key so auth retries are idempotent. */
    fun createUsageRecord(
        accessToken: String,
        record: UsageRecordDraft,
        idempotencyKey: String,
    ): UsageRecord

    /** Write one structured work event under the authenticated account. */
    fun createWorkEvent(accessToken: String, event: WorkEventDraft, idempotencyKey: String): WorkEvent

    /** Read a bounded page of the authenticated user's work events. */
    fun listWorkEvents(accessToken: String, limit: Int, offset: Int): RemoteWorkEvents

    /** Write one privacy-filtered diagnostic log under the authenticated account. */
    fun createLog(accessToken: String, log: LogDraft): AppLog

    /** Read a bounded page of the authenticated user's diagnostic logs. */
    fun listLogs(accessToken: String, limit: Int, offset: Int): RemoteLogs

    /** Administrator-only read models; the server enforces the role again. */
    fun adminOverview(accessToken: String): RemoteAdminOverview

    /** Read a bounded administrator member aggregate page. */
    fun adminMembers(accessToken: String, limit: Int, offset: Int): RemoteAdminMembers

    /** Read one selected member's bounded activity after server-side RBAC. */
    fun adminMemberActivity(accessToken: String, userId: Long, limit: Int): RemoteAdminMemberActivity
}

/** Remote summary DTO kept separate from the domain-facing model. */
internal data class RemoteSummary(
    val period: String,
    val from: String,
    val to: String,
    val totals: UsageTotals,
    val byModel: List<ModelUsage>,
    val trend: List<DailyUsage>,
    val records: List<UsageRecord>,
)

/** Remote model list returned by the bearer provider boundary. */
internal data class RemoteProviderModels(
    val provider: String,
    val models: List<String>,
)

/** Remote provider result already reduced to a UI-safe projection. */
internal data class RemoteProviderCall(
    val provider: String,
    val assistantText: String,
    val usageSummary: String,
    val record: UsageRecord?,
    val recorded: Boolean,
    val replayed: Boolean,
    val warning: String?,
)

/** Paged personal token records matching `GET /api/v1/records`. */
internal data class RemoteUsageRecords(
    val period: String,
    val from: String,
    val to: String,
    val records: List<UsageRecord>,
    val page: PageInfo,
)

/** Paged work-event response matching `/api/v1/events/work`. */
internal data class RemoteWorkEvents(
    val items: List<WorkEvent>,
    val page: PageInfo,
)

/** Paged diagnostic-log response matching `/api/v1/logs`. */
internal data class RemoteLogs(
    val items: List<AppLog>,
    val page: PageInfo,
)

/** Remote DTO for `GET /api/v1/admin/overview`. */
internal data class RemoteAdminOverview(
    val totalUsers: Long,
    val usage: UsageTotals,
    val workEvents: RemoteAdminWorkEventTotals,
    val logCount: Long,
    val byModel: List<ModelUsage>,
    val trend: List<DailyUsage>,
)

/** Success/failure aggregate nested in the administrator overview. */
internal data class RemoteAdminWorkEventTotals(
    val total: Long,
    val success: Long,
    val failure: Long,
)

/** Remote DTO for the top-level `items/limit/offset/total` member response. */
internal data class RemoteAdminMembers(
    val items: List<RemoteAdminMember>,
    val page: PageInfo,
)

/** Non-secret member aggregate returned only through the admin boundary. */
internal data class RemoteAdminMember(
    val id: Long,
    val username: String,
    val role: String,
    val createdAt: String,
    val calls: Long,
    val inputTokens: Long,
    val outputTokens: Long,
    val totalTokens: Long,
    val workEvents: Long,
    val logs: Long,
)

/** Remote DTO for `GET /api/v1/admin/users/<id>/records`. */
internal data class RemoteAdminMemberActivity(
    val user: RemoteAdminMemberIdentity,
    val records: List<UsageRecord>,
    val events: List<WorkEvent>,
    val logs: List<AppLog>,
)

/** Non-secret identity attached to an administrator-selected activity page. */
internal data class RemoteAdminMemberIdentity(
    val id: Long,
    val username: String,
    val role: String,
    val createdAt: String,
)

/** Stable error raised when the service returns a structured API error. */
class TrackerApiException(
    val statusCode: Int,
    val code: String,
    override val message: String,
    val requestId: String? = null,
) : java.io.IOException(message)
