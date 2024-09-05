package com.example.healthconnect.apidemo

import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.health.connect.client.permission.HealthPermission
import androidx.health.connect.client.records.SleepSessionRecord
import androidx.health.connect.client.records.StepsRecord
import androidx.health.connect.client.records.TotalCaloriesBurnedRecord
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import java.time.Instant
import java.time.LocalDate
import java.time.LocalDateTime
import java.time.LocalTime
import java.time.ZoneId

class MainActivity : AppCompatActivity() {
    private lateinit var healthConnectManager: HealthConnectManager
    private lateinit var requestPermissions: ActivityResultLauncher<Array<String>>
    private lateinit var dataTextView: TextView

    private val healthConnectPermissions = setOf(
        HealthPermission.getReadPermission(StepsRecord::class),
        HealthPermission.getWritePermission(StepsRecord::class),
        HealthPermission.getReadPermission(SleepSessionRecord::class),
        HealthPermission.getWritePermission(SleepSessionRecord::class),
        HealthPermission.getReadPermission(TotalCaloriesBurnedRecord::class),
        HealthPermission.getWritePermission(TotalCaloriesBurnedRecord::class)
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        healthConnectManager = HealthConnectManager(this)
        dataTextView = findViewById(R.id.text_view_data)

        healthConnectManager.checkHealthConnectAvailability()

        createRequestPermissionsObject()

        findViewById<Button>(R.id.button_read_steps_data).setOnClickListener {
            lifecycleScope.launch {
                if (!healthConnectManager.hasAllPermissions()) {
                    requestPermissions.launch(healthConnectPermissions.toTypedArray())
                } else {
                    readData()
                }
            }
        }

        startForegroundService()
    }

    private fun createRequestPermissionsObject() {
        requestPermissions = registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
            lifecycleScope.launch {
                if (healthConnectManager.hasAllPermissions()) {
                    Log.d("MainActivity", "All Health Connect permissions granted.")
                    runOnUiThread {
                        Toast.makeText(this@MainActivity, "Health Connect permissions granted", Toast.LENGTH_SHORT).show()
                    }
                    readData()
                } else {
                    Log.d("MainActivity", "Health Connect permissions not granted.")
                    showPermissionSettingsDialog()
                }
            }
        }
    }

    private fun showPermissionSettingsDialog() {
        runOnUiThread {
            AlertDialog.Builder(this@MainActivity)
                .setMessage("Permissions not granted. Please enable all permissions in the app settings.")
                .setPositiveButton("Settings") { _, _ ->
                    val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                        data = Uri.parse("package:$packageName")
                    }
                    startActivity(intent)
                }
                .setNegativeButton("Cancel", null)
                .show()
        }
    }

    private fun readData() {
        lifecycleScope.launch {
            try {
                if (!healthConnectManager.hasAllPermissions()) {
                    requestPermissions.launch(healthConnectPermissions.toTypedArray())
                    return@launch
                }

                val startTime = LocalDate.now().atStartOfDay(ZoneId.systemDefault()).toInstant()
                val endTime = Instant.now()

                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                    readDataForAndroid14AndAbove(startTime, endTime)
                } else {
                    readDataForAndroid13AndBelow(startTime, endTime)
                }
            } catch (e: Exception) {
                Log.e("MainActivity", "Error reading data", e)
            }
        }
    }

    private suspend fun readDataForAndroid14AndAbove(startTime: Instant, endTime: Instant) {
        Log.d("MainActivity", "Reading data from $startTime to $endTime")

        val stepsRecords = healthConnectManager.readStepsRecords(startTime, endTime)
        val totalSteps = stepsRecords.sumOf { it.count }
        Log.d("MainActivity", "Total Steps: $totalSteps")

        val calorieRecords = healthConnectManager.readTotalCaloriesBurnedRecords(startTime, endTime)
        val totalCalories = calorieRecords.sumOf { it.energy.inKilocalories }
        Log.d("MainActivity", "Total Calories: $totalCalories")

        val sleepStart = LocalDateTime.of(LocalDate.now().minusDays(1), LocalTime.of(18, 0)).atZone(ZoneId.systemDefault()).toInstant()
        val sleepScore = healthConnectManager.getSleepScoreForPeriod(sleepStart, endTime)
        Log.d("MainActivity", "Sleep Score: $sleepScore")

        displayData("Steps taken today: $totalSteps\nCalories burned: $totalCalories\nSleep Score: $sleepScore")
    }

    private suspend fun readDataForAndroid13AndBelow(startTime: Instant, endTime: Instant) {
        // Použitie rovnakých funkcií pre Android 13 a nižšie, pretože `Legacy` funkcie už nemáme
        readDataForAndroid14AndAbove(startTime, endTime)
    }

    private fun displayData(data: String) {
        runOnUiThread {
            dataTextView.text = data
        }
    }

    private fun startForegroundService() {
        val serviceIntent = Intent(this, HealthForegroundService::class.java)
        lifecycleScope.launch {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                if (healthConnectManager.hasAllPermissions()) {
                    startForegroundService(serviceIntent)
                } else {
                    requestPermissions.launch(healthConnectPermissions.toTypedArray())
                }
            } else {
                startForegroundService(serviceIntent)
            }
        }
    }
}