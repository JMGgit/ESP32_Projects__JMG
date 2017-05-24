/*
 * Ota.h
 *
 *  Created on: 09.11.2015
 *      Author: Jean-Martin George
 */


#ifndef OTA_H_
#define OTA_H_

#include "iap_https.h"


#define OTA_SOFTWARE_VERSION      9

#define WIFI_NETWORK_SSID         "Leo-Wohnung"
#define WIFI_NETWORK_PASSWORD     "ObereBurghalde33"

#define OTA_SERVER_HOST_NAME      "jmg-dev"
#define OTA_SERVER_METADATA_PATH  "/esp32/ota.txt"
#define OTA_POLLING_INTERVAL_S    5
#define OTA_AUTO_REBOOT           1


#define OTA_SERVER_ROOT_CA_PEM \
		"-----BEGIN CERTIFICATE-----\n" \
		"MIIDkDCCAngCCQCr/eNpvZ3kLDANBgkqhkiG9w0BAQsFADCBiTELMAkGA1UEBhMC\n" \
		"REUxGzAZBgNVBAgMEkJhZGVuLVfCgXJ0dGVtYmVyZzERMA8GA1UEBwwITGVvbmJl\n" \
		"cmcxDDAKBgNVBAoMA0pNRzEQMA4GA1UEAwwHam1nLWRldjEqMCgGCSqGSIb3DQEJ\n" \
		"ARYbamVhbm1hcnRpbi5nZW9yZ2VAZ21haWwuY29tMB4XDTE3MDUyMTE2Mzc0MloX\n" \
		"DTE4MDUyMTE2Mzc0MlowgYkxCzAJBgNVBAYTAkRFMRswGQYDVQQIDBJCYWRlbi1X\n" \
		"woFydHRlbWJlcmcxETAPBgNVBAcMCExlb25iZXJnMQwwCgYDVQQKDANKTUcxEDAO\n" \
		"BgNVBAMMB2ptZy1kZXYxKjAoBgkqhkiG9w0BCQEWG2plYW5tYXJ0aW4uZ2Vvcmdl\n" \
		"QGdtYWlsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOj4WhYR\n" \
		"DvgPHbh6sG6isaFlBPrV6ppzdllsy8yGO3s/LbeSnvBBBxl5ckF+iYrzdWBLyGxS\n" \
		"tnLiAqlp4CuGTR/Z3UiKcBHVm1M5ix/x2AhXcBFk76YrbsNwboKfF3T481MLPka/\n" \
		"0WTiS9urH/dQ6uZqbW6CStxko2IWpau2n0J2tGkNCHIVgGCz9opJ8f3/HxD9doXD\n" \
		"4b/kJKCeLwA/TlOS7qjSNIaOrPgJp8KeiTJXDebZewRa/sE+6k1lWa8oh3edeT4t\n" \
		"uwYx/6wkjC+YgG1MYyHGYPtEePQkA48EsLMmmr7C65jbqm9bRC1DlfNvdROT7R1t\n" \
		"maWTHVolytM3SDMCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAaVpYebM5vgr04y59\n" \
		"gKDQeu2ibl0FkWcRO2bbYDtvdp3FwyUsKmtglGbAhzqMNAlfiN1cFOwBeA2HNO11\n" \
		"il7q/M3nnKUSgybgD7fGvBJwSqgTuq4q4HNVodSEXmOTsyuvCWeJUmpC5lbIpJCH\n" \
		"EsQ4/9/UabQZkppTATRrcOn/XfP2K5I4J/yQwLLzLxX+/LikqjjZ/0CFtET2gKqg\n" \
		"gD0YBw0bAkrsd2HJnyK3uh7JSI6Z8Y9Hqv/Ae8P36qag9XGUD6/offw1Rwvj+9TN\n" \
		"bnpARNQkqsE3tGDNdDp7woFjJ5u3+0+cPQ6zZeJBsVLvsbHYOgMzPflCWdJW7kDo\n" \
		"tX6AUA==\n" \
		"-----END CERTIFICATE-----\n"

#define OTA_PEER_PEM \
		"-----BEGIN CERTIFICATE-----\n" \
		"MIIDkDCCAngCCQCr/eNpvZ3kLDANBgkqhkiG9w0BAQsFADCBiTELMAkGA1UEBhMC\n" \
		"REUxGzAZBgNVBAgMEkJhZGVuLVfCgXJ0dGVtYmVyZzERMA8GA1UEBwwITGVvbmJl\n" \
		"cmcxDDAKBgNVBAoMA0pNRzEQMA4GA1UEAwwHam1nLWRldjEqMCgGCSqGSIb3DQEJ\n" \
		"ARYbamVhbm1hcnRpbi5nZW9yZ2VAZ21haWwuY29tMB4XDTE3MDUyMTE2Mzc0MloX\n" \
		"DTE4MDUyMTE2Mzc0MlowgYkxCzAJBgNVBAYTAkRFMRswGQYDVQQIDBJCYWRlbi1X\n" \
		"woFydHRlbWJlcmcxETAPBgNVBAcMCExlb25iZXJnMQwwCgYDVQQKDANKTUcxEDAO\n" \
		"BgNVBAMMB2ptZy1kZXYxKjAoBgkqhkiG9w0BCQEWG2plYW5tYXJ0aW4uZ2Vvcmdl\n" \
		"QGdtYWlsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOj4WhYR\n" \
		"DvgPHbh6sG6isaFlBPrV6ppzdllsy8yGO3s/LbeSnvBBBxl5ckF+iYrzdWBLyGxS\n" \
		"tnLiAqlp4CuGTR/Z3UiKcBHVm1M5ix/x2AhXcBFk76YrbsNwboKfF3T481MLPka/\n" \
		"0WTiS9urH/dQ6uZqbW6CStxko2IWpau2n0J2tGkNCHIVgGCz9opJ8f3/HxD9doXD\n" \
		"4b/kJKCeLwA/TlOS7qjSNIaOrPgJp8KeiTJXDebZewRa/sE+6k1lWa8oh3edeT4t\n" \
		"uwYx/6wkjC+YgG1MYyHGYPtEePQkA48EsLMmmr7C65jbqm9bRC1DlfNvdROT7R1t\n" \
		"maWTHVolytM3SDMCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAaVpYebM5vgr04y59\n" \
		"gKDQeu2ibl0FkWcRO2bbYDtvdp3FwyUsKmtglGbAhzqMNAlfiN1cFOwBeA2HNO11\n" \
		"il7q/M3nnKUSgybgD7fGvBJwSqgTuq4q4HNVodSEXmOTsyuvCWeJUmpC5lbIpJCH\n" \
		"EsQ4/9/UabQZkppTATRrcOn/XfP2K5I4J/yQwLLzLxX+/LikqjjZ/0CFtET2gKqg\n" \
		"gD0YBw0bAkrsd2HJnyK3uh7JSI6Z8Y9Hqv/Ae8P36qag9XGUD6/offw1Rwvj+9TN\n" \
		"bnpARNQkqsE3tGDNdDp7woFjJ5u3+0+cPQ6zZeJBsVLvsbHYOgMzPflCWdJW7kDo\n" \
		"tX6AUA==\n" \
		"-----END CERTIFICATE-----\n"


void OTA__init (void);

#endif /* OTA_H_ */
