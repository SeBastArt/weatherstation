FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
WORKDIR /app
EXPOSE 8080
EXPOSE 8081

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
WORKDIR .
COPY [".", "."]
RUN ls -R
RUN dotnet restore "ThingsApi/ThingsApi.csproj"

WORKDIR .
COPY [".", "."]
RUN ls
RUN dotnet build "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/build

FROM build AS publish
ARG BUILD_CONFIGURATION=Release
WORKDIR .
COPY [".", "."]
RUN ls
RUN dotnet publish "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "ThingsApi.dll"]
