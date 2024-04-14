FROM mcr.microsoft.com/dotnet/aspnet:8.0-alpine AS base
USER $APP_UID
WORKDIR /app
EXPOSE 80
EXPOSE 443

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0-alpine AS build
ARG TARGETARCH
ARG BUILDPLATFORM

WORKDIR /src
COPY ["BackendApi/BackendApi.csproj", "BackendApi/"]
COPY ["Application/Application.csproj", "Application/"]
RUN ls -la
RUN dotnet restore "BackendApi/BackendApi.csproj"
RUN dotnet build "BackendApi/BackendApi.csproj" -c $BUILD_CONFIGURATION -o /app/build -a $TARGETARCH

FROM build AS publish
ARG BUILD_CONFIGURATION=Release
RUN dotnet publish "BackendApi/BackendApi.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false  -a $TARGETARCH

FROM --platform=$BUILDPLATFORM base AS final
ARG TARGETARCH
ARG BUILDPLATFORM
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "BackendApi.dll"]
