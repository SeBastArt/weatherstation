namespace HoneyPal.Dtos
{
    using System;
    using System.Collections.Generic;

    public class Amount
    {
        public string total { get; set; }
        public string currency { get; set; }
    }

    public class Link
    {
        public string href { get; set; }
        public string rel { get; set; }
        public string method { get; set; }
    }

    public class Resource
    {
        public string id { get; set; }
        public DateTime create_time { get; set; }
        public DateTime update_time { get; set; }
        public Amount amount { get; set; }
        public string payment_mode { get; set; }
        public string state { get; set; }
        public string protection_eligibility { get; set; }
        public string protection_eligibility_type { get; set; }
        public DateTime clearing_time { get; set; }
        public string parent_payment { get; set; }
        public List<Link> links { get; set; }
    }

    public class Root
    {
        public string id { get; set; }
        public string event_version { get; set; }
        public DateTime create_time { get; set; }
        public string resource_type { get; set; }
        public string event_type { get; set; }
        public string summary { get; set; }
        public Resource resource { get; set; }
        public List<Link> links { get; set; }
    }
}
