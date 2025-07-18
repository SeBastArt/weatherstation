"use client"

import React, {useEffect, useState} from "react"
import {Area, AreaChart, ResponsiveContainer, XAxis, YAxis} from "recharts"
import {Card, CardContent, CardHeader, CardTitle} from "./ui/card"
import {ChartContainer, ChartTooltip, ChartTooltipContent} from "./ui/chart"
import {
    Activity, Bed, Moon, CloudSnow, Cloud, CloudRain, Home, ShowerHead, Thermometer, TreePine, User, Wind,
    CloudLightning, CloudFog, Cloudy, CloudSun, CloudRainWind, CloudMoonRain, CloudMoon, CloudHail, CloudDrizzle, SunMoon, Sun, SunSnow
} from "lucide-react"

interface RoomData {
    hour: number;
    temp: number;
}

interface RoomInfo {
    name: string;
    eui: string;
    data: RoomData[];
    current: number;
    min: number;
    max: number;
    icon: React.ComponentType<React.SVGProps<SVGSVGElement>>;
    color: string;
    lightColor: string;
    textColor: string;
    chartColor: string;
    borderColor: string;
}

// --- Wetterdaten Typen ---
interface CurrentWeather {
    city: string;
    temperature: number;
    humidity: number;
    pressure: number;
    description: string;
    icon: string;
    windSpeed: number;
    windDegree: number;
    dateTime: string;
}

interface WeatherForecastEntry {
    dateTime: string;
    temperature: number;
    humidity: number;
    pressure: number;
    description: string;
    icon: string;
    windSpeed: number;
    windDegree: number;
}

interface WeatherForecast {
    city: string;
    forecast: WeatherForecastEntry[];
}

// --- Icon Mapping für OpenWeatherMap zu Lucide ---
const weatherIconMap: Record<string, React.ComponentType<any>> = {
    // Klarer Himmel
    "01d": Sun,               // Tag: Sonne
    "01n": Moon,              // Nacht: Mond
    // Wenige Wolken
    "02d": CloudSun,          // Tag: Sonne mit Wolke
    "02n": CloudMoon,         // Nacht: Mond mit Wolke
    // Überwiegend bewölkt
    "03d": Cloudy,            // Tag: bewölkt
    "03n": Cloudy,            // Nacht: bewölkt
    // Bewölkt
    "04d": Cloud,             // Tag: Wolke
    "04n": Cloud,             // Nacht: Wolke
    // Regen
    "09d": CloudDrizzle,      // Tag: Nieselregen
    "09n": CloudDrizzle,      // Nacht: Nieselregen
    "10d": CloudRain,         // Tag: Regen
    "10n": CloudMoonRain,     // Nacht: Regen mit Mond
    // Gewitter
    "11d": CloudLightning,    // Tag: Gewitter
    "11n": CloudLightning,    // Nacht: Gewitter
    // Schnee
    "13d": CloudSnow,   // Tag: Schnee
    "13n": CloudSnow,   // Nacht: Schnee
    // Nebel
    "50d": CloudFog,          // Tag: Nebel
    "50n": CloudFog           // Nacht: Nebel
};

export default function SmartHomeTemperature() {
    // State für alle Räume
    const [roomsData, setRoomsData] = useState<{ [key: string]: RoomData[] }>({})
    const [currentTemps, setCurrentTemps] = useState<{ [key: string]: number }>({})
    const [loading, setLoading] = useState(true)

    // Wetter-States
    const [currentWeather, setCurrentWeather] = useState<CurrentWeather | null>(null)
    const [weatherForecast, setWeatherForecast] = useState<WeatherForecast | null>(null)

    // Room configuration with EUIs
    const roomConfigs = [
        {
            name: "Balkon",
            eui: "0004A30B001FB02B",
            icon: TreePine,
            color: "bg-orange-500",
            lightColor: "bg-orange-50",
            textColor: "text-orange-600",
            chartColor: "#f97316",
            borderColor: "border-orange-200",
        },
        {
            name: "Wohnzimmer",
            eui: "0004A30B001FDC0B",
            icon: Home,
            color: "bg-emerald-500",
            lightColor: "bg-emerald-50",
            textColor: "text-emerald-600",
            chartColor: "#10b981",
            borderColor: "border-emerald-200",
        },
        {
            name: "Simon",
            eui: "A84041000181854C",
            icon: User,
            color: "bg-violet-500",
            lightColor: "bg-violet-50",
            textColor: "text-violet-600",
            chartColor: "#8b5cf6",
            borderColor: "border-violet-200",
        },
        {
            name: "Schlafzimmer",
            eui: "0025CA0A00000476",
            icon: Bed,
            color: "bg-blue-500",
            lightColor: "bg-blue-50",
            textColor: "text-blue-600",
            chartColor: "#3b82f6",
            borderColor: "border-blue-200",
        },
        {
            name: "Badezimmer",
            eui: "0004A30B002240C2",
            icon: ShowerHead, // Du kannst ein passenderes Icon wählen
            color: "bg-cyan-500",
            lightColor: "bg-cyan-50",
            textColor: "text-cyan-600",
            chartColor: "#06b6d4",
            borderColor: "border-cyan-200",
        },
    ]

    // Generic function to fetch current temperature
    const fetchCurrentTemperature = async (eui: string): Promise<number | null> => {
        try {
            const response = await fetch(`https://weatherstation.wondering-developer.de/api/ThingsNetwork/GetTemperature?eui=${eui}`)
            if (response.ok) {
                const temperature = await response.json()
                return temperature
            } else {
                console.error(`Failed to fetch current temperature for ${eui}`)
                return null
            }
        } catch (error) {
            console.error(`Error fetching current temperature for ${eui}:`, error)
            return null
        }
    }

    // Generic function to fetch temperatures of today
    const fetchTemperaturesOfToday = async (eui: string): Promise<RoomData[]> => {
        try {
            const response = await fetch(`https://weatherstation.wondering-developer.de/api/ThingsNetwork/GetTemperaturesOfToday?eui=${eui}`)
            if (response.ok) {
                const temps = await response.json()
                // Map API data to chart format: hour = index, temp = value
                return temps.map((temp: number, idx: number) => ({hour: idx, temp}))
            } else {
                console.error(`Failed to fetch temperatures of today for ${eui}`)
                return []
            }
        } catch (error) {
            console.error(`Error fetching temperatures of today for ${eui}:`, error)
            return []
        }
    }

    // Wetterdaten laden
    useEffect(() => {
        const fetchWeather = async () => {
            try {
                const res = await fetch("https://weatherstation.wondering-developer.de/api/ThingsNetwork/GetCurrentWeather")
                if (res.ok) {
                    const data = await res.json()
                    setCurrentWeather(data)
                }
            } catch (e) { console.error(e) }
        }
        const fetchForecast = async () => {
            try {
                const res = await fetch("https://weatherstation.wondering-developer.de/api/ThingsNetwork/GetWeatherForecast")
                if (res.ok) {
                    const data = await res.json()
                    setWeatherForecast(data)
                }
            } catch (e) { console.error(e) }
        }
        fetchWeather()
        fetchForecast()
    }, [])

    useEffect(() => {
        const fetchAllData = async () => {
            const tempPromises = roomConfigs.map(room => fetchCurrentTemperature(room.eui))
            const dataPromises = roomConfigs.map(room => fetchTemperaturesOfToday(room.eui))

            try {
                const [currentTempsResults, roomsDataResults] = await Promise.all([
                    Promise.all(tempPromises),
                    Promise.all(dataPromises)
                ])

                // Update current temperatures
                const newCurrentTemps: { [key: string]: number } = {}
                roomConfigs.forEach((room, index) => {
                    const temp = currentTempsResults[index]
                    if (temp !== null) {
                        newCurrentTemps[room.name] = temp
                    }
                })
                setCurrentTemps(newCurrentTemps)

                // Update room data
                const newRoomsData: { [key: string]: RoomData[] } = {}
                roomConfigs.forEach((room, index) => {
                    const data = roomsDataResults[index]
                    if (data.length > 0) {
                        newRoomsData[room.name] = data
                    }
                })
                setRoomsData(newRoomsData)

            } catch (error) {
                console.error('Error fetching data:', error)
            } finally {
                setLoading(false)
            }
        }

        void fetchAllData()
    }, [roomConfigs])

    // Default data for fallback
    const getDefaultData = (roomName: string): RoomData[] => {
        const defaultData = {
            "Balkon": [
                {hour: 0, temp: 8}, {hour: 1, temp: 7}, {hour: 2, temp: 6}, {hour: 3, temp: 5},
                {hour: 4, temp: 4}, {hour: 5, temp: 5}, {hour: 6, temp: 8}, {hour: 7, temp: 12},
                {hour: 8, temp: 16}, {hour: 9, temp: 20}, {hour: 10, temp: 23}, {hour: 11, temp: 25},
                {hour: 12, temp: 27}, {hour: 13, temp: 28}, {hour: 14, temp: 29}, {hour: 15, temp: 28},
                {hour: 16, temp: 26}, {hour: 17, temp: 24}, {hour: 18, temp: 21}, {hour: 19, temp: 18},
                {hour: 20, temp: 15}, {hour: 21, temp: 13}, {hour: 22, temp: 11}, {hour: 23, temp: 9},
            ],
            "Wohnzimmer": [
                {hour: 0, temp: 20}, {hour: 1, temp: 19}, {hour: 2, temp: 19}, {hour: 3, temp: 18},
                {hour: 4, temp: 18}, {hour: 5, temp: 18}, {hour: 6, temp: 19}, {hour: 7, temp: 21},
                {hour: 8, temp: 22}, {hour: 9, temp: 23}, {hour: 10, temp: 24}, {hour: 11, temp: 24},
                {hour: 12, temp: 25}, {hour: 13, temp: 25}, {hour: 14, temp: 24}, {hour: 15, temp: 24},
                {hour: 16, temp: 23}, {hour: 17, temp: 23}, {hour: 18, temp: 22}, {hour: 19, temp: 22},
                {hour: 20, temp: 21}, {hour: 21, temp: 21}, {hour: 22, temp: 20}, {hour: 23, temp: 20},
            ],
            "Simon": [
                {hour: 0, temp: 19}, {hour: 1, temp: 18}, {hour: 2, temp: 18}, {hour: 3, temp: 17},
                {hour: 4, temp: 17}, {hour: 5, temp: 17}, {hour: 6, temp: 18}, {hour: 7, temp: 19},
                {hour: 8, temp: 20}, {hour: 9, temp: 21}, {hour: 10, temp: 22}, {hour: 11, temp: 23},
                {hour: 12, temp: 24}, {hour: 13, temp: 24}, {hour: 14, temp: 23}, {hour: 15, temp: 23},
                {hour: 16, temp: 22}, {hour: 17, temp: 22}, {hour: 18, temp: 21}, {hour: 19, temp: 21},
                {hour: 20, temp: 20}, {hour: 21, temp: 20}, {hour: 22, temp: 19}, {hour: 23, temp: 19},
            ],
            "Schlafzimmer": [
                {hour: 0, temp: 18}, {hour: 1, temp: 17}, {hour: 2, temp: 17}, {hour: 3, temp: 16},
                {hour: 4, temp: 16}, {hour: 5, temp: 16}, {hour: 6, temp: 17}, {hour: 7, temp: 18},
                {hour: 8, temp: 19}, {hour: 9, temp: 20}, {hour: 10, temp: 21}, {hour: 11, temp: 21},
                {hour: 12, temp: 22}, {hour: 13, temp: 22}, {hour: 14, temp: 21}, {hour: 15, temp: 21},
                {hour: 16, temp: 20}, {hour: 17, temp: 20}, {hour: 18, temp: 19}, {hour: 19, temp: 19},
                {hour: 20, temp: 18}, {hour: 21, temp: 18}, {hour: 22, temp: 17}, {hour: 23, temp: 17},
            ],
            "Badezimmer": [
                {hour: 0, temp: 17}, {hour: 1, temp: 16}, {hour: 2, temp: 16}, {hour: 3, temp: 15},
                {hour: 4, temp: 15}, {hour: 5, temp: 15}, {hour: 6, temp: 16}, {hour: 7, temp: 17},
                {hour: 8, temp: 18}, {hour: 9, temp: 19}, {hour: 10, temp: 20}, {hour: 11, temp: 20},
                {hour: 12, temp: 21}, {hour: 13, temp: 21}, {hour: 14, temp: 20}, {hour: 15, temp: 20},
                {hour: 16, temp: 19}, {hour: 17, temp: 19}, {hour: 18, temp: 18}, {hour: 19, temp: 18},
                {hour: 20, temp: 17}, {hour: 21, temp: 17}, {hour: 22, temp: 16}, {hour: 23, temp: 16},
            ]
        }
        return defaultData[roomName as keyof typeof defaultData] || []
    }

    const forecastData = [
        {day: "Heute", temp: 24, condition: "Sonnig", icon: Sun, color: "text-amber-500"},
        {day: "Morgen", temp: 22, condition: "Bewölkt", icon: Cloud, color: "text-slate-500"},
        {day: "Mi", temp: 19, condition: "Regen", icon: CloudRain, color: "text-blue-500"},
        {day: "Do", temp: 21, condition: "Sonnig", icon: Sun, color: "text-amber-500"},
        {day: "Fr", temp: 25, condition: "Sonnig", icon: Sun, color: "text-amber-500"},
    ]

    // Build rooms array with live data
    const rooms: RoomInfo[] = roomConfigs.map(config => {
        const roomData = roomsData[config.name] || getDefaultData(config.name)
        const currentTemp = currentTemps[config.name]

        return {
            ...config,
            data: roomData,
            current: typeof currentTemp === "number" ? Number(currentTemp.toFixed(1)) :
                (roomData.length > 0 ? Number(roomData[roomData.length - 1].temp.toFixed(1)) : 24),
            min: roomData.length > 0 ? Math.min(...roomData.map((d) => d.temp)) : 0,
            max: roomData.length > 0 ? Math.max(...roomData.map((d) => d.temp)) : 0,
        }
    })

    const chartConfig = {
        temp: {
            label: "Temperature",
            color: "hsl(var(--chart-1))",
        },
    }

    // Get Balkon current temperature for weather display
    const balkonCurrentTemp = currentTemps["Balkon"]

    // Forecast für 5 Tage, jeweils um 12:00 Uhr (Mittag) auswählen
    const getDailyNoonForecast = (forecast: WeatherForecastEntry[]) => {
        // Gruppiere nach Tag
        const grouped: { [date: string]: WeatherForecastEntry[] } = {};
        forecast.forEach(entry => {
            const date = new Date(entry.dateTime).toISOString().split("T")[0];
            if (!grouped[date]) grouped[date] = [];
            grouped[date].push(entry);
        });
        // Für jeden Tag: Forecast um 12:00 suchen, sonst möglichst nah an 12:00
        return Object.values(grouped).map(entries => {
            let noon = entries.find(e => new Date(e.dateTime).getHours() === 12);
            if (!noon) {
                noon = entries.reduce((prev, curr) => {
                    return Math.abs(new Date(curr.dateTime).getHours() - 12) < Math.abs(new Date(prev.dateTime).getHours() - 12) ? curr : prev;
                });
            }
            return noon;
        }).slice(0, 5);
    };

    return (
        <div className="min-h-screen bg-slate-50 p-4 md:p-6 lg:p-8">
            <div className="max-w-7xl mx-auto space-y-8">
                {/* Header */}
                <div className="text-center space-y-3">
                    <div className="flex items-center justify-center gap-3 mb-4">
                        <div className="p-3 bg-slate-900 rounded-2xl">
                            <Thermometer className="h-8 w-8 text-white"/>
                        </div>
                    </div>
                    <h1 className="text-4xl md:text-5xl font-bold text-slate-900 tracking-tight">Smart Home</h1>
                    <p className="text-lg text-slate-600 font-medium">Temperatur-Monitoring in Echtzeit</p>
                </div>

                {/* Weather Section */}
                <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                    {/* Current Weather */}
                    <Card
                        className="bg-white border-2 border-slate-200 shadow-lg hover:shadow-xl transition-all duration-300">
                        <CardHeader className="pb-4">
                            <CardTitle className="text-slate-900 flex items-center gap-3 text-xl">
                                <div className="p-2 bg-amber-100 rounded-xl">
                                    {currentWeather && weatherIconMap[currentWeather.icon] ?
                                        React.createElement(weatherIconMap[currentWeather.icon], {className: "h-6 w-6 text-amber-600"}) :
                                        <Sun className="h-6 w-6 text-amber-600"/>}
                                </div>
                                Aktuelles Wetter
                            </CardTitle>
                        </CardHeader>
                        <CardContent className="space-y-4">
                            <div className="flex items-center justify-between">
                                <div className="space-y-2">
                                    <div className="text-4xl font-bold text-slate-900">
                                        {loading ? "Laden..." : currentWeather ? `${currentWeather.temperature.toFixed(1)}°C` : "-"}
                                    </div>
                                    <div className="text-slate-600 font-medium">
                                        {currentWeather ? currentWeather.description : "-"}
                                    </div>
                                </div>
                                <div className="p-4 bg-amber-50 rounded-2xl">
                                    {currentWeather && weatherIconMap[currentWeather.icon] ?
                                        React.createElement(weatherIconMap[currentWeather.icon], {className: "h-12 w-12 text-amber-500"}) :
                                        <Sun className="h-12 w-12 text-amber-500"/>}
                                </div>
                            </div>
                            <div className="flex items-center gap-6 pt-2 border-t border-slate-100">
                                <div className="flex items-center gap-2 text-slate-600">
                                    <Wind className="h-4 w-4"/>
                                    <span className="font-medium">{currentWeather ? `${currentWeather.windSpeed} m/s` : "-"}</span>
                                </div>
                                <div className="text-slate-600 font-medium">
                                    Luftfeuchtigkeit: {currentWeather ? `${currentWeather.humidity}%` : "-"}
                                </div>
                            </div>
                        </CardContent>
                    </Card>

                    {/* 5-Day Forecast */}
                    <Card
                        className="bg-white border-2 border-slate-200 shadow-lg hover:shadow-xl transition-all duration-300">
                        <CardHeader className="pb-4">
                            <CardTitle className="text-slate-900 flex items-center gap-3 text-xl">
                                <div className="p-2 bg-blue-100 rounded-xl">
                                    <Activity className="h-6 w-6 text-blue-600"/>
                                </div>
                                5-Tage Vorhersage
                            </CardTitle>
                        </CardHeader>
                        <CardContent>
                            <div className="space-y-4">
                                {weatherForecast ? getDailyNoonForecast(weatherForecast.forecast).map((entry, index) => {
                                    const Icon = weatherIconMap[entry.icon] || Sun;
                                    const day = new Date(entry.dateTime).toLocaleDateString("de-DE", {weekday: "short"});
                                    return (
                                        <div key={index} className="flex items-center justify-between p-3 rounded-xl hover:bg-slate-50 transition-colors">
                                            <div className="flex items-center gap-4">
                                                <div className="p-2 bg-slate-100 rounded-lg">
                                                    <Icon className="h-5 w-5 text-blue-500"/>
                                                </div>
                                                <div>
                                                    <div className="font-semibold text-slate-900">{index === 0 ? "Heute" : day}</div>
                                                    <div className="text-sm text-slate-600">{entry.description}</div>
                                                </div>
                                            </div>
                                            <div className="text-right">
                                                <div className="text-xl font-bold text-slate-900">{entry.temperature.toFixed(1)}°C</div>
                                            </div>
                                        </div>
                                    );
                                }) : <div>Laden...</div>}
                            </div>
                        </CardContent>
                    </Card>
                </div>

                {/* Room Temperature Charts */}
                <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                    {rooms.map((room, index) => (
                        <Card
                            key={room.name}
                            className={`bg-white border-2 ${room.borderColor} shadow-lg hover:shadow-xl transition-all duration-300 overflow-hidden`}
                        >
                            <CardHeader className="pb-4">
                                <CardTitle className="flex items-center justify-between">
                                    <div className="flex items-center gap-3">
                                        <div className={`p-3 ${room.lightColor} rounded-xl`}>
                                            <room.icon className={`h-6 w-6 ${room.textColor}`}/>
                                        </div>
                                        <div>
                                            <h3 className="text-xl font-bold text-slate-900">{room.name}</h3>
                                            <p className="text-sm text-slate-600">Raumtemperatur</p>
                                        </div>
                                    </div>
                                    <div className="text-right">
                                        <div className="text-3xl font-bold text-slate-900">{room.current}°C</div>
                                        <div className="text-sm text-slate-600 font-medium">Aktuell</div>
                                    </div>
                                </CardTitle>
                            </CardHeader>
                            <CardContent className="space-y-6">
                                <div className="h-48">
                                    <ChartContainer config={chartConfig} className="h-full w-full">
                                        <ResponsiveContainer width="100%" height="100%">
                                            <AreaChart data={room.data}
                                                       margin={{top: 10, right: 10, left: 10, bottom: 10}}>
                                                <defs>
                                                    <linearGradient id={`area-${index}`} x1="0" y1="0" x2="0" y2="1">
                                                        <stop offset="5%" stopColor={room.chartColor}
                                                              stopOpacity={0.2}/>
                                                        <stop offset="95%" stopColor={room.chartColor}
                                                              stopOpacity={0.05}/>
                                                    </linearGradient>
                                                </defs>
                                                <XAxis dataKey="hour" axisLine={false} tickLine={false} tick={false}/>
                                                <YAxis
                                                    axisLine={false}
                                                    tickLine={false}
                                                    tick={{fontSize: 12, fill: "#64748b", fontWeight: 500}}
                                                    domain={["dataMin - 1", "dataMax + 1"]}
                                                    tickFormatter={(value) => `${value}°`}
                                                />
                                                <ChartTooltip
                                                    content={<ChartTooltipContent/>}
                                                    labelFormatter={(value) => `${value}:00 Uhr`}
                                                    formatter={(value) => [`${value}°C`, "Temperatur"]}
                                                />
                                                <Area
                                                    type="monotone"
                                                    dataKey="temp"
                                                    stroke={room.chartColor}
                                                    strokeWidth={3}
                                                    fill={`url(#area-${index})`}
                                                    dot={false}
                                                    activeDot={{
                                                        r: 6,
                                                        stroke: room.chartColor,
                                                        strokeWidth: 2,
                                                        fill: "white",
                                                    }}
                                                />
                                            </AreaChart>
                                        </ResponsiveContainer>
                                    </ChartContainer>
                                </div>

                                <div className="grid grid-cols-3 gap-4">
                                    <div className="text-center p-3 bg-slate-50 rounded-xl">
                                        <div className="text-lg font-bold text-blue-600">{room.min.toFixed(1)}°C</div>
                                        <div className="text-sm text-slate-600 font-medium">Minimum</div>
                                    </div>
                                    <div className="text-center p-3 bg-slate-50 rounded-xl">
                                        <div className="text-lg font-bold text-red-600">{room.max.toFixed(1)}°C</div>
                                        <div className="text-sm text-slate-600 font-medium">Maximum</div>
                                    </div>
                                    <div className="text-center p-3 bg-slate-50 rounded-xl">
                                        <div className="text-lg font-bold text-emerald-600">
                                            {(room.data.reduce((sum, d) => sum + d.temp, 0) / room.data.length).toFixed(1)}°C
                                        </div>
                                        <div className="text-sm text-slate-600 font-medium">Durchschnitt</div>
                                    </div>
                                </div>
                            </CardContent>
                        </Card>
                    ))}
                </div>

                {/* System Status */}
                <Card className="bg-white border-2 border-slate-200 shadow-lg">
                    <CardHeader>
                        <CardTitle className="text-slate-900 flex items-center gap-3 text-xl">
                            <div className="p-2 bg-green-100 rounded-xl">
                                <Activity className="h-6 w-6 text-green-600"/>
                            </div>
                            System Status
                        </CardTitle>
                    </CardHeader>
                    <CardContent>
                        <div className="grid grid-cols-2 md:grid-cols-4 gap-6">
                            <div className="text-center p-4 bg-green-50 rounded-xl border border-green-200">
                                <div className="text-2xl font-bold text-green-600 mb-1">Online</div>
                                <div className="text-sm text-slate-600 font-medium">Alle Sensoren</div>
                            </div>
                            <div className="text-center p-4 bg-blue-50 rounded-xl border border-blue-200">
                                <div className="text-2xl font-bold text-blue-600 mb-1">
                                    {rooms.length > 0 ? (rooms.reduce((sum, room) => sum + room.current, 0) / rooms.length).toFixed(1) : "21.5"}°C
                                </div>
                                <div className="text-sm text-slate-600 font-medium">Haus-Durchschnitt</div>
                            </div>
                            <div className="text-center p-4 bg-amber-50 rounded-xl border border-amber-200">
                                <div className="text-2xl font-bold text-amber-600 mb-1">65%</div>
                                <div className="text-sm text-slate-600 font-medium">Energieeffizienz</div>
                            </div>
                            <div className="text-center p-4 bg-violet-50 rounded-xl border border-violet-200">
                                <div className="text-2xl font-bold text-violet-600 mb-1">Auto</div>
                                <div className="text-sm text-slate-600 font-medium">Heizmodus</div>
                            </div>
                        </div>
                    </CardContent>
                </Card>
            </div>
        </div>
    )
}