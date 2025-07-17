import SmartHomeTemperature from '@/components/smart-home-temperature'
import TemperatureDashboard from '@/components/temperature-dashboard'

export default function Home() {
  return (
    <main>
      {/* Zeige zunächst das Smart Home Dashboard */}
      <SmartHomeTemperature />
      
      {/* Optional: Auch das andere Dashboard */}
      {/* <TemperatureDashboard /> */}
    </main>
  )
}
