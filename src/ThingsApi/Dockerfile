FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
WORKDIR /app
EXPOSE 8080
EXPOSE 8081

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
WORKDIR /src
COPY ["./", "./"]
# The following ls commands will list the contents of /src, where you just copied your source code.
RUN ls -la
# Now, specifically list the contents of the ThingsApi folder to ensure it was copied correctly.
RUN ls -la ThingsApi
RUN ls -la Application
RUN dotnet restore "ThingsApi/ThingsApi.csproj"

RUN dotnet build "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/build

FROM build AS publish
ARG BUILD_CONFIGURATION=Release
RUN dotnet publish "ThingsApi/ThingsApi.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "ThingsApi.dll"]
