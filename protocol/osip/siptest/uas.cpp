/** 
 * 一个使用了 osip 和 eXosip 库的 UAS 代理服务端的演示程序 
 *  
 * - 只是简单的演示了使用了 osip 和 eXosip2 库的 UAS 代理服务端的如下几个功能： 
 *  
 * 编 译：g++ -I/usr/local/include -L/usr/local/lib ua_server.cpp -o ua_server -leXosip2 -losip2 -losipparser2 -lpthread 
 *  
 */  
  
#include <eXosip2/eXosip.h>  
  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
  
#include <iostream>  
#include <fstream>  
#include <string>  
#include <string.h>

using namespace std;  
  
int main()  
{  
    eXosip_event_t *je = NULL;  
    osip_message_t *ack = NULL;  
    osip_message_t *invite = NULL;  
    osip_message_t *answer = NULL;  
    sdp_message_t *remote_sdp = NULL;  
    int call_id, dialog_id;  
    int i,j;  
    int id;  
  
    char *sour_call = "sip:136@192.168.1.253";  
    char *dest_call = "sip:136@192.168.1.253:5061"; //client ip/port  
  
    char command;  
    char tmp[4096];  
    char localip[128];  
    int pos = 0;  
  
    // 初始化 sip  
  	eXosip_t *inst = eXosip_malloc();
    i = eXosip_init (inst);  
    if (i != 0)  
    {  
        cerr << "\n\t--> Can't initialize eXosip!\n";  
        return -1;  
    }  
    else  
    {  
        cout << "\n\t--> eXosip_init successfully!\n";  
    }  
      
    i = eXosip_listen_addr (inst, IPPROTO_UDP, NULL, 5060, AF_INET, 0);  
    if (i != 0)  
    {  
        eXosip_quit (inst);  
        cerr << "\n\t--> eXosip_listen_addr error! Couldn't initialize transport layer!\n";  
    }  
  
    for(;;)  
    {  
        // 侦听是否有消息到来  
        je = eXosip_event_wait (inst, 0, 50);   
  
        // 协议栈带有此语句,具体作用未知  
        eXosip_lock (inst);  
        eXosip_default_action (inst, je);  
//        eXosip_automatic_refresh (inst);  
        eXosip_unlock (inst);  
  
        if (je == NULL) // 没有接收到消息，继续  
        {  
            continue;  
        }  
  
        switch (je->type)  
        {  
        case EXOSIP_MESSAGE_NEW: // 新的消息到来  
            cout << "\n\t*** EXOSIP_MESSAGE_NEW!\n" << endl;  
  
            if (MSG_IS_MESSAGE (je->request)) // 如果接收到的消息类型是 MESSAGE  
            {  
                {  
                    osip_body_t *body;  
                    osip_message_get_body (je->request, 0, &body);   
                    cout << "I get the msg is: " << body->body << endl;  
                }  
  
                // 按照规则，需要回复 OK 信息  
                eXosip_message_build_answer (inst, je->tid, 200, &answer);  
                eXosip_message_send_answer (inst, je->tid, 200, answer);  
            }  
            break;  
  
        case EXOSIP_CALL_INVITE: // INVITE 请求消息  
            // 得到接收到消息的具体信息  
            cout << "\n\tReceived a INVITE msg from " << je->request->req_uri->host   
                 << " : " << je->request->req_uri->port   
                 << ", username is " << je->request->req_uri->username << endl;  
  
            // 得到消息体,认为该消息就是 SDP 格式.  
            remote_sdp = eXosip_get_remote_sdp (inst, je->did);  
            call_id = je->cid;  
            dialog_id = je->did;  
          
            eXosip_lock (inst);  
  
            eXosip_call_send_answer (inst, je->tid, 180, NULL);  
            i = eXosip_call_build_answer (inst, je->tid, 200, &answer);  
            if (i != 0)  
            {  
                cout << "\n\t--> This request msg is invalid! Cann't response!\n" << endl;  
                eXosip_call_send_answer (inst, je->tid, 400, NULL);  
            }  
            else  
            {  
                snprintf (tmp, 4096,  
                    "v=0\r\n"  
                    "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"  
                    "t=1 10\r\n"  
                    "a=username:rainfish\r\n"  
                    "a=password:123\r\n");  
              
                // 设置回复的SDP消息体,下一步计划分析消息体  
                // 没有分析消息体，直接回复原来的消息，这一块做的不好。  
                osip_message_set_body (answer, tmp, strlen(tmp));  
                osip_message_set_content_type (answer, "application/sdp");  
              
                eXosip_call_send_answer (inst, je->tid, 200, answer);  
                cout << "\n\t--> send 200 over!" << endl;  
            }  
  
            eXosip_unlock (inst);  
          
            // 显示出在 sdp 消息体中的 attribute 的内容,里面计划存放我们的信息  
            cout << "\n\t--> The INFO is :\n" ;  
            while (!osip_list_eol ( &(remote_sdp->a_attributes), pos))  
            {  
                sdp_attribute_t *at;  
              
                //这里解释了为什么在SDP消息体中属性a里面存放必须是两列  
                at = (sdp_attribute_t *) osip_list_get ( &remote_sdp->a_attributes, pos);  
                cout << "\n\t" << at->a_att_field   
                     << " : " << at->a_att_value << endl;  
              
                pos ++;  
            }  
            break;  
  
        case EXOSIP_CALL_ACK:  
            cout << "\n\t--> ACK recieved!\n" << endl;  
            // printf ("the cid is %s, did is %s\n", je->did, je->cid);   
            break;  
  
        case EXOSIP_CALL_CLOSED:  
            cout << "\n\t--> the remote hold the session!\n" << endl;  
            // eXosip_call_build_ack(inst, dialog_id, &ack);  
            // eXosip_call_send_ack(inst, dialog_id, ack);   
            i = eXosip_call_build_answer (inst, je->tid, 200, &answer);  
            if (i != 0)  
            {  
                printf ("This request msg is invalid!Cann't response!\n");  
                eXosip_call_send_answer (inst, je->tid, 400, NULL);  
            }  
            else  
            {  
                eXosip_call_send_answer (inst, je->tid, 200, answer);  
                cout << "\n\t--> bye send 200 over!\n";  
            }   
            break;  
  
        case EXOSIP_CALL_MESSAGE_NEW:  
  
            cout << "\n\t*** EXOSIP_CALL_MESSAGE_NEW\n" << endl;  
            if (MSG_IS_INFO(je->request) ) // 如果传输的是 INFO 方法  
            {  
                eXosip_lock (inst);  
                i = eXosip_call_build_answer (inst, je->tid, 200, &answer);  
                if (i == 0)  
                {  
                    eXosip_call_send_answer (inst, je->tid, 200, answer);  
                }  
  
                eXosip_unlock (inst);  
  
                {  
                    osip_body_t *body;  
                    osip_message_get_body (je->request, 0, &body);  
                    cout << "the body is " << body->body << endl;  
                }  
            }  
            break;   
  
        default:  
            cout << "\n\t--> Could not parse the msg!\n" << endl;  
        }  
    }   
  
    return 0;  
}  
