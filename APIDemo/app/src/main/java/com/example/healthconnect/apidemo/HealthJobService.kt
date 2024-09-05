package com.example.healthconnect.apidemo

import android.app.job.JobParameters
import android.app.job.JobService
import android.content.Intent
import android.os.Build
import android.util.Log

class HealthJobService : JobService() {

    override fun onStartJob(params: JobParameters?): Boolean {
        Log.d("HealthJobService", "Job started, restarting HealthForegroundService if needed")

        // Znovu spusti foreground service
        val serviceIntent = Intent(this, HealthForegroundService::class.java)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(serviceIntent)
        } else {
            startService(serviceIntent)
        }

        // Job je hotový, nie je potrebné opakovanie
        return false
    }

    override fun onStopJob(params: JobParameters?): Boolean {
        return false
    }
}