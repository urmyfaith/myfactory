/*
 * SIP Registration Agent -- by ww@styx.org
 * 
 * This program is Free Software, released under the GNU General
 * Public License v2.0 http://www.gnu.org/licenses/gpl
 *
 * This program will register to a SIP proxy using the contact
 * supplied on the command line. This is useful if, for some 
 * reason your SIP client cannot register to the proxy itself.
 * For example, if your SIP client registers to Proxy A, but
 * you want to be able to recieve calls that arrive at Proxy B,
 * you can use this program to register the client's contact
 * information to Proxy B.
 *
 * This program requires the eXosip library. To compile,
 * assuming your eXosip installation is in /usr/local,
 * use something like:
 *
 * gcc -O2 -I/usr/local/include -L/usr/local/lib sipreg.c \
 *         -o sipreg \
 *         -leXosip2 -losip2 -losipparser2 -lpthread
 *
 * It should compile and run on any POSIX compliant system
 * that supports pthreads.
 *
 */


#if defined(__arc__)
#define LOG_PERROR 1
#include <includes_api.h>
#include <os_cfg_pub.h>
#endif

#if !defined(WIN32) && !defined(_WIN32_WCE)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>
#ifndef OSIP_MONOTHREAD
#include <pthread.h>
#endif
#include <string.h>
#ifdef __linux
#include <signal.h>
#endif
#endif

#ifdef _WIN32_WCE
/* #include <syslog.h> */
#include <winsock2.h>
#endif

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>

#if !defined(WIN32) && !defined(_WIN32_WCE) && !defined(__arc__)
#define _GNU_SOURCE
#include <getopt.h>
#endif

#define PROG_NAME "sipreg"
#define PROG_VER  "1.0"
#define UA_STRING "SipReg v" PROG_VER
#define SYSLOG_FACILITY LOG_DAEMON

static volatile int keepRunning = 1;

static void
intHandler (int dummy)
{
  keepRunning = 0;
  exit(-1);
}

#if defined(WIN32) || defined(_WIN32_WCE)
static void
syslog_wrapper (int a, const char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  vfprintf (stdout, fmt, args);
  va_end (args);
}

#define LOG_INFO 0
#define LOG_ERR 0
#define LOG_WARNING 0
#define LOG_DEBUG 0

#else
#define syslog_wrapper(a,b...) fprintf(stderr,b);fprintf(stderr,"\n")
#endif

typedef struct regparam_t {
  int regid;
  int expiry;
  int auth;
} regparam_t;

struct eXosip_t *context_eXosip;

int
main (int argc, char *argv[])
{
  int c;
  int port = 5055;
  char *contact = NULL;
  char *fromuser = NULL;
  char *proxy = NULL;
  char *route = NULL;

  const char *localip = NULL;
  const char *firewallip = NULL;

#if !defined(__arc__)
  struct servent *service;
#endif
  char *username = "11000000004000000005";
  char *password = "123456";

  struct regparam_t regparam = { 0, 3600, 0 };
  int debug = 0;
  int nofork = 0;
  int err;

  signal (SIGINT, intHandler);
  
//  sprintf(route,"<sip:%s:%d;lr>","192.168.66.244",35060);  

  proxy = osip_strdup ("sip:11000000004000000005@192.168.6.94");
  fromuser = osip_strdup ("sip:11000000004000000005@192.168.6.94");

  context_eXosip = eXosip_malloc ();
  if (eXosip_init (context_eXosip)) {
    syslog_wrapper (LOG_ERR, "eXosip_init failed");
    exit (1);
  }

  err = eXosip_listen_addr (context_eXosip, IPPROTO_UDP, NULL, port, AF_INET, 0);
  if (err) {
    syslog_wrapper (LOG_ERR, "eXosip_listen_addr failed");
    exit (1);
  }

  if (localip) {
    syslog_wrapper (LOG_INFO, "local address: %s", localip);
    eXosip_masquerade_contact (context_eXosip, localip, port);
  }

  if (firewallip) {
    syslog_wrapper (LOG_INFO, "firewall address: %s:%i", firewallip, port);
    eXosip_masquerade_contact (context_eXosip, firewallip, port);
  }

  eXosip_set_user_agent (context_eXosip, UA_STRING);

 if (username && password) {
    syslog_wrapper (LOG_INFO, "username: %s", username);
    syslog_wrapper (LOG_INFO, "password: [removed]");
    if (eXosip_add_authentication_info (context_eXosip, username, username, password, "MD5", NULL)) {
      syslog_wrapper (LOG_ERR, "eXosip_add_authentication_info failed");
      exit (1);
    }
  }

  {
    osip_message_t *reg = NULL;
    int i;

    eXosip_clear_authentication_info(context_eXosip); 
    
    regparam.regid = eXosip_register_build_initial_register (context_eXosip, fromuser, proxy, contact, regparam.expiry, &reg);
    if (regparam.regid < 1) {
      syslog_wrapper (LOG_ERR, "eXosip_register_build_initial_register failed");
      exit (1);
    }
    
//    route = osip_strdup ("<sip:192.168.6.141:5060;lr>");
//    osip_message_set_route(reg,route);

    i = eXosip_register_send_register (context_eXosip, regparam.regid, reg);
    if (i != 0) {
      syslog_wrapper (LOG_ERR, "eXosip_register_send_register failed");
      exit (1);
    }
  }

  for (; keepRunning;) {

    static int counter = 0;
    eXosip_event_t *event;

    counter++;
    if (counter % 60000 == 0) {
      struct eXosip_stats stats;

      memset (&stats, 0, sizeof (struct eXosip_stats));
      eXosip_lock (context_eXosip);
      eXosip_set_option (context_eXosip, EXOSIP_OPT_GET_STATISTICS, &stats);
      eXosip_unlock (context_eXosip);
      syslog_wrapper (LOG_INFO, "eXosip stats: inmemory=(tr:%i//reg:%i) average=(tr:%f//reg:%f)", stats.allocated_transactions, stats.allocated_registrations, stats.average_transactions, stats.average_registrations);
    }

    if (!(event = eXosip_event_wait (context_eXosip, 0, 1))) {
//#ifdef OSIP_MONOTHREAD
      eXosip_execute (context_eXosip);
//#endif
      eXosip_automatic_action (context_eXosip);
      osip_usleep (10000);
      continue;
    }
//#ifdef OSIP_MONOTHREAD
    eXosip_execute (context_eXosip);
//#endif

    eXosip_lock (context_eXosip);
    eXosip_automatic_action (context_eXosip);

    switch (event->type) {
        case EXOSIP_REGISTRATION_SUCCESS:
          syslog_wrapper (LOG_INFO, "registrered successfully");
          break;
        case EXOSIP_REGISTRATION_FAILURE:
          if ((event->response != NULL) && (event->response->status_code == 401)) 
          { // ??401??          
              osip_message_t *reg = NULL;  

              osip_www_authenticate_t *dest = NULL;  
              osip_message_get_www_authenticate(event->response,0,&dest);  
              printf("response = %s \n", event->response);
              if(dest == NULL)  
                continue;  

              char realm[256];  
              strcpy(realm,osip_www_authenticate_get_realm(dest));  
              printf("realm = %s \n", realm);

              eXosip_clear_authentication_info(context_eXosip);
              eXosip_add_authentication_info(context_eXosip, username, username, password, "MD5", realm);
              eXosip_register_build_register(context_eXosip, event->rid, regparam.expiry, &reg);
              eXosip_register_send_register(context_eXosip, event->rid, reg);
          }
          else {
            syslog_wrapper (LOG_INFO, "registrered fail");    
          }  

          break;
        case EXOSIP_CALL_INVITE:
          {
            osip_message_t *answer;
            int i;

            i = eXosip_call_build_answer (context_eXosip, event->tid, 405, &answer);
            if (i != 0) {
              syslog_wrapper (LOG_ERR, "failed to reject INVITE");
              break;
            }
            osip_free (answer->reason_phrase);
            answer->reason_phrase = osip_strdup ("No Support for Incoming Calls");
            i = eXosip_call_send_answer (context_eXosip, event->tid, 405, answer);
            if (i != 0) {
              syslog_wrapper (LOG_ERR, "failed to reject INVITE");
              break;
            }
            syslog_wrapper (LOG_INFO, "INVITE rejected with 405");
            break;
          }
        case EXOSIP_MESSAGE_NEW:
          {
            osip_message_t *answer;
            int i;

            i = eXosip_message_build_answer (context_eXosip, event->tid, 405, &answer);
            if (i != 0) {
              syslog_wrapper (LOG_ERR, "failed to reject %s", event->request->sip_method);
              break;
            }
            i = eXosip_message_send_answer (context_eXosip, event->tid, 405, answer);
            if (i != 0) {
              syslog_wrapper (LOG_ERR, "failed to reject %s", event->request->sip_method);
              break;
            }
            syslog_wrapper (LOG_INFO, "%s rejected with 405", event->request->sip_method);
            break;
          }
        case EXOSIP_IN_SUBSCRIPTION_NEW:
          {
            osip_message_t *answer;
            int i;

            i = eXosip_insubscription_build_answer (context_eXosip, event->tid, 405, &answer);
            if (i != 0) {
              syslog_wrapper (LOG_ERR, "failed to reject %s", event->request->sip_method);
              break;
            }
            i = eXosip_insubscription_send_answer (context_eXosip, event->tid, 405, answer);
            if (i != 0) {
              syslog_wrapper (LOG_ERR, "failed to reject %s", event->request->sip_method);
              break;
            }
            syslog_wrapper (LOG_INFO, "%s rejected with 405", event->request->sip_method);
            break;
          }
        default:
          syslog_wrapper (LOG_DEBUG, "recieved unknown eXosip event (type, did, cid) = (%d, %d, %d)", event->type, event->did, event->cid);
    }

    eXosip_unlock (context_eXosip);
    eXosip_event_free (event);
  }


  eXosip_quit (context_eXosip);
  osip_free (context_eXosip);
  return 0;
}
