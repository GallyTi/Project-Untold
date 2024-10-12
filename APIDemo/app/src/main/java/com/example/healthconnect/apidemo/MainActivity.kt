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
import java.time.Duration
import java.time.Instant
import java.time.LocalDate
import java.time.ZoneId

class MainActivity : AppCompatActivity() {
    private lateinit var healthConnectManager: HealthConnectManager
    private lateinit var requestPermissions: ActivityResultLauncher<Array<String>>
    private lateinit var dataTextView: TextView

    private val healthConnectPermissions = setOf(
        HealthPermission.getReadPermission(StepsRecord::class),
        HealthPermission.getReadPermission(SleepSessionRecord::class),
        HealthPermission.getReadPermission(TotalCaloriesBurnedRecord::class)
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
        requestPermissions =
            registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
                lifecycleScope.launch {
                    if (healthConnectManager.hasAllPermissions()) {
                        Log.d("MainActivity", "All Health Connect permissions granted.")
                        runOnUiThread {
                            Toast.makeText(
                                this@MainActivity,
                                "Health Connect permissions granted",
                                Toast.LENGTH_SHORT
                            ).show()
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

                val zoneId = ZoneId.systemDefault()
                val today = LocalDate.now(zoneId)
                val startTime = today.atStartOfDay(zoneId).toInstant()
                val endTime = Instant.now()

                readDataForAndroid14AndAbove(startTime, endTime)
            } catch (e: Exception) {
                Log.e("MainActivity", "Error reading data", e)
            }
        }
    }

    private suspend fun readDataForAndroid14AndAbove(startTime: Instant, endTime: Instant) {
        Log.d("MainActivity", "Reading data from $startTime to $endTime")

        val zoneId = ZoneId.systemDefault()
        val today = LocalDate.now(zoneId)

        // Steps for today
        val stepsRecords = healthConnectManager.readStepsRecords(startTime, endTime)
        val totalSteps = stepsRecords.sumOf { it.count }
        Log.d("MainActivity", "Total Steps: $totalSteps")

        // Calories for today
        val calorieRecords = healthConnectManager.readTotalCaloriesBurnedRecords(startTime, endTime)
        val totalCalories = calorieRecords.sumOf { it.energy.inKilocalories }
        Log.d("MainActivity", "Total Calories: $totalCalories")

        // Sleep Data for today
        val sleepData = healthConnectManager.getSleepDataForDay(today)
        Log.d("MainActivity", "Sleep Data for today: $sleepData")

        displayData(
            "Steps taken today: $totalSteps\n" +
                    "Calories burned today: $totalCalories\n" +
                    "Total Sleep Time: ${sleepData.totalSleepTime} minutes\n" +
                    "Sleep Score: ${sleepData.sleepScore}\n" +
                    "Deep Sleep: ${sleepData.deepSleepTime} minutes\n" +
                    "REM Sleep: ${sleepData.remSleepTime} minutes\n" +
                    "Light Sleep: ${sleepData.lightSleepTime} minutes\n" +
                    "Awake Time: ${sleepData.awakeTime} minutes"
        )
    }

    private fun displayData(data: String) {
        runOnUiThread {
            dataTextView.text = data
        }
    }

    private fun startForegroundService() {
        val serviceIntent = Intent(this, HealthForegroundService::class.java)
        lifecycleScope.launch {
            if (healthConnectManager.hasAllPermissions()) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    startForegroundService(serviceIntent)
                } else {
                    startService(serviceIntent)
                }
            } else {
                requestPermissions.launch(healthConnectPermissions.toTypedArray())
            }
        }
    }
}
