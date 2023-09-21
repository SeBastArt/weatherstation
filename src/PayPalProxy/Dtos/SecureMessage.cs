namespace PayPalProxy.Dtos
{
    public class SecureMessage
    {
        public string Payload { get; set; }
        public string Hmac { get; set; }
        public long Timestamp { get; set; }
    }
}
