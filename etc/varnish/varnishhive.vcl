vcl 4.0;

backend other_varnish_1 {
    .host = "xx.xx.xx.xx";
    .port = "80";
}

backend other_varnish_2 {
    .host = "xx.xx.xx.xx";
    .port = "80";
}

sub varnishhive_vcl_recv {
    if (req.http.user-agent ~ "varnish") {
       if (client.ip == "ip_other_varnish_1") {
              set req.backend_hint = other_varnish_1;
       }
       elif (client.ip == "ip_other_varnish_2") {
              set req.backend_hint = other_varnish_2;
       }
    }
}