// HealthConnectManager.kt

package com.example.healthconnect.apidemo

import android.content.Context
import android.util.Log
import androidx.health.connect.client.HealthConnectClient
import androidx.health.connect.client.permission.HealthPermission
import androidx.health.connect.client.records.SleepSessionRecord
import androidx.health.connect.client.records.SleepSessionRecord.Stage
import androidx.health.connect.client.records.StepsRecord
import androidx.health.connect.client.records.TotalCaloriesBurnedRecord
import androidx.health.connect.client.records.metadata.Device
import androidx.health.connect.client.request.ReadRecordsRequest
import androidx.health.connect.client.time.TimeRangeFilter
import java.time.Duration
import java.time.Instant
import java.time.LocalDate
import java.time.ZoneId
import java.time.ZoneOffset

class HealthConnectManager(private val context: Context) {

    private val healthConnectClient: HealthConnectClient by lazy {
        HealthConnectClient.getOrCreate(context)
    }

    // Define the permissions required
    val permissions = setOf(
        HealthPermission.getReadPermission(StepsRecord::class),
        HealthPermission.getReadPermission(SleepSessionRecord::class),
        HealthPermission.getReadPermission(TotalCaloriesBurnedRecord::class)
    )

    // Data class to hold sleep data
    data class SleepData(
        val totalSleepTime: Double,
        val deepSleepTime: Double,
        val remSleepTime: Double,
        val lightSleepTime: Double,
        val awakeTime: Double,
        val timeInBed: Double,
        val sleepOnsetLatency: Double,
        val sleepScore: Double
    ) {
        companion object {
            fun empty() = SleepData(
                totalSleepTime = 0.0,
                deepSleepTime = 0.0,
                remSleepTime = 0.0,
                lightSleepTime = 0.0,
                awakeTime = 0.0,
                timeInBed = 0.0,
                sleepOnsetLatency = 0.0,
                sleepScore = 0.0
            )
        }
    }

    // Check if all permissions are granted
    suspend fun hasAllPermissions(): Boolean {
        val grantedPermissions = healthConnectClient.permissionController.getGrantedPermissions()
        return grantedPermissions.containsAll(permissions)
    }

    // Function to read sleep data for a specific day
    suspend fun getSleepDataForDay(day: LocalDate): SleepData {
        val zoneId = ZoneId.systemDefault()
        val dayStartZoned = day.atStartOfDay(zoneId)
        val dayEndZoned = day.plusDays(1).atStartOfDay(zoneId)

        // Adjust the time range to include possible overlapping sleep sessions
        val startTime = dayStartZoned.minusHours(18).toInstant()
        val endTime = dayEndZoned.toInstant()

        // Read sleep sessions within the adjusted time range
        val sleepRecords = readSleepSessionRecords(startTime, endTime)

        if (sleepRecords.isEmpty()) {
            // Return zero values if no sleep records are found
            return SleepData.empty()
        }

        // Filter sleep sessions that end on the given day in local time
        val sessionsEndingOnDay = sleepRecords.filter { session ->
            val sessionEndZoned = session.endTime.atZone(zoneId)
            sessionEndZoned.toLocalDate() == day
        }

        if (sessionsEndingOnDay.isEmpty()) {
            return SleepData.empty()
        }

        // Find the longest sleep session
        val mainSleepSession = sessionsEndingOnDay.maxByOrNull { session ->
            Duration.between(session.startTime, session.endTime).toMillis()
        } ?: return SleepData.empty()

        // Calculate sleep statistics from the main sleep session
        return calculateSleepDataFromSession(mainSleepSession)
    }

    // Helper method to calculate sleep data from a single sleep session
    private fun calculateSleepDataFromSession(session: SleepSessionRecord): SleepData {
        // Calculate total sleep duration
        val totalSleepDuration = Duration.between(session.startTime, session.endTime)

        // Get sleep stages
        val stages = session.stages

        // Calculate durations of each sleep stage
        val deepSleepDuration = calculateSleepStageDuration(stages, SleepSessionRecord.STAGE_TYPE_DEEP)
        val remSleepDuration = calculateSleepStageDuration(stages, SleepSessionRecord.STAGE_TYPE_REM)
        val lightSleepDuration = calculateSleepStageDuration(stages, SleepSessionRecord.STAGE_TYPE_LIGHT)
        val awakeDuration = calculateAwakeDurationFromStages(stages)

        // Calculate time in bed (from session start to end)
        val timeInBedDuration = totalSleepDuration

        // Calculate sleep onset latency
        val sleepOnsetLatencyDuration = calculateSleepOnsetLatency(session, stages)

        // Convert durations to minutes
        val totalSleepTime = totalSleepDuration.toMinutes().toDouble()
        val deepSleepTime = deepSleepDuration.toMinutes().toDouble()
        val remSleepTime = remSleepDuration.toMinutes().toDouble()
        val lightSleepTime = lightSleepDuration.toMinutes().toDouble()
        val awakeTime = awakeDuration.toMinutes().toDouble()
        val timeInBed = timeInBedDuration.toMinutes().toDouble()
        val sleepOnsetLatency = sleepOnsetLatencyDuration.toMinutes().toDouble()

        // Calculate sleep score
        val sleepScore = calculateSleepScore(
            totalSleepTime = totalSleepTime,
            deepSleepTime = deepSleepTime,
            remSleepTime = remSleepTime,
            lightSleepTime = lightSleepTime,
            awakeTime = awakeTime,
            timeInBed = timeInBed,
            sleepOnsetLatency = sleepOnsetLatency
        )

        // Return the sleep data
        return SleepData(
            totalSleepTime = totalSleepTime,
            deepSleepTime = deepSleepTime,
            remSleepTime = remSleepTime,
            lightSleepTime = lightSleepTime,
            awakeTime = awakeTime,
            timeInBed = timeInBed,
            sleepOnsetLatency = sleepOnsetLatency,
            sleepScore = sleepScore
        )
    }

    // Function to read sleep session records within a time range
    suspend fun readSleepSessionRecords(startTime: Instant, endTime: Instant): List<SleepSessionRecord> {
        val request = ReadRecordsRequest(
            SleepSessionRecord::class,
            timeRangeFilter = TimeRangeFilter.between(startTime, endTime)
        )

        val records = healthConnectClient.readRecords(request).records

        // Definovať povolené typy zariadení
        val allowedDeviceTypes = setOf(
            Device.TYPE_WATCH,
            Device.TYPE_RING,
            Device.TYPE_FITNESS_BAND,
            Device.TYPE_HEAD_MOUNTED,
            Device.TYPE_CHEST_STRAP,
            Device.TYPE_UNKNOWN // Ak chceš zahrnúť aj neznáme typy
        )

        // Filtrovanie záznamov spánku
        val filteredRecords = records.filter { record ->
            val device = record.metadata.device
            val deviceType = device?.type
            // Zahrnieme záznamy, kde zariadenie nie je null a typ zariadenia je v povolených typoch
            device != null && deviceType in allowedDeviceTypes
        }

        // Logovanie filtrovaných záznamov s časmi v lokálnom čase
        val zoneId = ZoneId.systemDefault()
        Log.d("SleepData", "Počet záznamov spánku po filtrovaní: ${filteredRecords.size}")
        for (record in filteredRecords) {
            val startTimeLocal = record.startTime.atZone(zoneId)
            val endTimeLocal = record.endTime.atZone(zoneId)
            Log.d(
                "SleepData",
                "Session from ${startTimeLocal} to ${endTimeLocal}, duration: ${
                    Duration.between(record.startTime, record.endTime).toMinutes()
                } minutes"
            )
        }

        return filteredRecords
    }

    // Function to calculate the duration of a specific sleep stage
    private fun calculateSleepStageDuration(
        stages: List<Stage>,
        stageType: Int
    ): Duration {
        return stages.filter { it.stage == stageType }
            .map { Duration.between(it.startTime, it.endTime) }
            .fold(Duration.ZERO, Duration::plus)
    }

    // Function to calculate the duration of awake stages
    private fun calculateAwakeDurationFromStages(
        stages: List<Stage>
    ): Duration {
        return stages.filter {
            it.stage == SleepSessionRecord.STAGE_TYPE_AWAKE ||
                    it.stage == SleepSessionRecord.STAGE_TYPE_AWAKE_IN_BED ||
                    it.stage == SleepSessionRecord.STAGE_TYPE_OUT_OF_BED
        }
            .map { Duration.between(it.startTime, it.endTime) }
            .fold(Duration.ZERO, Duration::plus)
    }

    // Function to calculate sleep onset latency
    private fun calculateSleepOnsetLatency(
        session: SleepSessionRecord,
        stages: List<Stage>
    ): Duration {
        val sleepSessionStartTime = session.startTime

        // Find the first sleep stage that is not an awake stage
        val firstSleepStage = stages
            .filter { stage ->
                stage.stage != SleepSessionRecord.STAGE_TYPE_AWAKE &&
                        stage.stage != SleepSessionRecord.STAGE_TYPE_AWAKE_IN_BED &&
                        stage.stage != SleepSessionRecord.STAGE_TYPE_OUT_OF_BED
            }
            .minByOrNull { it.startTime }

        return if (firstSleepStage != null) {
            Duration.between(sleepSessionStartTime, firstSleepStage.startTime)
        } else {
            // If no sleep stages found, assume sleep onset latency is zero
            Duration.ZERO
        }
    }

    fun checkHealthConnectAvailability() {
        when (HealthConnectClient.getSdkStatus(context)) {
            HealthConnectClient.SDK_AVAILABLE -> {
                // SDK is available
                Log.d("HealthConnect", "Health Connect SDK is available.")
            }
            HealthConnectClient.SDK_UNAVAILABLE -> {
                // Handle SDK unavailable
                Log.d("HealthConnect", "Health Connect SDK is unavailable.")
            }
            HealthConnectClient.SDK_UNAVAILABLE_PROVIDER_UPDATE_REQUIRED -> {
                // Handle provider update required
                Log.d("HealthConnect", "Health Connect requires a provider update.")
            }
            else -> {
                // Handle other cases if any
                Log.d("HealthConnect", "Health Connect SDK status unknown.")
            }
        }
    }

    // Function to calculate the sleep score
    private fun calculateSleepScore(
        totalSleepTime: Double,
        deepSleepTime: Double,
        remSleepTime: Double,
        lightSleepTime: Double,
        awakeTime: Double,
        timeInBed: Double,
        sleepOnsetLatency: Double
    ): Double {
        // Ideal values
        val idealTotalSleep = 480.0 // 8 hours in minutes
        val idealDeepSleepPercent = 20.0
        val idealRemSleepPercent = 22.5
        val idealLightSleepPercent = 57.5
        val idealAwakeTime = 45.0
        val idealSleepEfficiency = 0.85
        val idealSleepOnsetLatency = 20.0

        // Handle division by zero
        val totalSleepTimeNonZero = if (totalSleepTime > 0) totalSleepTime else 1.0
        val timeInBedNonZero = if (timeInBed > 0) timeInBed else 1.0

        // Calculations
        val actualTotalSleepPercent = (totalSleepTime / idealTotalSleep).coerceAtMost(1.0)
        val actualDeepSleepPercent = ((deepSleepTime / totalSleepTimeNonZero) * 100).coerceAtMost(100.0)
        val actualRemSleepPercent = ((remSleepTime / totalSleepTimeNonZero) * 100).coerceAtMost(100.0)
        val actualLightSleepPercent = ((lightSleepTime / totalSleepTimeNonZero) * 100).coerceAtMost(100.0)
        val actualSleepEfficiency = (totalSleepTime / timeInBedNonZero).coerceAtMost(1.0)
        val actualAwakeTimePercent = (awakeTime / idealAwakeTime).coerceAtMost(1.0)
        val actualSOLPercent = (sleepOnsetLatency / idealSleepOnsetLatency).coerceAtMost(1.0)

        // Individual scores
        val totalSleepScore = actualTotalSleepPercent // For higher is better
        val deepSleepScore = (actualDeepSleepPercent / idealDeepSleepPercent).coerceAtMost(1.0)
        val remSleepScore = (actualRemSleepPercent / idealRemSleepPercent).coerceAtMost(1.0)
        val lightSleepScore = (actualLightSleepPercent / idealLightSleepPercent).coerceAtMost(1.0)
        val sleepEfficiencyScore = (actualSleepEfficiency / idealSleepEfficiency).coerceAtMost(1.0)
        val awakeTimeScore = 1.0 - actualAwakeTimePercent // For lower is better
        val solScore = 1.0 - actualSOLPercent // For lower is better

        // Weights
        val weights = mapOf(
            "totalSleep" to 0.20,
            "sleepEfficiency" to 0.15,
            "deepSleep" to 0.15,
            "remSleep" to 0.15,
            "lightSleep" to 0.10,
            "awakeTime" to 0.15,
            "sleepOnsetLatency" to 0.10
        )

        // Calculate total score
        val sleepScore = (
                totalSleepScore * weights["totalSleep"]!! +
                        sleepEfficiencyScore * weights["sleepEfficiency"]!! +
                        deepSleepScore * weights["deepSleep"]!! +
                        remSleepScore * weights["remSleep"]!! +
                        lightSleepScore * weights["lightSleep"]!! +
                        awakeTimeScore * weights["awakeTime"]!! +
                        solScore * weights["sleepOnsetLatency"]!!
                ) * 100.0

        return sleepScore.coerceIn(0.0, 100.0)
    }

    // Additional functions for reading steps and calories, if needed
    suspend fun readStepsRecords(startTime: Instant, endTime: Instant): List<StepsRecord> {
        val request = ReadRecordsRequest(
            StepsRecord::class,
            timeRangeFilter = TimeRangeFilter.between(startTime, endTime)
        )
        return healthConnectClient.readRecords(request).records
    }

    suspend fun readTotalCaloriesBurnedRecords(startTime: Instant, endTime: Instant): List<TotalCaloriesBurnedRecord> {
        val request = ReadRecordsRequest(
            TotalCaloriesBurnedRecord::class,
            timeRangeFilter = TimeRangeFilter.between(startTime, endTime)
        )
        return healthConnectClient.readRecords(request).records
    }
}
