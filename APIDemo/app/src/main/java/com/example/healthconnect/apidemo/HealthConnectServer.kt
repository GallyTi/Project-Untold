package com.example.healthconnect.apidemo

import fi.iki.elonen.NanoHTTPD
import kotlinx.coroutines.runBlocking
import java.time.Duration
import java.time.Instant
import java.time.LocalDate
import java.time.ZoneId

class HealthConnectServer(
    private val healthConnectManager: HealthConnectManager,
    port: Int = 8082
) : NanoHTTPD("0.0.0.0", port) {

    override fun serve(session: IHTTPSession?): Response {
        return runBlocking {
            when (session?.uri) {
                "/steps" -> serveSteps()
                "/calories" -> serveCalories()
                "/sleep" -> serveSleep()
                "/allData" -> serveAllData()
                else -> newFixedLengthResponse(
                    Response.Status.NOT_FOUND,
                    MIME_PLAINTEXT,
                    "Not Found"
                )
            }
        }
    }

    private suspend fun serveSteps(): Response {
        val steps = healthConnectManager.readStepsRecords(
            Instant.now().minus(Duration.ofDays(7)),
            Instant.now()
        )
        val totalSteps = steps.sumOf { it.count }
        return newFixedLengthResponse("Total steps over the last 7 days: $totalSteps")
    }

    private suspend fun serveCalories(): Response {
        val calories = healthConnectManager.readTotalCaloriesBurnedRecords(
            Instant.now().minus(Duration.ofDays(7)),
            Instant.now()
        )
        val totalCalories = calories.sumOf { it.energy.inKilocalories }
        return newFixedLengthResponse("Total calories burned over the last 7 days: $totalCalories")
    }

    private suspend fun serveSleep(): Response {
        val startTime = Instant.now().minus(Duration.ofDays(7))
        val endTime = Instant.now()

        val sleepSessions = healthConnectManager.readSleepSessionRecords(startTime, endTime)
        val longestSession = sleepSessions.maxByOrNull { session ->
            Duration.between(session.startTime, session.endTime).toMillis()
        }

        return if (longestSession != null) {
            val duration = Duration.between(longestSession.startTime, longestSession.endTime)
            val hours = duration.toHours()
            val minutes = duration.toMinutes() % 60
            newFixedLengthResponse("Longest Sleep Session Duration: ${hours} hours and ${minutes} minutes")
        } else {
            newFixedLengthResponse("No sleep data available.")
        }
    }

    private suspend fun serveAllData(): Response {
        val responseList = mutableListOf<Map<String, Any>>()

        for (i in 0 until 7) {
            val day = LocalDate.now(ZoneId.systemDefault()).minusDays(i.toLong())
            val dayStart = day.atStartOfDay(ZoneId.systemDefault()).toInstant()
            val dayEnd = day.plusDays(1).atStartOfDay(ZoneId.systemDefault()).toInstant()

            // Steps
            val steps = healthConnectManager.readStepsRecords(dayStart, dayEnd).sumOf { it.count }

            // Calories
            val calories = healthConnectManager.readTotalCaloriesBurnedRecords(dayStart, dayEnd)
                .sumOf { it.energy.inKilocalories }

            // Sleep Data
            val sleepData = healthConnectManager.getSleepDataForDay(day)

            val dayData = mapOf(
                "date" to day.toString(),
                "steps" to steps,
                "calories" to calories,
                "sleepScore" to sleepData.sleepScore,
                "totalSleepTime" to sleepData.totalSleepTime,
                "deepSleepTime" to sleepData.deepSleepTime,
                "remSleepTime" to sleepData.remSleepTime,
                "lightSleepTime" to sleepData.lightSleepTime,
                "awakeTime" to sleepData.awakeTime
            )

            responseList.add(dayData)
        }

        val responseJson = responseList.joinToString(separator = ",", prefix = "[", postfix = "]") {
            """
            {
                "date": "${it["date"]}",
                "steps": ${it["steps"]},
                "calories": ${it["calories"]},
                "sleepScore": ${it["sleepScore"]},
                "totalSleepTime": ${it["totalSleepTime"]},
                "deepSleepTime": ${it["deepSleepTime"]},
                "remSleepTime": ${it["remSleepTime"]},
                "lightSleepTime": ${it["lightSleepTime"]},
                "awakeTime": ${it["awakeTime"]}
            }
            """.trimIndent()
        }

        return newFixedLengthResponse(Response.Status.OK, "application/json", responseJson)
    }
}