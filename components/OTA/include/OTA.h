/*
 * Ota.h
 *
 *  Created on: 09.11.2015
 *      Author: Jean-Martin George
 */


#ifndef OTA_H_
#define OTA_H_


#include "Main_Types.h"
#include "Main_Config.h"
#include "uC.h"
#include "iap_https.h"


#define WIFI_NETWORK_SSID         "Leo-Wohnung"
#define WIFI_NETWORK_PASSWORD     "ObereBurghalde33"

#define OTA_SERVER_HOST_NAME      "ds216j-jmg.tplinkdns.com"
#define OTA_SERVER_METADATA_PATH  "/esp32/ota_cfg.txt"
#define OTA_POLLING_INTERVAL_S    1
#define OTA_AUTO_REBOOT           0


/* chain.pem */
#define OTA_PEER_PEM \
		"-----BEGIN CERTIFICATE-----\n" \
		"MIIFEzCCA/ugAwIBAgISA4D1dNbjATMAkB9L2pN3qBvPMA0GCSqGSIb3DQEBCwUA\n" \
		"MEoxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MSMwIQYDVQQD\n" \
		"ExpMZXQncyBFbmNyeXB0IEF1dGhvcml0eSBYMzAeFw0xNzA5MjYxNTU2MDBaFw0x\n" \
		"NzEyMjUxNTU2MDBaMCMxITAfBgNVBAMTGGRzMjE2ai1qbWcudHBsaW5rZG5zLmNv\n" \
		"bTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALuze8gcfM+UArifnQWR\n" \
		"FD5sXixqWTa/YuGMHQnlRYFwmu+Qp6zvuCHFPVpUSYYmeR4p3kMqjTNmUOoPUwfH\n" \
		"u3MvKC1ZsruEOu3AxtaFhD7uB00KmehqeY8TC6Qvz37g1EeDSINjvECxBHl5PryT\n" \
		"xYrX7l9wmDmVR821jGSyO037WMh67s/glEDg/JiCUz0YBSltlq0feV5iDUQ42KNV\n" \
		"Q6ot4zDbTsJrOekudCJbRI8KaPQ7RZC5Z827QNsx8fO/KjZelnH4POPPKuPByKw/\n" \
		"qtKV4wOIPF4SPDwR+RsCv5GR6MGItH86M0BY6iFzIU3x7+ouYub/t+nsvwNub52Y\n" \
		"SiMCAwEAAaOCAhgwggIUMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAUBggrBgEF\n" \
		"BQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUmC8TEMjsLHBC\n" \
		"aIVz0e+oz0jgm8AwHwYDVR0jBBgwFoAUqEpqYwR93brm0Tm3pkVl7/Oo7KEwbwYI\n" \
		"KwYBBQUHAQEEYzBhMC4GCCsGAQUFBzABhiJodHRwOi8vb2NzcC5pbnQteDMubGV0\n" \
		"c2VuY3J5cHQub3JnMC8GCCsGAQUFBzAChiNodHRwOi8vY2VydC5pbnQteDMubGV0\n" \
		"c2VuY3J5cHQub3JnLzAjBgNVHREEHDAaghhkczIxNmotam1nLnRwbGlua2Rucy5j\n" \
		"b20wgf4GA1UdIASB9jCB8zAIBgZngQwBAgEwgeYGCysGAQQBgt8TAQEBMIHWMCYG\n" \
		"CCsGAQUFBwIBFhpodHRwOi8vY3BzLmxldHNlbmNyeXB0Lm9yZzCBqwYIKwYBBQUH\n" \
		"AgIwgZ4MgZtUaGlzIENlcnRpZmljYXRlIG1heSBvbmx5IGJlIHJlbGllZCB1cG9u\n" \
		"IGJ5IFJlbHlpbmcgUGFydGllcyBhbmQgb25seSBpbiBhY2NvcmRhbmNlIHdpdGgg\n" \
		"dGhlIENlcnRpZmljYXRlIFBvbGljeSBmb3VuZCBhdCBodHRwczovL2xldHNlbmNy\n" \
		"eXB0Lm9yZy9yZXBvc2l0b3J5LzANBgkqhkiG9w0BAQsFAAOCAQEAA53ZcmL2rq6a\n" \
		"4C/uBjWjsbwDrBFek9RgszHOpQGDeBvclooHCiwcbHTVbxgwwaKThgZzz2vX5S85\n" \
		"WKiGlI3dhT64+/SuvbTtKtn7Fpctoa+9iVdc9Gw2wqAhCM9Cqk7Rz9ThNLeY7miP\n" \
		"W7OvdLaBi3T8C5AqEmCw+DofAAVaEBnZd+T7OobT3dEIMV3g8jAMgl4bJxBktd5s\n" \
		"RTT8fm0s7+3Nz6HzlOzvC6qD7drn3xGg/wCwtAdKskiAotXB7nn1brGQT7/oAJtQ\n" \
		"1KxRDvX59yoGuxggMgvKT8YPAmtUiAizXgx73kNJEMnlhiRsL7OpuNVfE6ZNn/Th\n" \
		"3SqFzapSOQ==\n" \
		"-----END CERTIFICATE-----\n"

/* cert.pem */
#define OTA_SERVER_ROOT_CA_PEM \
		"-----BEGIN CERTIFICATE-----\n" \
		"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
		"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
		"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
		"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
		"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
		"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
		"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
		"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
		"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
		"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
		"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
		"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
		"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
		"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
		"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
		"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
		"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
		"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
		"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
		"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
		"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
		"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
		"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
		"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
		"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
		"-----END CERTIFICATE-----\n"


typedef enum
{
	OTA_STATE_IDLE = 0,
	OTA_STATE_DOWNLOAD_IN_PROGRESS,
	OTA_STATE_UPDATE_IN_PROGRESS,
	OTA_STATE_UPDADE_FINISHED
} OTA_State_t;


void OTA__init (void);

void OTA__enable (void);
void OTA__disable (void);

OTA_State_t OTA__getCurrentState (void);

uint64_t OTA__getCurrentSwVersion (void);
void OTA__setCurrentSwVersion (uint64_t newSwVersion);

void OTA__runBeforeSwUpdate (void);
void OTA__runAfterSwUpdate (void);

void OTA__triggerSwUpdate (void);
uint8_t OTA__isSwUpdateTriggered (void);


#endif /* OTA_H_ */
