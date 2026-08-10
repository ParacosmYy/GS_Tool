/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Compose-compatible dependency boundary for the first Android slice.
 * Module: Android feature / dependency construction.
 */
package com.aitokentracker.feature.auth

import android.content.Context
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import com.aitokentracker.BuildConfig
import com.aitokentracker.data.TrackerRepository
import com.aitokentracker.data.config.ApiEndpointStore
import com.aitokentracker.data.remote.JsonHttpDataSource
import com.aitokentracker.data.secure.EncryptedSessionStore

/** Creates one repository graph per Activity-owned ViewModel lifecycle. */
internal class TrackerViewModelFactory(context: Context) : ViewModelProvider.Factory {
    private val applicationContext = context.applicationContext

    @Suppress("UNCHECKED_CAST")
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        require(modelClass.isAssignableFrom(TrackerViewModel::class.java)) {
            "Unsupported ViewModel: ${modelClass.name}"
        }
        val store = EncryptedSessionStore(applicationContext)
        val endpointStore = ApiEndpointStore(applicationContext, BuildConfig.API_BASE_URL)
        val remote = JsonHttpDataSource(endpointStore.load())
        return TrackerViewModel(TrackerRepository(remote, store, endpointStore)) as T
    }
}
