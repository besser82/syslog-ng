#include "hostname.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <windns.h>
#include <windows.h>

static gchar local_hostname_fqdn[256];
static gchar local_hostname_short[256];

char *
get_ip_for_dns_request()
{
  struct in_addr addr;
  char accumlator[NI_MAXHOST] = {0};
  struct hostent *local_host = NULL;
  gethostname(accumlator,NI_MAXHOST);
  local_host = gethostbyname(accumlator);
  addr.s_addr = *(u_long *)local_host->h_addr;
  sprintf(accumlator,"%d.%d.%d.%d.IN-ADDR.ARPA",addr.S_un.S_un_b.s_b4,addr.S_un.S_un_b.s_b3,addr.S_un.S_un_b.s_b2,addr.S_un.S_un_b.s_b1);
  return strdup(accumlator);
}

int
has_dns_server()
{
  DWORD status = 0;
  DWORD length = 0;
  FIXED_INFO *buf = NULL;
  int result = 0;
  if (GetNetworkParams(NULL,&length) == ERROR_BUFFER_OVERFLOW)
     buf = malloc(length);
  if (buf && GetNetworkParams(buf,&length) == NO_ERROR)
    {
      result = strlen(buf->DnsServerList.IpAddress.String) ? 1 : 0;
    }
  free(buf);
}

char *
get_dnsname()
{
  DWORD status = 0;
  char *result = NULL;
  PDNS_RECORD dns_record;
  if (has_dns_server())
    {
      char *ip_for_dns_request = get_ip_for_dns_request();
      status = DnsQuery(
                        ip_for_dns_request,
                        DNS_TYPE_PTR,
	                DNS_QUERY_BYPASS_CACHE | DNS_QUERY_NO_LOCAL_NAME,
                        NULL,
                        &dns_record,
                        NULL);
      free(ip_for_dns_request);
      if (status == 0)
        {
          result = strdup(dns_record->Data.PTR.pNameHost);
          DnsRecordListFree(dns_record,DnsFreeRecordListDeep);
        }
    }
  return result;
}

void reset_cached_hostname(void)
{
  gchar *s;

  gethostname(local_hostname_fqdn, sizeof(local_hostname_fqdn) - 1);
  local_hostname_fqdn[sizeof(local_hostname_fqdn) - 1] = '\0';
  if (strchr(local_hostname_fqdn, '.') == NULL)
    {
      /* not fully qualified, resolve it using DNS */
      char *result = get_dnsname();
      if (result)
        {
          strncpy(local_hostname_fqdn, result, sizeof(local_hostname_fqdn) - 1);
          local_hostname_fqdn[sizeof(local_hostname_fqdn) - 1] = '\0';
          free(result);
        }
    }
  /* NOTE: they are the same size, they'll fit */
  strcpy(local_hostname_short, local_hostname_fqdn);
  s = strchr(local_hostname_short, '.');
  if (s != NULL)
    *s = '\0';
}

void
getlonghostname(gchar *buf, gsize buflen)
{
  if (!local_hostname_fqdn[0])
    reset_cached_hostname();
  strncpy(buf, local_hostname_fqdn, buflen);
  buf[buflen - 1] = 0;
}


const char *
get_cached_longhostname()
{
  return local_hostname_fqdn;
}

const char *
get_cached_shorthostname()
{
  return local_hostname_short;
}