/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Hold authentication and dashboard state outside Compose screens.
 * Module: Android feature / auth and session state.
 */
package com.aitokentracker.feature.auth

import android.os.Handler
import android.os.Looper
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.aitokentracker.data.AdminMember
import com.aitokentracker.data.AdminMemberActivity
import com.aitokentracker.data.AdminOverview
import com.aitokentracker.data.LogDraft
import com.aitokentracker.data.PageInfo
import com.aitokentracker.data.ProviderCallResult
import com.aitokentracker.data.ProviderModels
import com.aitokentracker.data.TrackerRepository
import com.aitokentracker.data.UsageRecord
import com.aitokentracker.data.UsageRecordDraft
import com.aitokentracker.data.UsageSummary
import com.aitokentracker.data.UserProfile
import com.aitokentracker.data.WorkEvent
import com.aitokentracker.data.WorkEventDraft
import com.aitokentracker.data.AppLog
import com.aitokentracker.data.remote.TrackerApiException
import java.net.ConnectException
import java.net.SocketTimeoutException
import java.util.UUID
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

/** Single source of truth for the first Android login → summary path. */
internal class TrackerViewModel(
    private val repository: TrackerRepository,
) : ViewModel() {
    val endpoint: String
        get() = repository.endpoint

    var state by mutableStateOf<TrackerUiState>(TrackerUiState.Booting)
        private set

    /** One-shot status for structured event/log actions rendered by the UI. */
    var actionMessage by mutableStateOf<String?>(null)
        private set

    var workEvents by mutableStateOf<List<WorkEvent>>(emptyList())
        private set

    var appLogs by mutableStateOf<List<AppLog>>(emptyList())
        private set

    /** Bounded personal token history loaded from the versioned records API. */
    var usageRecords by mutableStateOf<List<UsageRecord>>(emptyList())
        private set

    var usageRecordsPage by mutableStateOf<PageInfo?>(null)
        private set

    /** Ephemeral provider discovery state; no key is held in ViewModel state. */
    var providerModels by mutableStateOf<ProviderModels?>(null)
        private set

    var providerBusy by mutableStateOf(false)
        private set

    var providerMessage by mutableStateOf<String?>(null)
        private set

    var providerResult by mutableStateOf<ProviderCallResult?>(null)
        private set

    var adminOverview by mutableStateOf<AdminOverview?>(null)
        private set

    var adminMembers by mutableStateOf<List<AdminMember>>(emptyList())
        private set

    var selectedAdminMemberId by mutableStateOf<Long?>(null)
        private set

    var adminMemberActivity by mutableStateOf<AdminMemberActivity?>(null)
        private set

    var adminLoading by mutableStateOf(false)
        private set

    var adminError by mutableStateOf<String?>(null)
        private set

    private val worker: ExecutorService = Executors.newSingleThreadExecutor()
    private val mainHandler = Handler(Looper.getMainLooper())

    init {
        worker.execute {
            val user = repository.restoredUser()
            if (user == null) {
                publish { TrackerUiState.SignedOut }
            } else {
                try {
                    val summary = repository.summary("day")
                    if (isAdmin(user)) syncAdmin() else syncActivity()
                    publish { TrackerUiState.Ready(user, summary) }
                } catch (error: Exception) {
                    publish { TrackerUiState.Failure(userFriendlyMessage(error), user) }
                }
            }
        }
    }

    /** Begin a login request; password remains in the call stack only. */
    fun login(baseUrl: String, username: String, password: String) {
        if (baseUrl.trim().isEmpty() || username.trim().isEmpty() || password.isEmpty()) {
            state = TrackerUiState.Failure("请输入服务地址、用户名和密码", null)
            return
        }
        actionMessage = null
        state = TrackerUiState.SigningIn
        worker.execute {
            try {
                val user = repository.login(baseUrl, username, password)
                val summary = repository.summary("day")
                if (isAdmin(user)) syncAdmin() else syncActivity()
                publish { TrackerUiState.Ready(user, summary) }
            } catch (error: Exception) {
                publish { TrackerUiState.Failure(userFriendlyMessage(error), null) }
            }
        }
    }

    /** Refresh the personal summary without exposing bearer-token mechanics. */
    fun refresh() {
        val current = state
        val user = when (current) {
            is TrackerUiState.Ready -> current.user
            is TrackerUiState.Failure -> current.user
            else -> return
        }
        val previousSummary = when (current) {
            is TrackerUiState.Ready -> current.summary
            is TrackerUiState.Failure -> current.summary
            else -> null
        }
        state = TrackerUiState.Refreshing(user, previousSummary)
        worker.execute {
            try {
                val summary = repository.summary("day")
                if (isAdmin(user)) syncAdmin() else syncActivity()
                publish { TrackerUiState.Ready(user, summary) }
            } catch (error: Exception) {
                publish { TrackerUiState.Failure(userFriendlyMessage(error), user, previousSummary) }
            }
        }
    }

    /** Persist one token record, then refresh the server-owned summary projection. */
    fun recordUsage(record: UsageRecordDraft) {
        if (currentUser() == null) return
        actionMessage = "正在写入 token 记录…"
        val idempotencyKey = UUID.randomUUID().toString()
        worker.execute {
            try {
                repository.createUsageRecord(record, idempotencyKey)
                publishAction("token 记录已写入")
                mainHandler.post { refresh() }
            } catch (error: Exception) {
                publishAction(userFriendlyMessage(error))
            }
        }
    }

    /** Detect models using a short-lived API key supplied by the current form. */
    fun detectProviderModels(provider: String, baseUrl: String, apiKey: String) {
        if (currentUser() == null) return
        val safeBaseUrl = baseUrl.trim()
        val safeApiKey = apiKey.trim()
        if (safeBaseUrl.isBlank() || safeApiKey.isBlank()) {
            providerMessage = "请先填写上游 Base URL 和 API Key"
            return
        }
        providerBusy = true
        providerMessage = "正在检测模型…"
        providerModels = null
        providerResult = null
        worker.execute {
            try {
                val models = repository.providerModels(provider, safeBaseUrl, safeApiKey)
                mainHandler.post {
                    providerModels = models
                    providerBusy = false
                    providerMessage = "已检测到 ${models.models.size} 个模型"
                }
            } catch (error: Exception) {
                publishProviderError(userFriendlyMessage(error))
            }
        }
    }

    /** Send one non-streaming provider call and let the server record usage. */
    fun proxyProviderChat(
        provider: String,
        baseUrl: String,
        apiKey: String,
        model: String,
        prompt: String,
        note: String,
    ) {
        if (currentUser() == null) return
        val safeBaseUrl = baseUrl.trim()
        val safeApiKey = apiKey.trim()
        val safeModel = model.trim()
        val safePrompt = prompt.trim()
        if (safeBaseUrl.isBlank() || safeApiKey.isBlank() || safeModel.isBlank() || safePrompt.isBlank()) {
            providerMessage = "请填写 Base URL、API Key、模型和提示词"
            return
        }
        providerBusy = true
        providerMessage = "正在调用上游并自动记账…"
        providerResult = null
        val idempotencyKey = UUID.randomUUID().toString()
        worker.execute {
            try {
                val result = repository.proxyChat(
                    provider = provider,
                    baseUrl = safeBaseUrl,
                    apiKey = safeApiKey,
                    model = safeModel,
                    prompt = safePrompt,
                    note = note.trim(),
                    idempotencyKey = idempotencyKey,
                )
                mainHandler.post {
                    providerResult = result
                    providerBusy = false
                    providerMessage = when {
                        result.recorded && result.replayed -> "请求重试已复用原 token 记录"
                        result.recorded -> "调用完成，token 已自动写入"
                        else -> result.warning ?: "调用完成，但没有可记账的 usage"
                    }
                }
                if (result.recorded) {
                    syncActivity()
                    mainHandler.post { refresh() }
                }
            } catch (error: Exception) {
                publishProviderError(userFriendlyMessage(error))
            }
        }
    }

    /** Revoke the current server token when possible and always clear local state. */
    fun logout() {
        actionMessage = null
        resetProviderState()
        state = TrackerUiState.SigningOut
        worker.execute {
            repository.logout()
            publishActivity(emptyList(), emptyList(), emptyList(), null)
            publishAdminReset()
            publish { TrackerUiState.SignedOut }
        }
    }

    /** Load one member's bounded activity after an administrator selects it. */
    fun selectAdminMember(memberId: Long) {
        val user = currentUser() ?: return
        if (!isAdmin(user) || memberId < 1) return
        selectedAdminMemberId = memberId
        adminMemberActivity = null
        adminLoading = true
        adminError = null
        worker.execute {
            try {
                val activity = repository.adminMemberActivity(memberId, limit = 100)
                publishAdminActivity(activity)
            } catch (error: Exception) {
                publishAdminError(userFriendlyMessage(error))
            }
        }
    }

    /** Persist a privacy-safe work event and refresh the personal summary. */
    fun recordWorkEvent(event: WorkEventDraft) {
        if (currentUser() == null) return
        actionMessage = "正在写入工作信号…"
        worker.execute {
            try {
                repository.createWorkEvent(event, UUID.randomUUID().toString())
                syncActivity()
                publishAction("工作信号已写入")
                mainHandler.post { refresh() }
            } catch (error: Exception) {
                publishAction(userFriendlyMessage(error))
            }
        }
    }

    /** Persist a bounded diagnostic log without exposing the HTTP layer to UI. */
    fun recordLog(log: LogDraft) {
        if (currentUser() == null) return
        actionMessage = "正在写入诊断日志…"
        worker.execute {
            try {
                repository.createLog(log)
                syncActivity()
                publishAction("诊断日志已写入")
            } catch (error: Exception) {
                publishAction(userFriendlyMessage(error))
            }
        }
    }

    private fun currentUser(): UserProfile? = when (val current = state) {
        is TrackerUiState.Ready -> current.user
        is TrackerUiState.Refreshing -> current.user
        is TrackerUiState.Failure -> current.user
        else -> null
    }

    private fun publishAction(message: String) {
        mainHandler.post { actionMessage = message }
    }

    private fun publishProviderError(message: String) {
        mainHandler.post {
            providerBusy = false
            providerMessage = message
        }
    }

    private fun resetProviderState() {
        mainHandler.post {
            providerModels = null
            providerBusy = false
            providerMessage = null
            providerResult = null
        }
    }

    private fun syncActivity() {
        val events = try {
            repository.listWorkEvents(limit = 20).items
        } catch (_: Exception) {
            emptyList()
        }
        val logs = try {
            repository.listLogs(limit = 20).items
        } catch (_: Exception) {
            emptyList()
        }
        val usagePage = try {
            repository.listUsageRecords(period = "all", limit = 50, offset = 0)
        } catch (_: Exception) {
            null
        }
        publishActivity(events, logs, usagePage?.records.orEmpty(), usagePage?.page)
    }

    /** Load only aggregate admin data; member activity remains click-to-load. */
    private fun syncAdmin() {
        val user = currentUser()
        if (user != null && !isAdmin(user)) return
        publishAdminLoading(true)
        try {
            val overview = repository.adminOverview()
            val members = repository.adminMembers(limit = 50).items
            publishAdminData(overview, members)
        } catch (error: Exception) {
            publishAdminError(userFriendlyMessage(error))
        }
    }

    private fun publishActivity(
        events: List<WorkEvent>,
        logs: List<AppLog>,
        records: List<UsageRecord>,
        page: PageInfo?,
    ) {
        mainHandler.post {
            workEvents = events
            appLogs = logs
            usageRecords = records
            usageRecordsPage = page
        }
    }

    private fun publishAdminLoading(loading: Boolean) {
        mainHandler.post { adminLoading = loading }
    }

    private fun publishAdminData(overview: AdminOverview, members: List<AdminMember>) {
        mainHandler.post {
            adminOverview = overview
            adminMembers = members
            adminLoading = false
            adminError = null
        }
    }

    private fun publishAdminActivity(activity: AdminMemberActivity) {
        mainHandler.post {
            adminMemberActivity = activity
            adminLoading = false
            adminError = null
        }
    }

    private fun publishAdminError(message: String) {
        mainHandler.post {
            adminLoading = false
            adminError = message
        }
    }

    private fun publishAdminReset() {
        mainHandler.post {
            adminOverview = null
            adminMembers = emptyList()
            selectedAdminMemberId = null
            adminMemberActivity = null
            adminLoading = false
            adminError = null
        }
    }

    private fun publish(factory: () -> TrackerUiState) {
        mainHandler.post { state = factory() }
    }

    override fun onCleared() {
        worker.shutdownNow()
        super.onCleared()
    }

    private fun isAdmin(user: UserProfile): Boolean = user.role.equals("admin", ignoreCase = true)
}

/** UI states are immutable and renderable without reading a data source. */
internal sealed interface TrackerUiState {
    /** The repository is restoring or checking the local session. */
    data object Booting : TrackerUiState
    /** No authenticated user is available for the application surface. */
    data object SignedOut : TrackerUiState
    /** Credentials are being submitted and no duplicate login is accepted. */
    data object SigningIn : TrackerUiState
    /** Logout is clearing remote and encrypted local session state. */
    data object SigningOut : TrackerUiState
    /** A refresh is in flight while the last safe projection remains visible. */
    data class Refreshing(val user: UserProfile, val summary: UsageSummary?) : TrackerUiState
    /** The authenticated dashboard has a server-owned summary projection. */
    data class Ready(val user: UserProfile, val summary: UsageSummary) : TrackerUiState
    /** A user-safe error with optional last-known projection for recovery UI. */
    data class Failure(
        val message: String,
        val user: UserProfile?,
        val summary: UsageSummary? = null,
    ) : TrackerUiState
}

private fun userFriendlyMessage(error: Exception): String = when (error) {
    is TrackerApiException -> when (error.code) {
        "INVALID_CREDENTIALS" -> "用户名或密码不正确"
        "AUTH_REQUIRED", "INVALID_REFRESH_TOKEN" -> "登录状态已失效，请重新登录"
        "ADMIN_REQUIRED" -> "当前账号没有管理员权限"
        "USER_NOT_FOUND" -> "成员不存在或已删除"
        "RATE_LIMITED" -> "请求过于频繁，请稍后再试"
        "PROVIDER_INPUT_INVALID" -> "上游连接参数无效，请检查 URL、Key 或模型"
        "PROVIDER_UNAVAILABLE" -> "无法连接上游服务，请检查白名单、网络和 Key"
        "PROVIDER_RESPONSE_TOO_LARGE" -> "上游响应超过安全上限"
        "PROVIDER_INVALID_RESPONSE" -> "上游返回了无法解析的数据"
        "PROVIDER_ERROR" -> "上游服务拒绝了本次调用"
        else -> error.message.ifBlank { "服务请求失败" }
    }
    is SocketTimeoutException, is ConnectException -> "无法连接 Windows 中心服务"
    else -> "连接失败，请检查服务地址和网络"
}
