/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */


#include <gpac/m4_tools.h>
#include <sys/timeb.h>

#define SOCK_MICROSEC_WAIT	500


#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define LASTSOCKERROR errno

typedef s32 SOCKET;


struct _tagSock
{
	u32 status;
	SOCKET socket;
	u32 type;
	Bool blocking;
	//this is used for server sockets / multicast sockets
	struct sockaddr_in RemoteAddress;
};


/*
		Some NTP tools
*/

#define SECS_1900_TO_1970 2208988800ul

u32 M4_GetNTP32(u32 sec, u32 frac)
{
	return ( ((sec  & 0x0000ffff) << 16) |  ((frac & 0xffff0000) >> 16));
}
         

void M4_GetNTP(u32 *sec, u32 *frac)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	*sec = now.tv_sec + SECS_1900_TO_1970;
	*frac = (now.tv_usec << 12) + (now.tv_usec << 8) - ((now.tv_usec * 3650) >> 6);
}



M4Err SK_GetHostName(char *buffer)
{
	s32 ret;
	ret = gethostname(buffer, MAX_IP_NAME_LEN);

	if (ret == SOCKET_ERROR) return M4InvalidPeerName;
	return M4OK;
}

M4Err SK_GetSocketIP(M4Socket *sock, char *buffer)
{
	struct sockaddr_in name;
	char *ip;
	u32 len = sizeof(struct sockaddr_in);

	buffer[0] = 0;
	if (getsockname(sock->socket, (struct sockaddr*) &name, &len)) return M4NetworkFailure;

	ip = inet_ntoa(name.sin_addr);
	if (!ip) return M4NetworkFailure;
	sprintf(buffer, ip);
	return M4OK;
}


//Socket Group for select(). The group is a collection of sockets ready for reading / writing
struct _tagSockGroup
{
	//the max time value before a select returns
	struct timeval timeout;
	fd_set ReadGroup;
	fd_set WriteGroup;
};

M4Socket *NewSocket(u32 SocketType)
{
	M4Socket *tmp;
	if ((SocketType != SK_TYPE_UDP) && (SocketType != SK_TYPE_TCP)) return NULL;

	tmp = malloc(sizeof(M4Socket));
	memset(tmp, 0, sizeof(M4Socket));

	tmp->socket = socket(AF_INET, (SocketType == SK_TYPE_UDP) ? SOCK_DGRAM : SOCK_STREAM, 0);
	if (tmp->socket == INVALID_SOCKET) {
		free(tmp);
		return NULL;
	}

	tmp->type = SocketType;
	tmp->status = SK_STATUS_CREATE;
	tmp->blocking = 1;

	memset(&tmp->RemoteAddress, 0, sizeof(struct sockaddr_in));

	return tmp;
}

M4Err SK_SetBufferSize(M4Socket *sock, Bool SendBuffer, u32 NewSize)
{
	if (!sock) return M4BadParam;

	if (SendBuffer) {
		setsockopt(sock->socket, SOL_SOCKET, SO_SNDBUF, (char *) &NewSize, sizeof(u32) );
	} else {
		setsockopt(sock->socket, SOL_SOCKET, SO_RCVBUF, (char *) &NewSize, sizeof(u32) );
	}
	return M4OK;
}

M4Err SK_SetBlockingMode(M4Socket *sock, u32 NonBlockingOn)
{
	s32 res;
	s32 flag = fcntl(sock->socket, F_GETFL, 0);
	res = fcntl(sock->socket, F_SETFL, flag | O_NONBLOCK);
	if (res) return M4ServiceError;
	sock->blocking = NonBlockingOn ? 0 : 1;
	return M4OK;
}


void SK_Delete(M4Socket *sock)
{
	close(sock->socket);
	free(sock);
}

void SK_Reset(M4Socket *sock)
{
	u32 clear;

	if (!sock) return;
	//clear the socket buffer and state
	setsockopt(sock->socket, SOL_SOCKET, SO_ERROR, (char *) &clear, sizeof(u32) );

}


//connects a socket to a remote peer on a given port
M4Err SK_Connect(M4Socket *sock, char *PeerName, u16 PortNumber)
{
	s32 ret;
	struct hostent *Host;

	/*tcp connection - setup the address*/
	memset(&sock->RemoteAddress, 0, sizeof(struct sockaddr_in));
	sock->RemoteAddress.sin_family = AF_INET;
	sock->RemoteAddress.sin_port = htons(PortNumber);

	/*get the server IP. */
	sock->RemoteAddress.sin_addr.s_addr = inet_addr(PeerName);
	if (sock->RemoteAddress.sin_addr.s_addr==INADDR_NONE) {
		Host = gethostbyname(PeerName);
		if (Host == NULL) return M4AddressNotFound;
		memcpy((char *) &sock->RemoteAddress.sin_addr, Host->h_addr_list[0], Host->h_length);
	}

	if (sock->type==SK_TYPE_TCP) {
		ret = connect(sock->socket, (struct sockaddr *) &sock->RemoteAddress, sizeof(sock->RemoteAddress));
		if (ret == SOCKET_ERROR) {
			switch (LASTSOCKERROR) {
			case EAGAIN:
				return M4SockWouldBlock;
			case EISCONN:
				return M4OK;
			default:
				return M4ConnectionFailed;
			}
		}
	}
	sock->status = SK_STATUS_CONNECT;
	return M4OK;
}

//binds the given socket to the specified port. If ReUse is true
//this will enable reuse of ports on a single machine
M4Err SK_Bind(M4Socket *sock, u16 PortNumber, Bool reUse)
{
	s32 ret;
	s32 optval;
	char buf[MAX_IP_NAME_LEN];
	struct sockaddr_in LocalAdd;
	struct hostent *Host;

	if (!sock || (sock->status != SK_STATUS_CREATE)) return M4BadParam;

	memset((void *) &LocalAdd, 0, sizeof(LocalAdd));
	//ger the local name
	ret = gethostname(buf, MAX_IP_NAME_LEN);
	if (ret == SOCKET_ERROR) return M4InvalidPeerName;
	//get the IP address
	Host = gethostbyname(buf);
	if (Host == NULL) return M4AddressNotFound;
	//setup the address
	memcpy((char *) &LocalAdd.sin_addr, Host->h_addr_list[0], sizeof(u32));
	LocalAdd.sin_family = AF_INET;
	LocalAdd.sin_addr.s_addr = INADDR_ANY;
	LocalAdd.sin_port = htons(PortNumber);

	if (reUse) {
		//retry with ReUsability of socket names
		optval = 1;
		ret = setsockopt(sock->socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval));
		if (ret == SOCKET_ERROR) return M4ConnectionFailed;
	}

	//bind the socket
	ret = bind(sock->socket, (struct sockaddr *) &LocalAdd, sizeof(LocalAdd));
	if (ret == SOCKET_ERROR) return M4ConnectionFailed;

	sock->status = SK_STATUS_BIND;
	return M4OK;
}

//send length bytes of a buffer
M4Err SK_Send(M4Socket *sock, unsigned char *buffer, u32 length)
{
	M4Err e;
	u32 Count, Res, ready;
	struct timeval timeout;
	fd_set Group;

	e = M4OK;

	//the socket must be bound or connected
	if (sock->status != SK_STATUS_CONNECT) return M4BadParam;

	//can we write?
	FD_ZERO(&Group);
	FD_SET(sock->socket, &Group);
	timeout.tv_sec = 0;
	timeout.tv_usec = SOCK_MICROSEC_WAIT;

	ready = select(sock->socket+1, NULL, &Group, NULL, &timeout);
	if (ready == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	//should never happen (to check: is writeability is guaranteed for not-connected sockets)
	if (!ready || !FD_ISSET(sock->socket, &Group)) {
		return M4NetworkEmpty;
	}

	//direct writing
	Count = 0;
	while (Count < length) {
		if (sock->type == SK_TYPE_TCP) {
			Res = send(sock->socket, &buffer[Count], length - Count, 0);
		} else {
			Res = sendto(sock->socket, &buffer[Count], length - Count, 0, (struct sockaddr *) &sock->RemoteAddress, sizeof(struct sockaddr));
		}
		if (Res == SOCKET_ERROR) {
			switch (LASTSOCKERROR) {
			case EAGAIN:
				return M4SockWouldBlock;
			case ENOTCONN:
			case ECONNRESET:
				return M4ConnectionClosed;
			default:
				return M4NetworkUnreachable;
			}
		}
		Count += Res;
	}
	return M4OK;
}


u32 SK_IsMulticastAddress(char *multi_IPAdd)
{
	if (!multi_IPAdd) return 0;
	return ((htonl(inet_addr(multi_IPAdd)) >> 8) & 0x00f00000) == 0x00e00000;	
}

//binds MULTICAST
M4Err SK_MulticastSetup(M4Socket *sock, char *multi_IPAdd, u16 MultiPortNumber, u32 TTL, Bool NoBind)
{
	s32 ret;
	s32 optval;
	u_long mc_add;
	u32 flag; 
	struct sockaddr_in LocalAdd;
	struct ip_mreq M_req;

	if (!sock || (sock->status != SK_STATUS_CREATE)) return M4BadParam;

	if (TTL > 255) return M4BadParam;

	memset((void *) &LocalAdd, 0, sizeof(LocalAdd));

	//check the address
	mc_add = inet_addr(multi_IPAdd);
	if (!((mc_add >= 0xe0000000) || (mc_add <= 0xefffffff) )) return M4BadParam;

	//retry with ReUsability of socket names
	optval = SO_REUSEADDR;
	setsockopt(sock->socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &optval, sizeof(optval));

	//bind to ANY interface WITHOUT port number
	LocalAdd.sin_family = AF_INET;
	LocalAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	LocalAdd.sin_port = htons( MultiPortNumber);

	if (!NoBind) {
		//bind the socket
		ret = bind(sock->socket, (struct sockaddr *) &LocalAdd, sizeof(LocalAdd));
		if (ret == SOCKET_ERROR) {
			return M4ConnectionFailed;
		}
	}
	sock->status = SK_STATUS_BIND;
 
	//now join the multicast
	M_req.imr_multiaddr.s_addr = mc_add;
	//ANY interfaces for now
	M_req.imr_interface.s_addr = INADDR_ANY;
	ret = setsockopt(sock->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
				(char *) &M_req, sizeof(M_req));

	if (ret == SOCKET_ERROR) {
		return M4ConnectionFailed;
	}
	
	//set the Time To Live
	ret = setsockopt(sock->socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
	if (ret == SOCKET_ERROR) return M4ConnectionFailed;

	//Disable loopback
	flag = 1;
	ret = setsockopt(sock->socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &flag, sizeof(flag));
	if (ret == SOCKET_ERROR) return M4ConnectionFailed;

	sock->RemoteAddress.sin_family = AF_INET;
	sock->RemoteAddress.sin_addr.s_addr = mc_add;
	sock->RemoteAddress.sin_port = htons( MultiPortNumber);
	return M4OK;
}




//fetch nb bytes on a socket and fill the buffer from startFrom
//length is the allocated size of the receiving buffer
//BytesRead is the number of bytes read from the network
M4Err SK_Receive(M4Socket *sock, unsigned char *buffer, u32 length, u32 startFrom, u32 *BytesRead)
{
	M4Err e;
	u32 res, ready;
	struct timeval timeout;
	fd_set Group;

	e = M4OK;

	*BytesRead = 0;
	if (startFrom >= length) return 0;


	//can we read?
	FD_ZERO(&Group);
	FD_SET(sock->socket, &Group);
	timeout.tv_sec = 0;
	timeout.tv_usec = SOCK_MICROSEC_WAIT;

	res = 0;
	ready = select(sock->socket+1, &Group, NULL, NULL, &timeout);
	if (ready == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	if (!FD_ISSET(sock->socket, &Group)) {
		return M4NetworkEmpty;
	}

	res = recv(sock->socket, buffer + startFrom, length - startFrom, 0);
	if (res == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EMSGSIZE:
			return M4OutOfMem;
		case EAGAIN:
			return M4SockWouldBlock;
		case ENOTCONN:
		case ECONNRESET:
			return M4ConnectionClosed;
		default:
			return M4NetworkUnreachable;
		}
	}
	*BytesRead = res;
	return M4OK;
}


Bool SK_Listen(M4Socket *sock, u32 MaxConnection)
{
	s32 i;
	if (sock->status != SK_STATUS_BIND) return 0;
	if (MaxConnection >= SOMAXCONN) MaxConnection = SOMAXCONN;
	i = listen(sock->socket, MaxConnection);
	if (i == SOCKET_ERROR) return 0;
	sock->status = SK_STATUS_LISTEN;
	return 1;
}

M4Err SK_Accept(M4Socket *sock, M4Socket **newConnection)
{
	u32 clientAddSize, ready, res;
	SOCKET sk;
	struct timeval timeout;
	fd_set Group;

	*newConnection = NULL;
	if (sock->status != SK_STATUS_LISTEN) return M4BadParam;

	//can we read?
	FD_ZERO(&Group);
	FD_SET(sock->socket, &Group);
	timeout.tv_sec = 0;
	timeout.tv_usec = SOCK_MICROSEC_WAIT;

	res = 0;
	ready = select(sock->socket, &Group, NULL, NULL, &timeout);
	if (ready == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	if (!ready || !FD_ISSET(sock->socket, &Group)) return M4NetworkEmpty;

	clientAddSize = sizeof(struct sockaddr_in);
	sk = accept(sock->socket, (struct sockaddr *) &sock->RemoteAddress, &clientAddSize);

	//we either have an error or we have no connections
	if (sk == INVALID_SOCKET) {
		if (sock->blocking) return M4NetworkFailure;
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkFailure;
		}		
	}

	
	(*newConnection) = malloc(sizeof(M4Socket));
	(*newConnection)->socket = sk;
	(*newConnection)->type = sock->type;
	(*newConnection)->blocking = sock->blocking;
	(*newConnection)->status = SK_STATUS_CONNECT;
	memcpy( &(*newConnection)->RemoteAddress, &sock->RemoteAddress, clientAddSize);

	memset(&sock->RemoteAddress, 0, sizeof(struct sockaddr_in));

	return M4OK;
}

M4Err SK_GetLocalInfo(M4Socket *sock, u16 *Port, u32 *Familly)
{
	struct sockaddr_in the_add;
	u32 size, fam;

	*Port = 0;
	*Familly = 0;

	if (!sock || sock->status != SK_STATUS_CONNECT) return M4BadParam;

	size = sizeof(struct sockaddr_in);
	if (getsockname(sock->socket, (struct sockaddr *) &the_add, &size) == SOCKET_ERROR) return M4NetworkFailure;
	*Port = (u32) ntohs(the_add.sin_port);

	size = 4;
	if (getsockopt(sock->socket, SOL_SOCKET, SO_TYPE, (char *) &fam, &size) == SOCKET_ERROR)
		return M4NetworkFailure;

	switch (fam) {
	case SOCK_DGRAM:
		*Familly = SK_TYPE_UDP;
		return M4OK;
	case SOCK_STREAM:
		*Familly = SK_TYPE_TCP;
		return M4OK;
	default:
		*Familly = 0;
		return M4OK;
	}
}

//we have to do this for the server sockets as we use only one thread 
M4Err SK_SetServerMode(M4Socket *sock, Bool serverOn)
{
	u32 one;

	if (!sock 
		|| (sock->type != SK_TYPE_TCP)
		|| (sock->status != SK_STATUS_CONNECT)
		)
		return M4BadParam;

	one = serverOn ? 1 : 0;
	setsockopt(sock->socket, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(u32));
	setsockopt(sock->socket, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof(u32));
	return M4OK;
}

M4Err SK_GetRemoteAddress(M4Socket *sock, char *buf)
{
	if (!sock || sock->status != SK_STATUS_CONNECT) return M4BadParam;
	sprintf(buf, inet_ntoa(sock->RemoteAddress.sin_addr));
	return M4OK;	
}


M4Err SK_SetRemoteAddress(M4Socket *sock, char *address)
{
	struct hostent *Host;

	if (!sock || !address) return M4BadParam;

	//setup the address
	sock->RemoteAddress.sin_family = AF_INET;

	/*get the server IP. */
	sock->RemoteAddress.sin_addr.s_addr = inet_addr(address);
	if (sock->RemoteAddress.sin_addr.s_addr==INADDR_NONE) {
		Host = gethostbyname(address);
		if (Host == NULL) return M4AddressNotFound;
		memcpy((char *) &sock->RemoteAddress.sin_addr, Host->h_addr_list[0], Host->h_length);
	}
	return M4OK;
}

M4Err SK_SetRemotePort(M4Socket *sock, u16 RemotePort)
{
	if (!sock || !RemotePort) return M4BadParam;
	sock->RemoteAddress.sin_port = htons(RemotePort);
	return M4OK;
}


//send length bytes of a buffer
M4Err SK_SendTo(M4Socket *sock, unsigned char *buffer, u32 length, unsigned char *remoteHost, u16 remotePort)
{
	u32 Count, Res, ready;
	struct sockaddr_in remote;
	struct hostent *Host;
	struct timeval timeout;
	fd_set Group;

	//the socket must be bound or connected
	if ((sock->status != SK_STATUS_BIND) && (sock->status != SK_STATUS_CONNECT)) return M4BadParam;
	if (remoteHost && !remotePort) return M4BadParam;

	//can we write?
	FD_ZERO(&Group);
	FD_SET(sock->socket, &Group);
	timeout.tv_sec = 0;
	timeout.tv_usec = SOCK_MICROSEC_WAIT;

	ready = select(sock->socket+1, NULL, &Group, NULL, &timeout);
	if (ready == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	//to check: writeability is guaranteed for not-connected sockets
	if (sock->status == SK_STATUS_CONNECT) {
		if (!ready || !FD_ISSET(sock->socket, &Group)) return M4NetworkEmpty;
	}

	//setup the address
	remote.sin_family = AF_INET;
	//if a remote host is specified, use it. Otherwise use the default host
	if (remoteHost) {
		//setup the address
		remote.sin_port = htons(remotePort);
		//get the server IP
		Host = gethostbyname(remoteHost);
		if (Host == NULL) return M4AddressNotFound;
		memcpy((char *) &remote.sin_addr, Host->h_addr_list[0], sizeof(u32));
	} else {
		remote.sin_port = sock->RemoteAddress.sin_port;
		remote.sin_addr.s_addr = sock->RemoteAddress.sin_addr.s_addr;
	}
	
	Count = 0;
	while (Count < length) {
		Res = sendto(sock->socket, &buffer[Count], length - Count, 0, (struct sockaddr *) &remote, sizeof(remote));
		
		if (Res == SOCKET_ERROR) {
			switch (LASTSOCKERROR) {
			case EAGAIN:
				return M4SockWouldBlock;
			default:
				return M4NetworkUnreachable;
			}
		}
		Count += Res;
	}
	return M4OK;
}




M4Err SK_ReceiveWait(M4Socket *sock, unsigned char *buffer, u32 length, u32 startFrom, u32 *BytesRead, u32 Second )
{
	M4Err e;
	u32 res, ready;
	struct timeval timeout;
	fd_set Group;

	e = M4OK;

	*BytesRead = 0;
	if (startFrom >= length) return 0;


	//can we read?
	FD_ZERO(&Group);
	FD_SET(sock->socket, &Group);
	timeout.tv_sec = Second;
	timeout.tv_usec = SOCK_MICROSEC_WAIT;

	res = 0;
	ready = select(sock->socket+1, &Group, NULL, NULL, &timeout);
	if (ready == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	if (!FD_ISSET(sock->socket, &Group)) {
		return M4NetworkEmpty;
	}

	res = recv(sock->socket, buffer + startFrom, length - startFrom, 0);
	if (res == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	*BytesRead = res;
	return M4OK;
}


//send length bytes of a buffer
M4Err SK_SendWait(M4Socket *sock, unsigned char *buffer, u32 length, u32 Second )
{

	M4Err e;
	u32 Count, Res, ready;
	struct timeval timeout;
	fd_set Group;

	e = M4OK;

	//the socket must be bound or connected
	if (sock->status != SK_STATUS_CONNECT) return M4BadParam;

	//can we write?
	FD_ZERO(&Group);
	FD_SET(sock->socket, &Group);
	timeout.tv_sec = Second;
	timeout.tv_usec = SOCK_MICROSEC_WAIT;

	ready = select(sock->socket+1, NULL, &Group, NULL, &timeout);
	if (ready == SOCKET_ERROR) {
		switch (LASTSOCKERROR) {
		case EAGAIN:
			return M4SockWouldBlock;
		default:
			return M4NetworkUnreachable;
		}
	}
	//should never happen (to check: is writeability is guaranteed for not-connected sockets)
	if (!ready || !FD_ISSET(sock->socket, &Group)) {
		return M4NetworkEmpty;
	}

	//direct writing
	Count = 0;
	while (Count < length) {
		Res = send(sock->socket, &buffer[Count], length - Count, 0);
		if (Res == SOCKET_ERROR) {
			switch (LASTSOCKERROR) {
			case EAGAIN:
				return M4SockWouldBlock;
			case ECONNRESET:
				return M4ConnectionClosed;
			default:
				return M4NetworkUnreachable;
			}
		}
		Count += Res;
	}
	return M4OK;
}











SockGroup *NewSockGroup()
{	
	SockGroup *tmp = malloc(sizeof(SockGroup));
	if (!tmp) return NULL;
	FD_ZERO(&tmp->ReadGroup);
	FD_ZERO(&tmp->WriteGroup);
	return tmp;
}

void SKG_Delete(SockGroup *group)
{
	free(group);
}

void SKG_SetWatchTime(SockGroup *group, u32 DelayInS, u32 DelayInMicroS)
{
	group->timeout.tv_sec = DelayInS;
	group->timeout.tv_usec = DelayInMicroS;
}

void SKG_AddSocket(SockGroup *group, M4Socket *sock, u32 GroupType)
{

	switch (GroupType) {
	case SK_GROUP_READ:
		FD_SET(sock->socket, &group->ReadGroup);
		return;
	case SK_GROUP_WRITE:
		FD_SET(sock->socket, &group->WriteGroup);
		return;
	default:
		return;
	}
}

void SKG_RemoveSocket(SockGroup *group, M4Socket *sock, u32 GroupType)
{
	switch (GroupType) {
	case SK_GROUP_READ:
		FD_CLR(sock->socket, &group->ReadGroup);
		return;
	case SK_GROUP_WRITE:
		FD_CLR(sock->socket, &group->WriteGroup);
		return;
	default:
		return;
	}
}


Bool SKG_IsSocketIN(SockGroup *group, M4Socket *sock, u32 GroupType)
{

	switch (GroupType) {
	case SK_GROUP_READ:
		if (FD_ISSET(sock->socket, &group->ReadGroup)) return 1;
		return 0;
	case SK_GROUP_WRITE:
		if (FD_ISSET(sock->socket, &group->WriteGroup)) return 1;
		return 0;
	default:
		return 0;
	}
}

u32 SKG_Select(SockGroup *group)
{
	u32 ready, rien = 0;
	ready = select(rien, &group->ReadGroup, &group->WriteGroup, NULL, &group->timeout);
	if (ready == SOCKET_ERROR) return 0;
	return ready;
}


