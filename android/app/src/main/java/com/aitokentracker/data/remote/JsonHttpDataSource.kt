/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Implement the small, dependency-free HTTP adapter for `/api/v1`.
 * Module: Android data / remote source.
 *
 * The adapter deliberately uses platform URLConnection APIs for this first
 * slice. This keeps the project build-independent until the user approves a
 * networking dependency such as OkHttp or Ktor.
 */
package com.aitokentracker.data.remote

import com.aitokentracker.data.AppLog
import com.aitokentracker.data.config.normalizeApiBaseUrl
import com.aitokentracker.data.DailyUsage
import com.aitokentracker.data.LogDraft
import com.aitokentracker.data.ModelUsage
import com.aitokentracker.data.PageInfo
import com.aitokentracker.data.TokenPair
import com.aitokentracker.data.UsageRecord
import com.aitokentracker.data.UsageRecordDraft
import com.aitokentracker.data.UsageTotals
import com.aitokentracker.data.UserProfile
import com.aitokentracker.data.WorkEvent
import com.aitokentracker.data.WorkEventDraft
import java.io.IOException
import java.net.HttpURLConnection
import java.net.URL
import java.util.UUID
import org.json.JSONArray
import org.json.JSONObject

private const val CONNECT_TIMEOUT_MS = 10_000
private const val READ_TIMEOUT_MS = 15_000
private const val MAX_RESPONSE_CHARS = 1_048_576
private const val MAX_ASSISTANT_PREVIEW_CHARS = 6_000

/** Main-safe JSON data source; callers must invoke it off the main thread. */
internal class JsonHttpDataSource(
    baseUrl: String,
) : TrackerRemoteDataSource {
    @Volatile
    private var normalizedBaseUrl = normalizeApiBaseUrl(baseUrl)

    override fun configureBaseUrl(baseUrl: String) {
        normalizedBaseUrl = normalizeApiBaseUrl(baseUrl)
    }

    override fun login(username: String, password: String): TokenPair {
        val body = JSONObject()
            .put("username", username)
            .put("password", password)
        return parseTokenPair(request("/auth/login", "POST", body))
    }

    override fun refresh(refreshToken: String): TokenPair {
        val body = JSONObject().put("refresh_token", refreshToken)
        return parseTokenPair(request("/auth/refresh", "POST", body))
    }

    override fun logout(accessToken: String) {
        request("/auth/logout", "POST", bearer = accessToken)
    }

    override fun summary(accessToken: String, period: String): RemoteSummary {
        val body = request("/me/summary?period=${encodeQueryValue(period)}", "GET", bearer = accessToken)
        val totals = body.requiredObject("totals")
        return RemoteSummary(
            period = body.optionalString("period") ?: period,
            from = body.optionalString("from").orEmpty(),
            to = body.optionalString("to").orEmpty(),
            totals = parseTotals(totals),
            byModel = parseModels(body.requiredArray("by_model")),
            trend = parseTrend(body.requiredArray("trend")),
            records = parseRecords(body.requiredArray("records")),
        )
    }

    /** Discover provider models without retaining the submitted key. */
    override fun providerModels(
        accessToken: String,
        provider: String,
        baseUrl: String,
        apiKey: String,
    ): RemoteProviderModels {
        val body = JSONObject()
            .put("provider", provider)
            .put("base_url", baseUrl)
            .put("api_key", apiKey)
        val response = request("/provider/models", "POST", body, bearer = accessToken)
        return RemoteProviderModels(
            provider = response.optionalString("provider") ?: provider,
            models = parseModelIds(response.requiredArray("models")),
        )
    }

    /** Call one provider request and reduce the response to a UI-safe projection. */
    override fun proxyChat(
        accessToken: String,
        provider: String,
        baseUrl: String,
        apiKey: String,
        model: String,
        prompt: String,
        note: String,
        idempotencyKey: String,
    ): RemoteProviderCall {
        val messages = JSONArray().put(
            JSONObject()
                .put("role", "user")
                .put("content", prompt),
        )
        val body = JSONObject()
            .put("provider", provider)
            .put("base_url", baseUrl)
            .put("api_key", apiKey)
            .put("model", model)
            .put("messages", messages)
            .put("note", note)
        val response = request(
            path = "/proxy/chat/completions",
            method = "POST",
            body = body,
            bearer = accessToken,
            headers = mapOf("Idempotency-Key" to idempotencyKey),
        )
        val providerResponse = response.optJSONObject("response")
        val record = response.optJSONObject("record")?.let(::parseRecord)
        return RemoteProviderCall(
            provider = response.optionalString("provider") ?: provider,
            assistantText = extractAssistantText(providerResponse),
            usageSummary = formatUsageSummary(response.optJSONObject("usage")),
            record = record,
            recorded = response.optBoolean("recorded", false),
            replayed = response.optBoolean("replayed", false),
            warning = response.optionalString("warning"),
        )
    }

    override fun listUsageRecords(
        accessToken: String,
        period: String,
        limit: Int,
        offset: Int,
    ): RemoteUsageRecords {
        val safeLimit = limit.coerceIn(1, 200)
        val safeOffset = offset.coerceAtLeast(0)
        val body = request(
            "/records?period=${encodeQueryValue(period)}&limit=$safeLimit&offset=$safeOffset",
            "GET",
            bearer = accessToken,
        )
        return RemoteUsageRecords(
            period = body.optionalString("period") ?: period,
            from = body.optionalString("from").orEmpty(),
            to = body.optionalString("to").orEmpty(),
            records = parseRecords(body.requiredArray("records")),
            page = parsePage(body.requiredObject("pagination")),
        )
    }

    override fun createUsageRecord(
        accessToken: String,
        record: UsageRecordDraft,
        idempotencyKey: String,
    ): UsageRecord {
        val body = JSONObject()
            .put("model", record.model)
            .put("input_tokens", record.inputTokens)
            .put("output_tokens", record.outputTokens)
            .put("note", record.note)
        return parseRecord(
            request(
                path = "/records",
                method = "POST",
                body = body,
                bearer = accessToken,
                headers = mapOf("Idempotency-Key" to idempotencyKey),
            ).requiredObject("record"),
        )
    }

    override fun createWorkEvent(
        accessToken: String,
        event: WorkEventDraft,
        idempotencyKey: String,
    ): WorkEvent {
        val envelope = JSONObject()
            .put("protocol_version", 1)
            .put("command", "work_event.create")
            .put("request_id", UUID.randomUUID().toString())
            .put("idempotency_key", idempotencyKey)
            .put("payload", event.toJson())
        return parseWorkEvent(
            request(
                path = "/events/work",
                method = "POST",
                body = envelope,
                bearer = accessToken,
                headers = mapOf("Idempotency-Key" to idempotencyKey),
            ).requiredObject("event"),
        )
    }

    override fun listWorkEvents(accessToken: String, limit: Int, offset: Int): RemoteWorkEvents {
        val body = request(
            "/events/work?limit=${limit.coerceIn(1, 200)}&offset=${offset.coerceAtLeast(0)}",
            "GET",
            bearer = accessToken,
        )
        return RemoteWorkEvents(
            items = parseWorkEvents(body.requiredArray("events")),
            page = parsePage(body.requiredObject("pagination")),
        )
    }

    override fun createLog(accessToken: String, log: LogDraft): AppLog {
        val envelope = JSONObject()
            .put("protocol_version", 1)
            .put("command", "app_log.create")
            .put("request_id", UUID.randomUUID().toString())
            .put("payload", log.toJson())
        return parseLog(request("/logs", "POST", envelope, accessToken).requiredObject("log"))
    }

    override fun listLogs(accessToken: String, limit: Int, offset: Int): RemoteLogs {
        val body = request(
            "/logs?limit=${limit.coerceIn(1, 200)}&offset=${offset.coerceAtLeast(0)}",
            "GET",
            bearer = accessToken,
        )
        return RemoteLogs(
            items = parseLogs(body.requiredArray("logs")),
            page = parsePage(body.requiredObject("pagination")),
        )
    }

    override fun adminOverview(accessToken: String): RemoteAdminOverview {
        val body = request("/admin/overview", "GET", bearer = accessToken)
        val usage = body.requiredObject("usage")
        val workEvents = body.requiredObject("work_events")
        return RemoteAdminOverview(
            totalUsers = body.requiredLong("users"),
            usage = parseTotals(usage),
            workEvents = RemoteAdminWorkEventTotals(
                total = workEvents.requiredLong("total"),
                success = workEvents.requiredLong("success"),
                failure = workEvents.requiredLong("failure"),
            ),
            logCount = body.requiredLong("logs"),
            byModel = parseModels(body.requiredArray("by_model")),
            trend = parseTrend(body.requiredArray("trend")),
        )
    }

    override fun adminMembers(accessToken: String, limit: Int, offset: Int): RemoteAdminMembers {
        val body = request(
            "/admin/users?limit=${limit.coerceIn(1, 200)}&offset=${offset.coerceAtLeast(0)}",
            "GET",
            bearer = accessToken,
        )
        return RemoteAdminMembers(
            items = body.requiredArray("items").objects().map(::parseAdminMember),
            page = parsePage(body),
        )
    }

    override fun adminMemberActivity(
        accessToken: String,
        userId: Long,
        limit: Int,
    ): RemoteAdminMemberActivity {
        val safeUserId = userId.coerceAtLeast(1)
        val safeLimit = limit.coerceIn(1, 200)
        val body = request(
            "/admin/users/$safeUserId/records?limit=$safeLimit",
            "GET",
            bearer = accessToken,
        )
        val user = body.requiredObject("user")
        return RemoteAdminMemberActivity(
            user = RemoteAdminMemberIdentity(
                id = user.requiredLong("id"),
                username = user.requiredString("username"),
                role = user.optionalString("role") ?: "user",
                createdAt = user.requiredString("created_at"),
            ),
            records = parseRecords(body.requiredArray("records")),
            events = parseWorkEvents(body.requiredArray("events")),
            logs = parseLogs(body.requiredArray("logs")),
        )
    }

    private fun request(
        path: String,
        method: String,
        body: JSONObject? = null,
        bearer: String? = null,
        headers: Map<String, String> = emptyMap(),
    ): JSONObject {
        val connection = (URL("$normalizedBaseUrl$path").openConnection() as HttpURLConnection).apply {
            requestMethod = method
            connectTimeout = CONNECT_TIMEOUT_MS
            readTimeout = READ_TIMEOUT_MS
            doInput = true
            instanceFollowRedirects = false
            setRequestProperty("Accept", "application/json")
            bearer?.let { setRequestProperty("Authorization", "Bearer $it") }
            headers.forEach { (key, value) -> setRequestProperty(key, value) }
        }
        return try {
            if (body != null) {
                connection.doOutput = true
                connection.setRequestProperty("Content-Type", "application/json; charset=utf-8")
                connection.outputStream.use { output ->
                    output.write(body.toString().toByteArray(Charsets.UTF_8))
                }
            }
            val statusCode = connection.responseCode
            val input = if (statusCode in 200..299) connection.inputStream else connection.errorStream
            val text = input?.use(::readBounded).orEmpty()
            if (statusCode !in 200..299) throw parseApiError(statusCode, text)
            if (text.isBlank()) JSONObject() else parseJson(statusCode, text)
        } catch (error: TrackerApiException) {
            throw error
        } catch (error: IOException) {
            throw error
        } finally {
            connection.disconnect()
        }
    }

    private fun readBounded(input: java.io.InputStream): String {
        val reader = input.bufferedReader(Charsets.UTF_8)
        val result = StringBuilder()
        val buffer = CharArray(8_192)
        while (true) {
            val count = reader.read(buffer)
            if (count < 0) break
            if (result.length + count > MAX_RESPONSE_CHARS) {
                throw TrackerApiException(502, "RESPONSE_TOO_LARGE", "服务响应超过安全上限")
            }
            result.append(buffer, 0, count)
        }
        return result.toString()
    }

    private fun parseJson(statusCode: Int, text: String): JSONObject = try {
        JSONObject(text)
    } catch (error: Exception) {
        throw TrackerApiException(statusCode, "INVALID_RESPONSE", "服务返回了无法解析的数据")
    }

    private fun parseApiError(statusCode: Int, text: String): TrackerApiException {
        return try {
            val root = JSONObject(text)
            val error = root.optJSONObject("error")
            TrackerApiException(
                statusCode = statusCode,
                code = error?.optString("code").orEmpty().ifBlank { "HTTP_$statusCode" },
                message = error?.optString("message").orEmpty().ifBlank { "请求失败，请稍后重试" },
                requestId = root.optionalString("request_id"),
            )
        } catch (_: Exception) {
            TrackerApiException(statusCode, "HTTP_$statusCode", "请求失败，请稍后重试")
        }
    }

    private fun parseTokenPair(root: JSONObject): TokenPair {
        val user = root.requiredObject("user")
        return TokenPair(
            accessToken = root.requiredString("access_token"),
            refreshToken = root.requiredString("refresh_token"),
            expiresInSeconds = root.requiredLong("expires_in"),
            user = UserProfile(
                id = user.requiredLong("id"),
                username = user.requiredString("username"),
                role = user.optionalString("role") ?: "user",
            ),
        )
    }

    private fun parseTotals(value: JSONObject): UsageTotals = UsageTotals(
        calls = value.requiredLong("calls"),
        inputTokens = value.requiredLong("input_tokens"),
        outputTokens = value.requiredLong("output_tokens"),
        totalTokens = value.requiredLong("total_tokens"),
    )

    private fun parseModels(values: JSONArray): List<ModelUsage> = values.objects().map { value ->
        ModelUsage(
            model = value.requiredString("model"),
            calls = value.requiredLong("calls"),
            inputTokens = value.requiredLong("input_tokens"),
            outputTokens = value.requiredLong("output_tokens"),
            totalTokens = value.requiredLong("total_tokens"),
        )
    }

    private fun parseTrend(values: JSONArray): List<DailyUsage> = values.objects().map { value ->
        DailyUsage(
            day = value.requiredString("day"),
            inputTokens = value.requiredLong("input_tokens"),
            outputTokens = value.requiredLong("output_tokens"),
            totalTokens = value.requiredLong("total_tokens"),
        )
    }

    private fun parseModelIds(values: JSONArray): List<String> = buildList {
        for (index in 0 until values.length()) {
            val model = values.optString(index).trim()
            if (model.isNotEmpty()) add(model)
        }
    }

    /** Extract only the first assistant text; raw upstream JSON never reaches UI state. */
    private fun extractAssistantText(response: JSONObject?): String {
        val choices = response?.optJSONArray("choices") ?: return ""
        val first = choices.optJSONObject(0) ?: return ""
        val message = first.optJSONObject("message")
        val content = message?.opt("content") ?: first.opt("text")
        val text = when (content) {
            is String -> content
            is JSONArray -> extractContentParts(content)
            else -> ""
        }
        return text.trim().take(MAX_ASSISTANT_PREVIEW_CHARS)
    }

    private fun extractContentParts(values: JSONArray): String = buildString {
        for (index in 0 until values.length()) {
            val item = values.optJSONObject(index) ?: continue
            val text = item.opt("text") as? String ?: continue
            if (isNotEmpty()) append('\n')
            append(text)
            if (length >= MAX_ASSISTANT_PREVIEW_CHARS) break
        }
    }

    /** Render numeric usage aliases without exposing the provider response map. */
    private fun formatUsageSummary(usage: JSONObject?): String {
        if (usage == null) return ""
        val input = usage.optionalLongAlias("prompt_tokens", "input_tokens", "promptTokens")
        val output = usage.optionalLongAlias("completion_tokens", "output_tokens", "completionTokens")
        val total = usage.optionalLongAlias("total_tokens", "totalTokens") ?: listOfNotNull(input, output).takeIf { it.size == 2 }?.sum()
        return buildList {
            input?.let { add("输入 ${formatTokenCount(it)}") }
            output?.let { add("输出 ${formatTokenCount(it)}") }
            total?.let { add("合计 ${formatTokenCount(it)}") }
        }.joinToString(" · ")
    }

    private fun parseRecords(values: JSONArray): List<UsageRecord> = values.objects().map(::parseRecord)

    private fun parseRecord(value: JSONObject): UsageRecord = UsageRecord(
        id = value.requiredLong("id"),
        model = value.requiredString("model"),
        inputTokens = value.requiredLong("input_tokens"),
        outputTokens = value.requiredLong("output_tokens"),
        totalTokens = value.requiredLong("total_tokens"),
        timestamp = value.requiredString("timestamp"),
        note = value.optionalString("note").orEmpty(),
        source = value.optionalString("source").orEmpty(),
    )

    private fun parseWorkEvents(values: JSONArray): List<WorkEvent> = values.objects().map(::parseWorkEvent)

    private fun parseWorkEvent(value: JSONObject): WorkEvent = WorkEvent(
        id = value.requiredLong("id"),
        direction = value.requiredString("direction"),
        outcome = value.requiredString("outcome"),
        durationMs = value.optionalLong("duration_ms"),
        efficiencyScore = value.optionalLong("efficiency_score")?.toInt(),
        resultCode = value.optionalString("result_code"),
        errorCode = value.optionalString("error_code"),
        project = value.optionalString("project").orEmpty(),
        taskType = value.optionalString("task_type").orEmpty(),
        note = value.optionalString("note").orEmpty(),
        createdAt = value.requiredString("created_at"),
    )

    private fun parseLogs(values: JSONArray): List<AppLog> = values.objects().map(::parseLog)

    private fun parseLog(value: JSONObject): AppLog = AppLog(
        id = value.requiredLong("id"),
        requestId = value.requiredString("request_id"),
        level = value.requiredString("level"),
        eventType = value.requiredString("event_type"),
        message = value.requiredString("message"),
        errorCode = value.optionalString("error_code"),
        createdAt = value.requiredString("created_at"),
    )

    private fun parsePage(value: JSONObject): PageInfo = PageInfo(
        limit = value.requiredLong("limit").toInt(),
        offset = value.requiredLong("offset").toInt(),
        total = value.requiredLong("total").toInt(),
    )

    private fun parseAdminMember(value: JSONObject): RemoteAdminMember = RemoteAdminMember(
        id = value.requiredLong("id"),
        username = value.requiredString("username"),
        role = value.optionalString("role") ?: "user",
        createdAt = value.requiredString("created_at"),
        calls = value.requiredLong("calls"),
        inputTokens = value.requiredLong("input_tokens"),
        outputTokens = value.requiredLong("output_tokens"),
        totalTokens = value.requiredLong("total_tokens"),
        workEvents = value.requiredLong("work_events"),
        logs = value.requiredLong("logs"),
    )
}

private fun WorkEventDraft.toJson(): JSONObject = JSONObject().apply {
    put("direction", direction)
    put("outcome", outcome)
    durationMs?.let { put("duration_ms", it) }
    efficiencyScore?.let { put("efficiency_score", it) }
    resultCode?.let { put("result_code", it) }
    errorCode?.let { put("error_code", it) }
    put("project", project)
    put("task_type", taskType)
    put("note", note)
}

private fun LogDraft.toJson(): JSONObject = JSONObject().apply {
    put("level", level)
    put("event_type", eventType)
    put("message", message)
    errorCode?.let { put("error_code", it) }
    put("metadata", JSONObject().apply { metadata.forEach { (key, value) -> put(key, value) } })
}

private fun JSONObject.requiredObject(key: String): JSONObject = optJSONObject(key)
    ?: throw TrackerApiException(502, "INVALID_RESPONSE", "服务响应缺少 $key")

private fun JSONObject.requiredArray(key: String): JSONArray = optJSONArray(key)
    ?: throw TrackerApiException(502, "INVALID_RESPONSE", "服务响应缺少 $key")

private fun JSONObject.requiredString(key: String): String {
    val value = opt(key)
    if (value !is String || value.isBlank()) throw TrackerApiException(502, "INVALID_RESPONSE", "服务响应缺少 $key")
    return value
}

private fun JSONObject.optionalString(key: String): String? = opt(key).let { value ->
    if (value == null || value == JSONObject.NULL) null else value as? String
}

private fun JSONObject.requiredLong(key: String): Long = optionalLong(key)
    ?: throw TrackerApiException(502, "INVALID_RESPONSE", "服务响应缺少 $key")

private fun JSONObject.optionalLong(key: String): Long? = when (val value = opt(key)) {
    is Number -> value.toLong()
    else -> null
}

private fun JSONObject.optionalLongAlias(vararg keys: String): Long? = keys.firstNotNullOfOrNull { key ->
    when (val value = opt(key)) {
        is Number -> value.toLong()
        is String -> value.toLongOrNull()
        else -> null
    }
}

private fun JSONArray.objects(): List<JSONObject> = buildList {
    for (index in 0 until length()) {
        add(optJSONObject(index) ?: throw TrackerApiException(502, "INVALID_RESPONSE", "服务列表包含无效项目"))
    }
}

private fun formatTokenCount(value: Long): String = when {
    value >= 1_000_000 -> "%.1fM".format(java.util.Locale.US, value / 1_000_000.0)
    value >= 1_000 -> "%.1fK".format(java.util.Locale.US, value / 1_000.0)
    else -> value.toString()
}

private fun encodeQueryValue(value: String): String = java.net.URLEncoder.encode(value, Charsets.UTF_8.name())
