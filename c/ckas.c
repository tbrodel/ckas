/*
 * Implemenation of "Generic UDP Interace" for CKAS Mechatronics W3s 6DoF motion
 * simulators. Spec provided by CKAS, contact vendor for details.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "m_pd.h"

#define ANGLE_MAX 0.17
#define ANGLE_MIN -0.17
#define CART_MAX 50.0
#define CART_MIN -50.0
#define HEAD "0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, "
#define LOCAL_IP "127.0.0.1"
#define NULL_TERM 1
#define PACKET_LEN 172
#define PORT 1254

static t_class *ckas_class;

typedef struct t_ckas {
        t_object x_obj;
        t_float x, y, z, pitch, roll, yaw;
        char *host; /* String to hold CKAS target IP */
		int sockfd; /* Socket file descriptor */
} t_ckas;

void
send_packet(t_ckas *x)
{
        struct sockaddr_in simcor;        
        int slen = sizeof(simcor);
        char packet[PACKET_LEN + NULL_TERM];

        memset(&simcor, 0, sizeof(simcor));
        simcor.sin_family = AF_INET;
        simcor.sin_port = htons(PORT);    
        if (!(inet_aton(x->host, &simcor.sin_addr))) {
                post("Invalid IP Address");
                return;
        }

        snprintf(packet, (PACKET_LEN + NULL_TERM) * sizeof(char), 
            "%s%010.6f, %010.6f, %010.6f, %09.6f, %09.6f, %09.6f", HEAD, x->x,
            x->y, x->z, x->pitch, x->roll, x->yaw);
		packet[PACKET_LEN] = '\0';

        if (sendto(x->sockfd, packet, PACKET_LEN + NULL_TERM, 0, 
            (struct sockaddr *)&simcor, slen) == -1) {
                perror("sendto() failed");
                post("sendto() Failed");
                return;
        }
}

void 
mk_sockfd(t_ckas *x)
{
        if ((x->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                perror("fail..");
                post("Failed to open socket");
		}
}

void
check_float(t_float *f, float max, float min)
{
        if (*f > max) {
                *f = max;
                return;
        }    

        if (*f < min)
                *f = min;
}

void
ckas_bang(t_ckas *x)
{
	/* Ensure values are in valid range */
    check_float(&x->x, CART_MAX, CART_MIN);
    check_float(&x->y, CART_MAX, CART_MIN);
    check_float(&x->z, CART_MAX, CART_MIN);
    check_float(&x->pitch, ANGLE_MAX, ANGLE_MIN);
    check_float(&x->roll, ANGLE_MAX, ANGLE_MIN);
    check_float(&x->yaw, ANGLE_MAX, ANGLE_MIN);

    send_packet(x);
}

void *
ckas_new(t_symbol *s, int argc, t_atom *argv)
{
        t_ckas *x = (t_ckas *)pd_new(ckas_class);
        t_symbol *sym;

        floatinlet_new(&x->x_obj, &x->x);
        floatinlet_new(&x->x_obj, &x->y);
        floatinlet_new(&x->x_obj, &x->z);
        floatinlet_new(&x->x_obj, &x->pitch);
        floatinlet_new(&x->x_obj, &x->roll);
        floatinlet_new(&x->x_obj, &x->yaw);
        
		/* 
         * If object has an argument make it the IP address, otherwise default
		 * to localhost.
		 */
        switch(argc) {
        case 1:
                sym = atom_getsymbolarg(0, argc, argv);
                x->host = sym->s_name;
                break;
        case 0:
        default:
                x->host = LOCAL_IP;
        }

		/* Make socket descriptor once, changing sockets confuses Simcor */
		mk_sockfd(x);

        return (void *)x;
}

void 
ckas_free(t_ckas *x)
{
		/* Close socket when object is destroyed */
		close(x->sockfd);
}

void
ckas_setup(void)
{
        ckas_class = class_new(gensym("ckas"), (t_newmethod)ckas_new, 
			(t_method)ckas_free, sizeof(t_ckas), CLASS_DEFAULT, A_GIMME, 0); 

        class_addbang(ckas_class, ckas_bang);
}
