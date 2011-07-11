/*
    Audio Scout - audio content indexing software
    Copyright (C) 2010  D. Grant Starkweather & Evan Klinger
    
    Audio Scout is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    D. Grant Starkweather - dstarkweather@phash.org
    Evan Klinger          - eklinger@phash.org
*/

#include <string.h>
#include <stdint.h>
#include <zmq.h>

#ifndef _HELPERH_H
#define _HELPHER_H


void free_fn(void *data, void *hint){
    free(data);
}


void* socket_bind(void *ctx, int skt_type, char *address){
    void *skt = zmq_socket(ctx, skt_type);
    if (skt == NULL){
	return NULL;
    }
    int res = zmq_bind(skt, address);
    if (res){
	zmq_close(skt);
	return NULL;
    }
    return skt;
}
void* socket_connect(void *ctx, int skt_type, char *address){
    void *skt = zmq_socket(ctx, skt_type);
    if (skt == NULL){
	return NULL;
    }
    int res = zmq_connect(skt, address);
    if (res){
	zmq_close(skt);
	return NULL;
    }
    return skt;
}

int recvsnd(void *rcvskt, void *sndskt, size_t *msg_size, int64_t *more, size_t *more_size,\
                        void **data, const char *topic){
    if (!rcvskt || !sndskt || !msg_size || !more || !more_size)
	return -1;

    zmq_msg_t topic_msg;
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recv(rcvskt, &msg, 0);
    *msg_size = zmq_msg_size(&msg);
    if (data){
	*data = malloc(*msg_size);
	memcpy(*data, zmq_msg_data(&msg), *msg_size);
    }
    if (topic){
	zmq_msg_init_size(&topic_msg, strlen(topic));
        memcpy(zmq_msg_data(&topic_msg), topic, strlen(topic));
	zmq_send(sndskt, &topic_msg, ZMQ_SNDMORE);
	zmq_msg_close(&topic_msg);
    }
    int res = zmq_send(sndskt, &msg, (more) ? ZMQ_SNDMORE : 0);
    zmq_getsockopt(rcvskt, ZMQ_RCVMORE, more, more_size);
    zmq_msg_close(&msg);

    return res;
}

int recieve_msg(void *skt, size_t *msg_size, int64_t *more, size_t *more_size,\
		void **data){
    if (skt == NULL) return -1;

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int res = zmq_recv(skt, &msg, 0);
    if (msg_size){
	*msg_size = zmq_msg_size(&msg);
    }
    if (data) {
	*data = malloc(zmq_msg_size(&msg));
	memcpy(*data, zmq_msg_data(&msg), zmq_msg_size(&msg));
    }
    zmq_getsockopt(skt, ZMQ_RCVMORE, more, more_size);
    zmq_msg_close(&msg);
    return res;
}

int send_empty_msg(void *skt){
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, 0);
    int res = zmq_send(skt, &msg, 0);
    zmq_msg_close(&msg);
    return res;
}

int send_msg_vsm(void *skt, void *data, size_t size){
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, size);
    memcpy(zmq_msg_data(&msg), data, size);
    int res = zmq_send(skt, &msg, 0);
    zmq_msg_close(&msg);
    return res;
}

int sendmore_msg_vsm(void *skt, void *data, size_t size){
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, size);
    memcpy(zmq_msg_data(&msg), data, size);
    int res = zmq_send(skt, &msg, ZMQ_SNDMORE);
    zmq_msg_close(&msg);
    return res;
}

int send_msg_data(void *skt,void *data,size_t size,void *freefnc,void *hint){
    zmq_msg_t msg;
    zmq_msg_init_data(&msg, data, size, freefnc, hint);
    int res = zmq_send(skt, &msg, 0);
    zmq_msg_close(&msg);
    return res;
}

int sendmore_msg_data(void *skt,void *data,size_t size,void *freefnc,void *hint){
    zmq_msg_t msg;
    zmq_msg_init_data(&msg, data, size, freefnc, hint);
    int res = zmq_send(skt, &msg, ZMQ_SNDMORE);
    zmq_msg_close(&msg);
    return res;
}

int flushall_msg_parts(void *skt){
    zmq_msg_t msg;
    int64_t more = 0;
    size_t more_size = sizeof(int64_t);
    zmq_getsockopt(skt, ZMQ_RCVMORE, &more, &more_size);
    while (more){
	zmq_msg_init(&msg);
	zmq_recv(skt, &msg, 0);
	zmq_getsockopt(skt, ZMQ_RCVMORE, &more, &more_size);
	zmq_msg_close(&msg);
    }
    return 0;
}



#endif /* _HELPHER_H */
