package com.example.healthconnect.apidemo

import android.content.Context
import androidx.activity.result.ActivityResultLauncher
import androidx.health.connect.client.HealthConnectClient
import androidx.health.connect.client.permission.HealthPermission
import androidx.health.connect.client.records.SleepSessionRecord
import androidx.health.connect.client.records.StepsRecord
import androidx.health.connect.client.records.TotalCaloriesBurnedRecord
import androidx.health.connect.client.request.ReadRecordsRequest
import androidx.health.connect.client.time.TimeRangeFilter
import java.time.Duration
import java.time.Instant
import java.time.ZoneOffset

class HealthConnectManager(private val context: Context) {
    val healthConnectClient: HealthConnectClient by lazy {
        HealthConnectClient.getOrCreate(context)
    }

    val permissions = setOf(
        HealthPermission.getWritePermission(StepsRecord::class),
        HealthPermission.getReadPermission(StepsRecord::class),
        HealthPermission.getWritePermission(SleepSessionRecord::class),
        HealthPermission.getReadPermission(SleepSessionRecord::class),
        HealthPermission.getWritePermission(TotalCaloriesBurnedRecord::class),
        HealthPermission.getReadPermission(TotalCaloriesBurnedRecord::class)
    )

    suspend fun hasAllPermissions(): Boolean {
        val grantedPermissions = healthConnectClient.permissionController.getGrantedPermissions()
        return grantedPermissions.containsAll(permissions)
    }

    fun requestPermissionsLauncher(launcher: ActivityResultLauncher<Array<String>>) {
        launcher.launch(permissions.map { it.toString() }.toTypedArray())
    }

    suspend fun readStepsRecords(startTime: Instant, endTime: Instant): List<StepsRecord> {
        val request = ReadRecordsRequest(
            StepsRecord::class,
            timeRangeFilter = TimeRangeFilter.between(startTime, endTime)
        )
        return healthConnectClient.readRecords(request).records
    }

    suspend fun readStepsRecordsLegacy(startTime: Instant, endTime: Instant): List<StepsRecord> {
        return readStepsRecords(startTime, endTime)
    }

    suspend fun readTotalCaloriesBurnedRecords(startTime: Instant, endTime: Instant): List<TotalCaloriesBurnedRecord> {
        val request = ReadRecordsRequest(
            TotalCaloriesBurnedRecord::class,
            timeRangeFilter = TimeRangeFilter.between(startTime, endTime)
        )
        return healthConnectClient.readRecords(request).records
    }

    suspend fun getSleepScoreForPeriod(startTime: Instant, endTime: Instant): Double {
        val sleepRecords = readSleepSessionRecords(startTime, endTime)
        val mergedRecords = mergeCloseSleepSessions(sleepRecords)
        val longestSleepRecord = mergedRecords.maxByOrNull { Duration.between(it.startTime, it.endTime).toMinutes() }

        if (longestSleepRecord == null) {
            return 0.0
        }

        val totalSleepTime = Duration.between(longestSleepRecord.startTime, longestSleepRecord.endTime).toHours().toDouble()
        val deepSleepTime = calculateDeepSleepDurationFromStages(longestSleepRecord.stages).toHours().toDouble()
        val remSleepTime = calculateRemSleepDurationFromStages(longestSleepRecord.stages).toHours().toDouble()

        val interruptionPenalty = calculateInterruptionPenalty(longestSleepRecord.interruptions, longestSleepRecord.interruptionDurationMinutes)

        return calculateSleepScore(totalSleepTime, deepSleepTime, remSleepTime) - interruptionPenalty
    }

    private suspend fun readSleepSessionRecords(startTime: Instant, endTime: Instant): List<SleepSessionRecord> {
        val request = ReadRecordsRequest(
            SleepSessionRecord::class,
            timeRangeFilter = TimeRangeFilter.between(startTime, endTime)
        )
        return healthConnectClient.readRecords(request).records
    }

    data class MergedSleepSession(
        val startTime: Instant,
        val endTime: Instant,
        val startZoneOffset: ZoneOffset?,
        val endZoneOffset: ZoneOffset?,
        val stages: List<SleepSessionRecord.Stage>,
        val interruptions: Int,
        val interruptionDurationMinutes: Long
    )

    private fun mergeCloseSleepSessions(sleepRecords: List<SleepSessionRecord>, maxGapMinutes: Long = 30): List<MergedSleepSession> {
        if (sleepRecords.isEmpty()) return emptyList()

        val mergedRecords = mutableListOf<MergedSleepSession>()
        var currentRecord = sleepRecords.first()
        var interruptions = 0
        var totalInterruptionDuration = 0L

        for (record in sleepRecords.drop(1)) {
            val gapDuration = Duration.between(currentRecord.endTime, record.startTime).toMinutes()
            if (gapDuration <= maxGapMinutes) {
                interruptions += 1
                totalInterruptionDuration += gapDuration
                currentRecord = SleepSessionRecord(
                    startTime = currentRecord.startTime,
                    endTime = record.endTime,
                    startZoneOffset = currentRecord.startZoneOffset,
                    endZoneOffset = record.endZoneOffset,
                    stages = currentRecord.stages + record.stages
                )
            } else {
                mergedRecords.add(MergedSleepSession(
                    startTime = currentRecord.startTime,
                    endTime = currentRecord.endTime,
                    startZoneOffset = currentRecord.startZoneOffset,
                    endZoneOffset = currentRecord.endZoneOffset,
                    stages = currentRecord.stages,
                    interruptions = interruptions,
                    interruptionDurationMinutes = totalInterruptionDuration
                ))
                currentRecord = record
                interruptions = 0
                totalInterruptionDuration = 0L
            }
        }
        mergedRecords.add(MergedSleepSession(
            startTime = currentRecord.startTime,
            endTime = currentRecord.endTime,
            startZoneOffset = currentRecord.startZoneOffset,
            endZoneOffset = currentRecord.endZoneOffset,
            stages = currentRecord.stages,
            interruptions = interruptions,
            interruptionDurationMinutes = totalInterruptionDuration
        ))

        return mergedRecords
    }

    private fun calculateInterruptionPenalty(interruptions: Int, interruptionDurationMinutes: Long): Double {
        val penaltyPerInterruption = 5.0  // Penalizácia za každé prerušenie
        val penaltyPerMinute = 0.1  // Penalizácia za každú minútu prerušenia

        return (interruptions * penaltyPerInterruption) + (interruptionDurationMinutes * penaltyPerMinute)
    }

    private fun calculateDeepSleepDurationFromStages(stages: List<SleepSessionRecord.Stage>): Duration {
        return stages.filter { it.stage == SleepSessionRecord.STAGE_TYPE_DEEP }
            .map { Duration.between(it.startTime, it.endTime) }
            .fold(Duration.ZERO, Duration::plus)
    }

    private fun calculateRemSleepDurationFromStages(stages: List<SleepSessionRecord.Stage>): Duration {
        return stages.filter { it.stage == SleepSessionRecord.STAGE_TYPE_REM }
            .map { Duration.between(it.startTime, it.endTime) }
            .fold(Duration.ZERO, Duration::plus)
    }

    private fun calculateSleepScore(
        totalSleepTime: Double,
        deepSleepTime: Double,
        remSleepTime: Double
    ): Double {
        val deepSleepScore = normalize(deepSleepTime, ideal = 1.5) // Ideálna hodnota pre deep sleep
        val remSleepScore = normalize(remSleepTime, ideal = 2.0)   // Ideálna hodnota pre REM sleep
        val totalSleepScore = normalize(totalSleepTime, ideal = 7.0) // Ideálna celková doba spánku

        return (deepSleepScore + remSleepScore + totalSleepScore) / 3.0 * 100
    }

    private fun normalize(actual: Double, ideal: Double): Double {
        return if (actual >= ideal) {
            1.0
        } else {
            actual / ideal
        }
    }

    fun checkHealthConnectAvailability() {
        when (HealthConnectClient.getSdkStatus(context)) {
            HealthConnectClient.SDK_UNAVAILABLE -> {
                // Implementácia logiky na ošetrenie, keď Health Connect SDK nie je dostupné
            }
            HealthConnectClient.SDK_UNAVAILABLE_PROVIDER_UPDATE_REQUIRED -> {
                // Implementácia logiky na ošetrenie, keď Health Connect SDK potrebuje aktualizáciu poskytovateľa
            }
            else -> {
                // Health Connect SDK je dostupné
            }
        }
    }
}