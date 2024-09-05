package com.example.healthconnect.apidemo

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.app.job.JobInfo
import android.app.job.JobScheduler
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import fi.iki.elonen.NanoHTTPD
import fi.iki.elonen.NanoHTTPD.newFixedLengthResponse
import kotlinx.coroutines.runBlocking
import java.time.Instant
import java.time.LocalDate
import java.time.ZoneId

class HealthForegroundService : Service() {

    companion object {
        const val CHANNEL_ID = "HealthConnectServiceChannel"
        const val NOTIFICATION_ID = 1
        const val TAG = "HealthConnectService"
    }

    private lateinit var healthConnectManager: HealthConnectManager

    override fun onCreate() {
        super.onCreate()
        healthConnectManager = HealthConnectManager(this)

        Log.d(TAG, "Service created")

        // Vytvorenie notifikačného kanála (pre Android 8.0 a vyššie)
        createNotificationChannel()

        val notification = createNotification()
        startForeground(NOTIFICATION_ID, notification)

        // Spusti NanoHTTPD server
        startNanoHttpServer()

        // Naplánovanie JobScheduleru
        scheduleJob()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.d(TAG, "Service started")
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    private fun startNanoHttpServer() {
        val server = HealthConnectServer(healthConnectManager, 8082)
        server.start()
        Log.d(TAG, "NanoHTTPD server started on port 8082")
    }

    private fun createNotification(): Notification {
        val notificationBuilder = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("HealthConnect Service")
            .setContentText("Service is running")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setPriority(NotificationCompat.PRIORITY_LOW)
        return notificationBuilder.build()
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val serviceChannel = NotificationChannel(
                CHANNEL_ID,
                "HealthConnect Service Channel",
                NotificationManager.IMPORTANCE_LOW
            )
            val manager = getSystemService(NotificationManager::class.java)
            manager?.createNotificationChannel(serviceChannel)
        }
    }

    private fun scheduleJob() {
        val componentName = ComponentName(this, HealthJobService::class.java)
        val jobInfo = JobInfo.Builder(123, componentName)
            .setRequiresDeviceIdle(false)
            .setRequiresCharging(false)
            .setPeriodic(15 * 60 * 1000)  // Každých 15 minút
            .build()

        val jobScheduler = getSystemService(Context.JOB_SCHEDULER_SERVICE) as JobScheduler
        jobScheduler.schedule(jobInfo)
    }
}
