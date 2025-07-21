# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a weather station system with a microservices architecture that collects IoT sensor data and displays weather information. The system consists of:

- **Backend APIs**: ASP.NET Core Web APIs for data processing and external API integration
- **Frontend**: Blazor WebAssembly application for web interface
- **Temperature Dashboard**: Next.js React application for modern data visualization
- **IoT Client**: Arduino-based E-Paper display client
- **Infrastructure**: Docker, Kubernetes, and Helm configurations for deployment

## Development Commands

### .NET Projects (BackendApi, ThingsApi, Frontend, Application)
```bash
# Build and run locally
cd src/[ProjectName]
dotnet restore
dotnet build
dotnet run

# Run tests
dotnet test src/[ProjectName]

# Docker build
docker build -t [projectname] .
```

### Next.js Dashboard (temperature-dashboard)
```bash
# Development
cd src/temperature-dashboard
npm install
npm run dev          # Development server with Turbopack
npm run build        # Production build
npm start            # Production server
npm run lint         # ESLint

# Docker build
docker build -t temperature-dashboard .
```

### Docker Compose (Full Stack)
```bash
# Start all services
docker-compose up

# Start specific service
docker-compose up [service-name]
```

### Kubernetes/Helm Deployment
```bash
# Deploy to Kubernetes
helm upgrade --install weatherstation ./helm/weatherstation \
  --create-namespace \
  --namespace weatherstation \
  --set backend.imageVersion=[version] \
  --set frontend.imageVersion=[version]
```

## Architecture Overview

### Core Services

1. **Application Library** (`src/Application/`)
   - Shared business logic and data models
   - InfluxDB service for time-series data storage
   - Payload processors for different IoT device types (Dragino, Decentlab, Liard)
   - Interfaces: `IInfluxDbService`, `IInfluxDbConfig`, `IPayloadProcessor`

2. **BackendApi** (`src/BackendApi/`)
   - Main REST API aggregating data from multiple sources
   - Endpoints for sensor data (temperature, humidity, battery)
   - OpenWeatherMap API integration for external weather data
   - Controllers: `ThingsNetworkController`

3. **ThingsApi** (`src/ThingsApi/`)
   - Dedicated API for IoT data ingestion from The Things Network
   - Processes webhook payloads from LoRaWAN devices
   - Stores sensor data in InfluxDB

4. **Frontend** (`src/Frontend/`)
   - Blazor WebAssembly application
   - Consumes BackendApi for data display

5. **Temperature Dashboard** (`src/temperature-dashboard/`)
   - Modern React/Next.js dashboard
   - Uses Recharts for data visualization
   - TailwindCSS for styling, Lucide React for icons
   - Configurable backend URL via `NEXT_PUBLIC_BACKEND_API_URL`

### Data Flow

1. IoT sensors → The Things Network → ThingsApi webhook → InfluxDB
2. BackendApi queries InfluxDB + external APIs → Frontend/Dashboard
3. Payload processing handles different device types via factory pattern

### Configuration

- **Environment Variables**: Stored in `.env` files per service
- **InfluxDB**: Time-series database for sensor data storage
- **OpenWeatherMap**: External weather API integration
- **Docker**: Multi-container setup with service dependencies

### Deployment Architecture

- **Docker Compose**: Local development environment
- **Kubernetes**: Production deployment with Istio service mesh
- **Helm Charts**: Infrastructure as code in `helm/weatherstation/`
- **GitHub Actions**: CI/CD pipeline building ARM64 images
- **InfluxDB**: Persistent volume for data storage

## Key Development Patterns

### Payload Processing
The system uses a factory pattern for handling different IoT device payloads:
- `PayloadProcessorFactory` creates appropriate processors
- Device-specific processors: `DraginoPayloadProcessor`, `DecentlabPayloadProcessor`, `LiardPayloadProcessor`

### Data Models
- `WeatherDataPoint`: Core sensor data structure
- `TheThingsPayload<T>`: Generic payload wrapper for The Things Network
- DTOs for API responses: `CurrentWeatherDto`, `WeatherForecastDto`

### Service Dependencies
- All .NET projects depend on the Application library
- Services communicate via HTTP APIs
- InfluxDB serves as the central data store for time-series data

## Environment Configuration

Each service requires specific environment variables for proper operation:
- InfluxDB connection settings (`INFLUXDB_HOST`, `INFLUXDB_ADMIN_TOKEN`, etc.)
- OpenWeatherMap API key for external weather data
- Backend URL configuration for frontend services