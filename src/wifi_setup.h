#pragma once

#include <IPAddress.h>

bool ConfigureStaticIp(IPAddress& ip_out, IPAddress& gateway_out,
                       IPAddress& subnet_out, IPAddress& dns_out);
void SetupWifi();
void ResetWifiSettings();
