FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
WORKDIR /app
EXPOSE 8080
EXPOSE 8081

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
WORKDIR .
COPY ["ThingsApi/ThingsApi.csproj", "app/ThingsApi/"]
COPY ["Application/Application.csproj", "app/Application/"]

RUN ls -la
RUN ls -la app
RUN ls -la app/ThingsApi
RUN ls -la app/Application

RUN dotnet restore "app/ThingsApi/ThingsApi.csproj"
COPY ["ThingsApi/", "app/ThingsApi/"]
COPY ["Application/", "app/Application/"]
RUN dotnet build "app/ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/build

FROM build AS publish
ARG BUILD_CONFIGURATION=Release
RUN dotnet publish "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "ThingsApi.dll"]
