FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
USER $APP_UID
WORKDIR /app
EXPOSE 8080
EXPOSE 8081

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
WORKDIR .
COPY [".", "app/"]
RUN ls
RUN ls app
RUN dotnet restore "app/ThingsApi/ThingsApi.csproj"
RUN dotnet build "app/ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/build

FROM build AS publish
WORKDIR /app
ARG BUILD_CONFIGURATION=Release
RUN dotnet publish "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "ThingsApi.dll"]
