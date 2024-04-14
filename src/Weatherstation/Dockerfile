FROM mcr.microsoft.com/dotnet/aspnet:8.0-alpine AS base
USER $APP_UID
WORKDIR /app
EXPOSE 80
EXPOSE 443

FROM --platform=$BUILDPLATFORM mcr.microsoft.com/dotnet/sdk:8.0-alpine AS build
ARG TARGETARCH
ARG BUILDPLATFORM

WORKDIR /src
COPY ["Weatherstation", "Weatherstation/"]

RUN dotnet restore "Weatherstation/Weatherstation.csproj"
RUN dotnet build "Weatherstation/Weatherstation.csproj" -c Release -o /app/build -a $TARGETARCH

FROM build AS publish
RUN dotnet publish "Weatherstation/Weatherstation.csproj" -c Release -o /app/publish /p:UseAppHost=true -a $TARGETARCH

FROM --platform=$BUILDPLATFORM base AS final
ARG TARGETARCH
ARG BUILDPLATFORM
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "Weatherstation.dll"]
