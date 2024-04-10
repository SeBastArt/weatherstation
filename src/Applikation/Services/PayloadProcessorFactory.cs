using Applikation.Interfaces;
using Applikation.Services.PayloadProcessors;

namespace Applikation.Services;

public class PayloadProcessorFactory
{
    private readonly List<IPayloadProcessor> _processors =
    [
        new DraginoPayloadProcessor(),
        new DecentlabPayloadProcessor(),
        new LiardPayloadProcessor()
    ];

    public IPayloadProcessor GetProcessor(string deviceId)
    {
        return _processors.FirstOrDefault(p => p.CanProcess(deviceId)) ?? throw new InvalidOperationException("No processor found for the device.");
    }
}