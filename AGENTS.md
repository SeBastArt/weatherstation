---
name: "BackendApi"
description: "Zentrales ASP.NET Core Web API Backend für die Wetterstation, entwickelt mit .NET 8. Aggregiert Daten aus verschiedenen Quellen und stellt Endpunkte für Frontend und weitere Services bereit."
category: "Backend"
tags: ["csharp", ".NET", "api", "weather", "backend"]
lastUpdated: "2025-07-21"
---

# AGENTS.md: Hinweise für Jules

## Übersicht
Das Projekt `BackendApi` befindet sich unter `src/BackendApi` und ist das zentrale Backend der Wetterstation. Es stellt REST-API-Endpunkte bereit, aggregiert Daten aus ThingsApi, Application und externen Quellen (z.B. OpenWeatherMap) und ist für die Kommunikation mit Frontend und Dashboard zuständig.

## Build- und Testanweisungen

- **Lokal ausführen:**
    1. Ins Verzeichnis `src/BackendApi` wechseln.
    2. Folgende Befehle ausführen:
        ```
        dotnet restore
        dotnet build
        dotnet run
        ```
    3. Die API ist dann unter `http://localhost:5000` (oder im Terminal angegebenen Port) erreichbar.

- **Docker:**
    1. Im Verzeichnis `src/BackendApi`:
        ```
        docker build -t backendapi .
        docker run -p 5000:80 backendapi
        ```
    2. Alternativ über das Root-Docker-Compose:
        ```
        docker-compose up backendapi
        ```

- **Kubernetes/Helm:**
    - Deployment erfolgt über das Helm Chart im Verzeichnis `helm/weatherstation`. Die Konfiguration für BackendApi liegt unter `helm/weatherstation/templates/backend/`.

- **Tests:**
    - Unit-Tests können mit folgendem Befehl ausgeführt werden:
        ```
        dotnet test src/BackendApi
        ```
    - Die API-Dokumentation ist nach dem Starten unter `/swagger` verfügbar.

## Zusammenspiel mit anderen Projekten

- Nutzt die Application-Library (`src/Application`) für Geschäftslogik und Datenzugriff.
- Kommuniziert mit ThingsApi (`src/ThingsApi`) für IoT-Daten.
- Stellt Endpunkte für Frontend (`src/Frontend`) und das Temperatur-Dashboard (`src/temperature-dashboard`) bereit.
- Externe API-Keys (z.B. OpenWeatherMap) werden über Umgebungsvariablen oder Secrets (z.B. GitHub Actions, Helm Secrets) bereitgestellt.

## Troubleshooting/Setup-Hinweise

- Benötigte .NET-Version: .NET 8 (ggf. in Setup-Skript installieren: `sudo apt-get update && sudo apt-get install -y dotnet-sdk-8.0`)
- Tests laufen mit: `dotnet test src/BackendApi`
- Für API-Änderungen immer die OpenAPI-Spezifikation (`swagger.json`) aktualisieren.

---

name: "temperature-dashboard"
description: "React-basiertes Next.js Frontend für die Anzeige von Temperatur- und Wetterdaten. Nutzt Lucide-Icons, TailwindCSS und kommuniziert mit dem BackendApi für Live-Daten."
category: "Frontend"
tags: ["react", "nextjs", "dashboard", "weather", "frontend"]
lastUpdated: "2025-07-21"
---

## Übersicht
Das Projekt `temperature-dashboard` befindet sich unter `src/temperature-dashboard`. Es ist ein modernes Web-Frontend, das Wetter- und Temperaturdaten aus dem BackendApi visualisiert. Die Anwendung nutzt Next.js, React, Lucide-Icons und TailwindCSS für UI und Styling.

## Build- und Testanweisungen

- **Lokal ausführen:**
    1. Ins Verzeichnis `src/temperature-dashboard` wechseln.
    2. Abhängigkeiten installieren:
        ```
        npm install
        ```
    3. Entwicklungsserver starten:
        ```
        npm run dev
        ```
    4. Die Anwendung ist dann unter `http://localhost:3000` erreichbar.

- **Build für Produktion:**
    ```
    npm run build
    npm start
    ```

- **Docker:**
    1. Im Verzeichnis `src/temperature-dashboard`:
        ```
        docker build -t temperature-dashboard .
        docker run -p 3000:3000 temperature-dashboard
        ```
    2. Alternativ über das Root-Docker-Compose:
        ```
        docker-compose up temperature-dashboard
        ```

- **Tests:**
    - Linting:
        ```
        npm run lint
        ```
    - (Optional) Unit-Tests können mit gängigen React-Testframeworks ergänzt werden.

## Zusammenspiel mit anderen Projekten

- Holt Wetter- und Temperaturdaten über REST-Endpunkte vom BackendApi (`src/BackendApi`).
- Zeigt die Daten in Echtzeit und mit passenden Wetter-Icons an.
- Kann in Kubernetes/Helm über das Chart `helm/weatherstation` ausgerollt werden (Konfiguration unter `helm/weatherstation/templates/frontend/`).

## Troubleshooting/Setup-Hinweise

- Benötigte Node.js-Version: >=18
- Bei Problemen mit Abhängigkeiten: `npm install --force`
- Für UI-Änderungen TailwindCSS und Lucide-Icons Dokumentation beachten.

---

name: "Frontend"
description: "Blazor-basiertes Web-Frontend für die Wetterstation. Stellt eine Benutzeroberfläche für die Anzeige und Interaktion mit Sensordaten und Wetterinformationen bereit. Entwickelt mit .NET 8."
category: "Frontend"
tags: ["csharp", ".NET", "blazor", "frontend", "weather"]
lastUpdated: "2025-07-21"
---

## Übersicht
Das Projekt `Frontend` befindet sich unter `src/Frontend`. Es handelt sich um eine Blazor WebAssembly Anwendung, die Sensordaten und Wetterinformationen aus dem BackendApi visualisiert und Interaktionen ermöglicht.

## Build- und Testanweisungen

- **Lokal ausführen:**
    1. Ins Verzeichnis `src/Frontend` wechseln.
    2. Folgende Befehle ausführen:
        ```
        dotnet restore
        dotnet build
        dotnet run
        ```
    3. Die Anwendung ist dann unter `http://localhost:5000` (oder im Terminal angegebenen Port) erreichbar.

- **Docker:**
    1. Im Verzeichnis `src/Frontend`:
        ```
        docker build -t frontend .
        docker run -p 5000:80 frontend
        ```
    2. Alternativ über das Root-Docker-Compose:
        ```
        docker-compose up frontend
        ```

- **Kubernetes/Helm:**
    - Deployment erfolgt über das Helm Chart im Verzeichnis `helm/weatherstation`. Die Konfiguration für Frontend liegt unter `helm/weatherstation/templates/frontend/`.

- **Tests:**
    - Unit-Tests können mit folgendem Befehl ausgeführt werden:
        ```
        dotnet test src/Frontend
        ```

## Zusammenspiel mit anderen Projekten

- Holt Sensordaten und Wetterinformationen über REST-Endpunkte vom BackendApi (`src/BackendApi`).
- Kann in Kubernetes/Helm über das Chart `helm/weatherstation` ausgerollt werden (Konfiguration unter `helm/weatherstation/templates/frontend/`).

## Troubleshooting/Setup-Hinweise

- Benötigte .NET-Version: .NET 8
- Bei Problemen mit Abhängigkeiten: `dotnet restore --force`
- Für UI-Änderungen Blazor und .NET Dokumentation beachten.

---

name: "ThingsApi"
description: "ASP.NET Core Web API für IoT-Daten. Stellt Endpunkte zur Erfassung und Bereitstellung von Sensordaten aus dem Things Network bereit. Entwickelt mit .NET 8."
category: "Backend"
tags: ["csharp", ".NET", "api", "iot", "thingsnetwork"]
lastUpdated: "2025-07-21"
---

## Übersicht
Das Projekt `ThingsApi` befindet sich unter `src/ThingsApi`. Es handelt sich um eine ASP.NET Core Web API, die Sensordaten aus dem Things Network verarbeitet und für andere Services (z.B. BackendApi) bereitstellt.

## Build- und Testanweisungen

- **Lokal ausführen:**
    1. Ins Verzeichnis `src/ThingsApi` wechseln.
    2. Folgende Befehle ausführen:
        ```
        dotnet restore
        dotnet build
        dotnet run
        ```
    3. Die API ist dann unter `http://localhost:5000` (oder im Terminal angegebenen Port) erreichbar.

- **Docker:**
    1. Im Verzeichnis `src/ThingsApi`:
        ```
        docker build -t thingsapi .
        docker run -p 5000:80 thingsapi
        ```
    2. Alternativ über das Root-Docker-Compose:
        ```
        docker-compose up thingsapi
        ```

- **Kubernetes/Helm:**
    - Deployment erfolgt über das Helm Chart im Verzeichnis `helm/weatherstation`. Die Konfiguration für ThingsApi liegt unter `helm/weatherstation/templates/backend/`.

- **Tests:**
    - Unit-Tests können mit folgendem Befehl ausgeführt werden:
        ```
        dotnet test src/ThingsApi
        ```
    - Die API-Dokumentation ist nach dem Starten unter `/swagger` verfügbar.

## Zusammenspiel mit anderen Projekten

- Stellt Sensordaten für das BackendApi (`src/BackendApi`) bereit.
- Nutzt die Application-Library (`src/Application`) für Geschäftslogik und Datenzugriff.
- Kann in Kubernetes/Helm über das Chart `helm/weatherstation` ausgerollt werden.

## Troubleshooting/Setup-Hinweise

- Benötigte .NET-Version: .NET 8
- Bei Problemen mit Abhängigkeiten: `dotnet restore --force`
- Für API-Änderungen immer die OpenAPI-Spezifikation (`swagger.json`) aktualisieren.
