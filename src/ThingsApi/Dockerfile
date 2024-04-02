FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
WORKDIR /app
EXPOSE 8080
EXPOSE 8081

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
# Set the WORKDIR to /src to match the relative paths in your project references.
WORKDIR /src
# Copy the csproj files specifically first to optimize layer caching and restore.
COPY ["ThingsApi/ThingsApi.csproj", "ThingsApi/"]
COPY ["Application/Application.csproj", "Application/"]
# Restore dependencies based on the csproj files.
RUN dotnet restore "ThingsApi/ThingsApi.csproj"

# Now copy the rest of the source files.
COPY ["ThingsApi/", "ThingsApi/"]
COPY ["Application/", "Application/"]
# Build the project.
RUN dotnet build "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/build

FROM build AS publish
ARG BUILD_CONFIGURATION=Release
# The working directory is already /src, so this path should directly reference the project.
RUN dotnet publish "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "ThingsApi.dll"]
