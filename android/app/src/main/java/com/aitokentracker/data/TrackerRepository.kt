/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Own the Android session and expose one source of truth to ViewModels.
 * Module: Android data / repository boundary.
 *
 * The repository is the only layer allowed to coordinate remote calls and the
 * encrypted session store. UI code never sees refresh-token rotation details.
 */
package com.aitokentracker.data

import com.aitokentracker.data.config.ApiEndpointStore
import com.aitokentracker.data.config.normalizeApiBaseUrl
import com.aitokentracker.data.remote.TrackerApiException
import com.aitokentracker.data.remote.TrackerRemoteDataSource
import com.aitokentracker.data.secure.EncryptedSessionStore

/** Cross-client repository for login, summary, events, and diagnostics. */
internal class TrackerRepository(
    private val remote: TrackerRemoteDataSource,
    private val sessionStore: EncryptedSessionStore,
    private val endpointStore: ApiEndpointStore,
    private val allowInsecureHttp: Boolean,
) {
    val endpoint: String
        get() = endpointStore.load()

    /** Authenticate, rotate session storage, and return the server user projection. */
    fun login(baseUrl: String, username: String, password: String): UserProfile {
        configureEndpoint(baseUrl)
        val pair = remote.login(username.trim(), password)
        sessionStore.save(pair)
        return pair.user
    }

    /** Restore only the non-secret user projection from the encrypted session. */
    fun restoredUser(): UserProfile? = sessionStore.load()?.user

    /** Read a server-owned summary; a 401 triggers one refresh-and-retry cycle. */
    fun summary(period: String): UsageSummary = withAccess { token ->
        remote.summary(token, period).toDomain()
    }

    /** Discover provider models through the authenticated Windows boundary. */
    fun providerModels(provider: String, baseUrl: String, apiKey: String): ProviderModels = withAccess { token ->
        remote.providerModels(token, provider, baseUrl, apiKey).toDomain()
    }

    /** Proxy one provider call and receive only the server-approved projection. */
    fun proxyChat(
        provider: String,
        baseUrl: String,
        apiKey: String,
        model: String,
        prompt: String,
        note: String,
        idempotencyKey: String,
    ): ProviderCallResult = withAccess { token ->
        remote.proxyChat(token, provider, baseUrl, apiKey, model, prompt, note, idempotencyKey).toDomain()
    }

    /** Read bounded personal history through the same account-owned API. */
    fun listUsageRecords(
        period: String = "all",
        limit: Int = 50,
        offset: Int = 0,
    ): UsageRecordsPage = withAccess { token ->
        remote.listUsageRecords(token, period, limit, offset).toDomain()
    }

    /** Persist one user-entered usage record through the versioned API. */
    fun createUsageRecord(record: UsageRecordDraft, idempotencyKey: String): UsageRecord = withAccess { token ->
        remote.createUsageRecord(token, record, idempotencyKey)
    }

    /** Persist a structured work event with the caller-provided idempotency key. */
    fun createWorkEvent(event: WorkEventDraft, idempotencyKey: String): WorkEvent = withAccess { token ->
        remote.createWorkEvent(token, event, idempotencyKey)
    }

    /** Read bounded personal work-event history. */
    fun listWorkEvents(limit: Int = 50, offset: Int = 0): RemoteWorkEvents = withAccess { token ->
        remote.listWorkEvents(token, limit, offset)
    }

    /** Persist a privacy-filtered diagnostic log. */
    fun createLog(log: LogDraft): AppLog = withAccess { token ->
        remote.createLog(token, log)
    }

    /** Read bounded personal diagnostic-log history. */
    fun listLogs(limit: Int = 50, offset: Int = 0): RemoteLogs = withAccess { token ->
        remote.listLogs(token, limit, offset)
    }

    /** Read administrator data only after the Windows service authorizes it. */
    fun adminOverview(): AdminOverview = withAccess { token ->
        remote.adminOverview(token).toDomain()
    }

    /** Read the server-authorized member aggregate page. */
    fun adminMembers(limit: Int = 50, offset: Int = 0): AdminMembersPage = withAccess { token ->
        remote.adminMembers(token, limit, offset).toDomain()
    }

    /** Read one server-authorized member activity projection. */
    fun adminMemberActivity(userId: Long, limit: Int = 100): AdminMemberActivity = withAccess { token ->
        remote.adminMemberActivity(token, userId, limit).toDomain()
    }

    /** Normalize and persist a new endpoint, clearing tokens scoped to the old host. */
    private fun configureEndpoint(baseUrl: String) {
        val normalized = normalizeApiBaseUrl(baseUrl, allowInsecureHttp)
        if (normalized != endpointStore.load()) {
            // Tokens are scoped to the previous Windows service; never replay
            // them against a newly entered host or classroom deployment.
            sessionStore.clear()
        }
        remote.configureBaseUrl(normalized)
        endpointStore.save(normalized)
    }

    /** Attempt remote revocation, then always clear the local encrypted session. */
    fun logout() {
        val current = sessionStore.load()
        if (current != null) {
            try {
                remote.logout(current.accessToken)
            } catch (_: Exception) {
                // Local sign-out must still complete when the service is offline.
            }
        }
        sessionStore.clear()
    }

    /** Execute one bearer operation and perform at most one refresh retry on 401. */
    private fun <T> withAccess(operation: (String) -> T): T {
        val current = sessionStore.load()
            ?: throw TrackerApiException(401, "AUTH_REQUIRED", "请先登录")
        return try {
            operation(current.accessToken)
        } catch (error: TrackerApiException) {
            if (error.statusCode != 401) throw error
            val refreshed = sessionStore.load()?.let { snapshot ->
                try {
                    remote.refresh(snapshot.refreshToken)
                } catch (refreshError: TrackerApiException) {
                    // A definitive auth rejection must not leave a poisoned
                    // encrypted session that will fail on every app launch.
                    if (refreshError.statusCode == 401 || refreshError.code == "INVALID_REFRESH_TOKEN") {
                        sessionStore.clear()
                    }
                    throw refreshError
                }
            } ?: throw TrackerApiException(401, "INVALID_REFRESH_TOKEN", "登录状态已失效，请重新登录")
            sessionStore.save(refreshed)
            operation(refreshed.accessToken)
        }
    }
}

private fun com.aitokentracker.data.remote.RemoteSummary.toDomain(): UsageSummary = UsageSummary(
    period = period,
    from = from,
    to = to,
    totals = totals,
    byModel = byModel,
    trend = trend,
    records = records,
)

private fun com.aitokentracker.data.remote.RemoteProviderModels.toDomain(): ProviderModels = ProviderModels(
    provider = provider,
    models = models,
)

private fun com.aitokentracker.data.remote.RemoteProviderCall.toDomain(): ProviderCallResult = ProviderCallResult(
    provider = provider,
    assistantText = assistantText,
    usageSummary = usageSummary,
    record = record,
    recorded = recorded,
    replayed = replayed,
    warning = warning,
)

private fun com.aitokentracker.data.remote.RemoteUsageRecords.toDomain(): UsageRecordsPage = UsageRecordsPage(
    period = period,
    from = from,
    to = to,
    records = records,
    page = page,
)
