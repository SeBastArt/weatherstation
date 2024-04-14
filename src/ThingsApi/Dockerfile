FROM mcr.microsoft.com/dotnet/aspnet:8.0-alpine AS base
USER $APP_UID
WORKDIR /app
EXPOSE 80
EXPOSE 443

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0-alpine AS build
ARG TARGETARCH
ARG BUILDPLATFORM

WORKDIR /src
COPY ["ThingsApi", "ThingsApi/"]
COPY ["Application", "Application/"]
RUN dotnet restore "ThingsApi/ThingsApi.csproj"
RUN dotnet build "ThingsApi/ThingsApi.csproj" -c Release -o /app/build -a $TARGETARCH

FROM build AS publish
RUN dotnet publish "ThingsApi/ThingsApi.csproj" -c Release -o /app/publish /p:UseAppHost=false -a $TARGETARCH

FROM --platform=$BUILDPLATFORM base AS final
ARG TARGETARCH
ARG BUILDPLATFORM
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "ThingsApi.dll"]