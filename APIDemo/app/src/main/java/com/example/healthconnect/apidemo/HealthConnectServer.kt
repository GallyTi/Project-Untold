package com.example.healthconnect.apidemo

import fi.iki.elonen.NanoHTTPD
import kotlinx.coroutines.runBlocking
import java.time.Instant
import java.time.LocalDate
import java.time.ZoneId

class HealthConnectServer(private val healthConnectManager: HealthConnectManager, port: Int) : NanoHTTPD("127.0.0.1", 8082) {

    override fun serve(session: IHTTPSession?): Response {
        return runBlocking {
            when (session?.uri) {
                "/steps" -> serveSteps()
                "/calories" -> serveCalories()
                "/sleep" -> serveSleep()
                "/allData" -> serveAllData()
                else -> newFixedLengthResponse(Response.Status.NOT_FOUND, MIME_PLAINTEXT, "Not Found")
            }
        }
    }

    private suspend fun serveSteps(): Response {
        val steps = healthConnectManager.readStepsRecords(Instant.now().minusSeconds(86400 * 7), Instant.now())
        val totalSteps = steps.sumOf { it.count }
        return newFixedLengthResponse("Total steps over the last 7 days: $totalSteps")
    }

    private suspend fun serveCalories(): Response {
        val calories = healthConnectManager.readTotalCaloriesBurnedRecords(Instant.now().minusSeconds(86400 * 7), Instant.now())
        val totalCalories = calories.sumOf { it.energy.inKilocalories }
        return newFixedLengthResponse("Total calories burned over the last 7 days: $totalCalories")
    }

    private suspend fun serveSleep(): Response {
        val sleepScore = healthConnectManager.getSleepScoreForPeriod(
            Instant.now().minusSeconds(86400 * 7), Instant.now()
        )
        return newFixedLengthResponse("Sleep Score over the last 7 days: $sleepScore")
    }

    private suspend fun serveAllData(): Response {
        val responseList = mutableListOf<Map<String, Any>>()

        for (i in 0 until 7) {
            val startOfDay = LocalDate.now().minusDays(i.toLong()).atStartOfDay(ZoneId.systemDefault()).toInstant()
            val endOfDay = startOfDay.plusSeconds(86400)

            val steps = healthConnectManager.readStepsRecords(startOfDay, endOfDay).sumOf { it.count }
            val calories = healthConnectManager.readTotalCaloriesBurnedRecords(startOfDay, endOfDay).sumOf { it.energy.inKilocalories }

            val sleepScore = healthConnectManager.getSleepScoreForPeriod(startOfDay, endOfDay)

            val dayData = mapOf(
                "date" to startOfDay.toString(),
                "steps" to steps,
                "calories" to calories,
                "sleepScore" to sleepScore
            )

            responseList.add(dayData)
        }

        val responseJson = responseList.joinToString(separator = ",", prefix = "[", postfix = "]") {
            """
            {
                "date": "${it["date"]}",
                "steps": ${it["steps"]},
                "calories": ${it["calories"]},
                "sleepScore": ${it["sleepScore"]}
            }
            """.trimIndent()
        }

        return newFixedLengthResponse(Response.Status.OK, "application/json", responseJson)
    }
}
