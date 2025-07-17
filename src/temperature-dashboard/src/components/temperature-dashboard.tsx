"use client"

import { LineChart, Line, XAxis, YAxis, ResponsiveContainer } from "recharts"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { ChartContainer, ChartTooltip, ChartTooltipContent } from "@/components/ui/chart"
import { Thermometer, Sun, Cloud, CloudRain, Snowflake } from "lucide-react"

export default function TemperatureDashboard() {
  // Sample data for different scenarios
  const springData = [
    { time: "6AM", temp: 12 },
    { time: "9AM", temp: 16 },
    { time: "12PM", temp: 22 },
    { time: "3PM", temp: 25 },
    { time: "6PM", temp: 20 },
    { time: "9PM", temp: 15 },
  ]

  const summerData = [
    { time: "6AM", temp: 22 },
    { time: "9AM", temp: 28 },
    { time: "12PM", temp: 35 },
    { time: "3PM", temp: 38 },
    { time: "6PM", temp: 32 },
    { time: "9PM", temp: 26 },
  ]

  const autumnData = [
    { time: "6AM", temp: 8 },
    { time: "9AM", temp: 12 },
    { time: "12PM", temp: 18 },
    { time: "3PM", temp: 20 },
    { time: "6PM", temp: 14 },
    { time: "9PM", temp: 10 },
  ]

  const winterData = [
    { time: "6AM", temp: -2 },
    { time: "9AM", temp: 1 },
    { time: "12PM", temp: 5 },
    { time: "3PM", temp: 7 },
    { time: "6PM", temp: 3 },
    { time: "9PM", temp: -1 },
  ]

  const chartConfig = {
    temp: {
      label: "Temperature",
      color: "hsl(var(--chart-1))",
    },
  }

  const currentTemp = 23
  const currentCondition = "Sunny"

  const getSeasonIcon = (season: string) => {
    switch (season) {
      case "Spring":
        return <Sun className="h-5 w-5 text-yellow-500" />
      case "Summer":
        return <Sun className="h-5 w-5 text-orange-500" />
      case "Autumn":
        return <Cloud className="h-5 w-5 text-gray-500" />
      case "Winter":
        return <Snowflake className="h-5 w-5 text-blue-500" />
      default:
        return <Thermometer className="h-5 w-5" />
    }
  }

  const seasons = [
    { name: "Balkon", data: springData, color: "hsl(142, 76%, 36%)" },
    { name: "Wohnzimmer", data: summerData, color: "hsl(25, 95%, 53%)" },
    { name: "Schlafzimmer", data: autumnData, color: "hsl(32, 95%, 44%)" },
    { name: "Simon", data: winterData, color: "hsl(221, 83%, 53%)" },
  ]

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-100 p-4">
      <div className="max-w-7xl mx-auto space-y-6">
        {/* Header */}
        <div className="text-center space-y-2">
          <h1 className="text-3xl font-bold text-gray-900">Temperature Dashboard</h1>
          <p className="text-gray-600">Daily temperature variations across seasons</p>
        </div>

        {/* Current Temperature Card */}
        <Card className="max-w-md mx-auto bg-white/80 backdrop-blur-sm border-0 shadow-lg">
          <CardContent className="pt-6">
            <div className="flex items-center justify-center space-x-4">
              <Thermometer className="h-8 w-8 text-red-500" />
              <div className="text-center">
                <div className="text-4xl font-bold text-gray-900">{currentTemp}°C</div>
                <div className="text-sm text-gray-600">{currentCondition}</div>
              </div>
            </div>
          </CardContent>
        </Card>

        {/* Temperature Charts Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          {seasons.map((season) => (
            <Card key={season.name} className="bg-white/80 backdrop-blur-sm border-0 shadow-lg">
              <CardHeader className="pb-2">
                <CardTitle className="flex items-center space-x-2 text-lg">
                  {getSeasonIcon(season.name)}
                  <span>{season.name}</span>
                </CardTitle>
              </CardHeader>
              <CardContent>
                <ChartContainer config={chartConfig} className="h-[200px] w-full">
                  <ResponsiveContainer width="100%" height="100%">
                    <LineChart data={season.data} margin={{ top: 5, right: 5, left: 5, bottom: 5 }}>
                      <XAxis dataKey="time" axisLine={false} tickLine={false} tick={{ fontSize: 10 }} />
                      <YAxis
                        axisLine={false}
                        tickLine={false}
                        tick={{ fontSize: 10 }}
                        domain={["dataMin - 2", "dataMax + 2"]}
                      />
                      <ChartTooltip
                        content={<ChartTooltipContent />}
                        labelFormatter={(value) => `Time: ${value}`}
                        formatter={(value) => [`${value}°C`, "Temperature"]}
                      />
                      <Line
                        type="monotone"
                        dataKey="temp"
                        stroke={season.color}
                        strokeWidth={3}
                        dot={{ fill: season.color, strokeWidth: 2, r: 4 }}
                        activeDot={{ r: 6, stroke: season.color, strokeWidth: 2 }}
                      />
                    </LineChart>
                  </ResponsiveContainer>
                </ChartContainer>
                <div className="mt-2 text-center">
                  <div className="text-sm text-gray-600">
                    Range: {Math.min(...season.data.map((d) => d.temp))}°C -{" "}
                    {Math.max(...season.data.map((d) => d.temp))}°C
                  </div>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>

        {/* Additional Info */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mt-8">
          <Card className="bg-white/80 backdrop-blur-sm border-0 shadow-lg">
            <CardContent className="pt-6 text-center">
              <Sun className="h-8 w-8 text-yellow-500 mx-auto mb-2" />
              <div className="text-2xl font-bold text-gray-900">UV Index</div>
              <div className="text-lg text-gray-600">6 - High</div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-sm border-0 shadow-lg">
            <CardContent className="pt-6 text-center">
              <CloudRain className="h-8 w-8 text-blue-500 mx-auto mb-2" />
              <div className="text-2xl font-bold text-gray-900">Humidity</div>
              <div className="text-lg text-gray-600">65%</div>
            </CardContent>
          </Card>

          <Card className="bg-white/80 backdrop-blur-sm border-0 shadow-lg">
            <CardContent className="pt-6 text-center">
              <Cloud className="h-8 w-8 text-gray-500 mx-auto mb-2" />
              <div className="text-2xl font-bold text-gray-900">Wind Speed</div>
              <div className="text-lg text-gray-600">12 km/h</div>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  )
}
