namespace Application.Interfaces;

public interface IInfluxDbConfig
{
    string Host { get; }
    string Token { get; }
    string Bucket { get; }
    string Organization { get; }
}