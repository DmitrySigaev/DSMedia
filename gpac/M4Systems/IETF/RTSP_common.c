/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / IETF RTP/RTSP/SDP sub-project
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

#include <gpac/intern/m4_ietf_dev.h>


M4Err RTSP_ReadReply(LPRTSPSESSION sess)
{
	M4Err e;
	u32 res, body_size;
	u32 BodyStart = 0;

	//fetch more data on the socket if needed
	while (1) {
		//Locate header / body
		if (!BodyStart) RTSP_GetBodyInfo(sess, &BodyStart, &body_size);
		
		//enough data
		res = sess->CurrentSize - sess->CurrentPos;
		if (!body_size || (res >= body_size + BodyStart)) {
			//done
			break;
		}
		//this is the tricky part: if we do NOT have a body start -> we refill
		e = RTSP_RefillBuffer(sess);
		if (e) return e;
	}
	return M4OK;
}

void RTSP_GetBodyInfo(RTSPSession *sess, u32 *body_start, u32 *body_size)
{
	u32 i;
	char *buffer;
	char *cl_str, val[30];

	*body_start = *body_size = 0;

	buffer = sess->TCPBuffer + sess->CurrentPos;
	*body_start = SP_FindPattern(buffer, 0, sess->CurrentSize - sess->CurrentPos, "\r\n\r\n");

	//if found add the 2 "\r\n" and parse it
	if (*body_start) {
		*body_start += 4;
		
		//get the content length
		cl_str = strstr(buffer, "Content-Length: ");
		if (!cl_str) cl_str = strstr(buffer, "Content-length: ");
		
		if (cl_str) {
			cl_str += 16;
			i = 0;
			while (cl_str[i] != '\r') {
				val[i] = cl_str[i];
				i += 1;
			}
			val[i] = 0;
			*body_size = atoi(val);
		} else {
			*body_size = 0;
		}
	}
}


M4Err RTSP_RefillBuffer(RTSPSession *sess)
{
	M4Err e;
	u32 res;
	char *ptr;

	if (!sess) return M4BadParam;
	if (!sess->connection) return M4NetworkEmpty;

	res = sess->CurrentSize - sess->CurrentPos;
	if (!res) return RTSP_FillTCPBuffer(sess);

//	printf("Forcing reading\n");
	
	ptr = malloc(sizeof(char) * res);
	memcpy(ptr, sess->TCPBuffer+sess->CurrentPos, res);
	memcpy(sess->TCPBuffer, ptr, res);
	free(ptr);

	sess->CurrentPos = 0;
	sess->CurrentSize = res;

	//now read from current pos
	e = SK_Receive(sess->connection, sess->TCPBuffer + sess->CurrentSize, 
				RTSP_TCP_BUF_SIZE - sess->CurrentSize, 
				0, &res);

	if (!e) {
		sess->CurrentSize += res;
	}
	return e;
}


M4Err RTSP_FillTCPBuffer(RTSPSession *sess)
{
	M4Err e = M4OK;

	if (!sess->connection) return M4NetworkEmpty;

	if (sess->CurrentSize == sess->CurrentPos) {
		e = SK_Receive(sess->connection, sess->TCPBuffer, RTSP_TCP_BUF_SIZE, 0, &sess->CurrentSize);
		sess->CurrentPos = 0;
		sess->TCPBuffer[sess->CurrentSize] = 0;
		if (e) sess->CurrentSize = 0;
	} else if (!sess->CurrentSize) e = M4NetworkEmpty;
	return e;
}


RTSPTransport *ParseTransport(char *buffer)
{
	Bool IsFirst;
	char buf[100], param_name[100], param_val[100];
	s32 pos, nPos;
	RTSPTransport *tmp;
	pos = 0;
	if (!buffer) return NULL;
	//only support for RTP/AVP for now
	if (!strstr(buffer, "RTP/AVP")) 
		if (!strstr(buffer, "rtp/avp")) 
			return NULL;

	tmp = malloc(sizeof(RTSPTransport));
	memset(tmp, 0, sizeof(RTSPTransport));

	IsFirst = 1;
	pos = 0;
	while (1) {	
		pos = SP_GetComponent(buffer, pos, " ;", buf, 100);
		if (pos <= 0) break;
		if (strstr(buf, "=")) {
			nPos = SP_GetComponent(buf, 0, "=", param_name, 100);
			nPos = SP_GetComponent(buf, nPos, "=", param_val, 100);
		} else {
			strcpy(param_name, buf);
		}
		
		//very first param is the profile
		if (IsFirst) {
			tmp->Profile = strdup(param_name);
			IsFirst = 0;
			continue;
		}

		if (!stricmp(param_name, "destination")) {
			if (tmp->destination) free(tmp->destination);
			tmp->destination = strdup(param_val);
		}
		else if (!stricmp(param_name, "source")) {
			if (tmp->source) free(tmp->source);
			tmp->source = strdup(param_val);
		}
		else if (!stricmp(param_name, "unicast")) tmp->IsUnicast = 1;
		else if (!stricmp(param_name, "RECORD")) tmp->IsRecord = 1;
		else if (!stricmp(param_name, "append")) tmp->Append = 1;
		else if (!stricmp(param_name, "interleaved")) {
			u32 rID, rcID;
			tmp->IsInterleaved = 1;
			if (sscanf(param_val, "%d-%d", &rID, &rcID) == 1) {
				sscanf(param_val, "%d", &rID);
				tmp->rtcpID = tmp->rtpID = (u8) rID;
			} else {
				tmp->rtpID = (u8) rID;
				tmp->rtcpID = (u8) rcID;
			}
		}
		else if (!stricmp(param_name, "layers")) sscanf(param_val, "%d", &tmp->MulticastLayers);
		else if (!stricmp(param_name, "ttl")) sscanf(param_val, "%c	", &tmp->TTL);
		else if (!stricmp(param_name, "port")) sscanf(param_val, "%hd-%hd", &tmp->port_first, &tmp->port_last);
		else if (!stricmp(param_name, "server_port")) sscanf(param_val, "%hd-%hd", &tmp->port_first, &tmp->port_last);
		else if (!stricmp(param_name, "client_port")) sscanf(param_val, "%hd-%hd", &tmp->client_port_first, &tmp->client_port_last);
		else if (!stricmp(param_name, "ssrc")) sscanf(param_val, "%d", &tmp->SSRC);
	}
	return tmp;
}



M4Err RTSP_ParseHeaderLines(char *buffer, u32 BufferSize, u32 BodyStart, RTSPCommand *com, RTSPResponse *rsp)
{
	unsigned char LineBuffer[1024];
	unsigned char HeaderBuf[100], ValBuf[1024], temp[400];
	s32 Pos, LinePos;
	u32 HeaderLine;

	//then parse the full header
	LinePos = 0;
	strcpy(HeaderBuf, "");
	while (1) {
		HeaderLine = 0;
		LinePos = SP_GetOneLine(buffer, LinePos, BufferSize, LineBuffer, 1024);
		if (LinePos <= 0) return M4SignalingFailure;
		
		//extract field header and value. Warning: some params (transport, ..) may be on several lines 
		Pos = SP_GetComponent(LineBuffer, 0, ":\r\n", temp, 400);
		
		//end of header
		if (Pos <= 0) {
			HeaderLine = 2;
		}
		//this is a header
		else if (LineBuffer[0] != ' ') {
			HeaderLine = 1;
		} else {
			Pos = SP_GetComponent(LineBuffer, 0, ", \r\n", temp, 400);
			//end of header - process any pending one
			if (Pos <= 0) {
				HeaderLine = 2;
			} else {
				//n-line value - append
				strcat(ValBuf, "\r\n");
				strcat(ValBuf, temp);
				continue;
			}
		}
		//process current value
		if (HeaderLine && strlen(HeaderBuf)) {
			if (rsp) {
				RTSP_SetResponseVal(rsp, HeaderBuf, ValBuf);
			}
			else {
				RTSP_SetCommandVal(com, HeaderBuf, ValBuf);
			}
		}
		//done with the header
		if ( (HeaderLine == 2) || ((u32) LinePos >= BodyStart) ) return M4OK;

		//process current line
		strcpy(HeaderBuf, temp);

		//skip ':'
		Pos += 1;
		//a server should normally reply with a space, but check it
		if (LineBuffer[Pos] == ' ') Pos += 1;
		Pos = SP_GetComponent(LineBuffer, Pos, "\r\n", ValBuf, 400);
		if (Pos <= 0) break;

	}
	//if we get here we haven't reached the BodyStart
	return M4SignalingFailure;
}


const char *RTSP_NC_TO_String(u32 ErrCode)
{
	switch (ErrCode) {
	case NC_RTSP_Continue:
		return "Continue";
	case NC_RTSP_OK:	
		return "OK";
	case NC_RTSP_Created:	
		return "Created";
	case NC_RTSP_Low_on_Storage_Space:
		return "Low on Storage Space";
	case NC_RTSP_Multiple_Choice:
		return "Multiple Choice";
	case NC_RTSP_Moved_Permanently:
		return "Moved Permanently";
	case NC_RTSP_Moved_Temporarily:
		return "Moved Temporarily";
	case NC_RTSP_See_Other:
		return "See Other";
	case NC_RTSP_Use_Proxy:
		return "Use Proxy";
	case NC_RTSP_Bad_Request:
		return "Bad Request";
	case NC_RTSP_Unauthorized:
		return "Unauthorized";
	case NC_RTSP_Payment_Required:
		return "Payment Required";
	case NC_RTSP_Forbidden:
		return "Forbidden";
	case NC_RTSP_Not_Found:
		return "Not Found";
	case NC_RTSP_Method_Not_Allowed:
		return "Method Not Allowed";
	case NC_RTSP_Not_Acceptable:
		return "Not Acceptable";
	case NC_RTSP_Proxy_Authentication_Required:
		return "Proxy Authentication Required";
	case NC_RTSP_Request_Timeout:
		return "Request Timeout";
	case NC_RTSP_Gone:
		return "Gone";
	case NC_RTSP_Length_Required:
		return "Length Required";
	case NC_RTSP_Precondition_Failed:
		return "Precondition Failed";
	case NC_RTSP_Request_Entity_Too_Large:
		return "Request Entity Too Large";
	case NC_RTSP_Request_URI_Too_Long:
		return "Request URI Too Long";
	case NC_RTSP_Unsupported_Media_Type:
		return "Unsupported Media Type";
	case NC_RTSP_Invalid_parameter:
		return "Invalid parameter";
	case NC_RTSP_Illegal_Conference_Identifier:
		return "Illegal Conference Identifier";
	case NC_RTSP_Not_Enough_Bandwidth:
		return "Not Enough Bandwidth";
	case NC_RTSP_Session_Not_Found:
		return "Session Not Found";
	case NC_RTSP_Method_Not_Valid_In_This_State:
		return "Method Not Valid In This State";
	case NC_RTSP_Header_Field_Not_Valid:
		return "Header Field Not Valid";
	case NC_RTSP_Invalid_Range:
		return "Invalid Range";
	case NC_RTSP_Parameter_Is_ReadOnly:
		return "Parameter Is Read-Only";
	case NC_RTSP_Aggregate_Operation_Not_Allowed:
		return "Aggregate Operation Not Allowed";
	case NC_RTSP_Only_Aggregate_Operation_Allowed:
		return "Only Aggregate Operation Allowed";
	case NC_RTSP_Unsupported_Transport:
		return "Unsupported Transport";
	case NC_RTSP_Destination_Unreachable:
		return "Destination Unreachable";
	case NC_RTSP_Internal_Server_Error:
		return "Internal Server Error";
	case NC_RTSP_Bad_Gateway:
		return "Bad Gateway";
	case NC_RTSP_Service_Unavailable:
		return "Service Unavailable";
	case NC_RTSP_Gateway_Timeout:
		return "Gateway Timeout";
	case NC_RTSP_RTSP_Version_Not_Supported:
		return "RTSP Version Not Supported";
	case NC_RTSP_Option_not_support:
		return "Option not support";

	case NC_RTSP_Not_Implemented:
	default:
		return "Not Implemented";
	}
}
