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


void MP4TypeToString(u32 type, char *name)
{
	u32 ch, i;
	for (i = 0; i < 4; i++, name++) {
		ch = type >> (8 * (3-i) ) & 0xff;
		if ( ch >= 0x20 && ch <= 0x7E ) {
			*name = ch;
		} else {
			*name = '.';
		}
	}
	*name = 0;
}

const char *M4ErrToString(M4Err e)
{
	switch (e) {
	case M4ScriptInfo:
		return "Script message";
	case M4FieldNotQuantized:
		return "Warning: Field not quantized";
	case M4EOF:
		return "End Of Stream / File";
	case M4OK:
		return "No Error";

	/*General errors */
	case M4BadParam:
		return "Bad Parameter";
	case M4OutOfMem:
		return "Out Of Memory";
	case M4IOErr:
		return "I/O Error";
	case M4NotSupported:
		return "Feature Not Supported";
	case M4CorruptedData:
		return "Corrupted Data in file/stream";
	case M4InvalidPlugin:
		return "Invalid Plugin";

	/*File Format Errors */
	case M4ReadAtomFailed:
		return "Read MP4 Atom Failed";
	case M4WriteAtomFailed:
		return "Write MP4 Atom Failed";
	case M4InvalidAtom:
		return "Invalid MP4 Atom";
	case M4UncompleteFile:
		return "MP4 File is truncated";
	case M4MP4ReadFailed:
		return "MP4 File Read Failed";
	case M4InvalidMP4File:
		return "Invalid MP4 File";
	case M4InvalidMP4Media:
		return "Invalid MP4 Media";
	case M4InvalidMP4Mode:
		return "Invalid MP4 Mode while accessing the file";
	case M4InvalidRTPHint:
		return "Invalid RTP Hint Track";
	case M4HintPresent:
		return "Hint Present";
	case M4DataRefNotFound:
		return "Media Data Reference not found";
	case M4InvalidTrackID:
		return "Invalid Track ID";

	/*Object Descriptor Errors */
	case M4ReadDescriptorFailed:
		return "Read MPEG-4 Descriptor Failed";
	case M4WriteDescriptorFailed:
		return "Write MPEG-4 Descriptor Failed";
	case M4InvalidDescriptor:
		return "Invalid MPEG-4 Descriptor";
	case M4DescriptorNotAllowed:
		return "MPEG-4 Descriptor Not Allowed";
	case M4PrivateDescriptorUnknown:
		return "Private Descriptor Unknown to MPEG-4";
	case M4DescSizeOutOfRange:
		return "MPEG-4 Descriptor too big";
	case M4ReadODCommandFailed:
		return "Read OD Command Failed";
	case M4CommandNotAllowed: 
		return "OD Command Not Allowed";
	case M4ISOForbiddenDescriptor:
		return "ISO Forbidden Descriptor";
	case M4ISOForbiddenQoS:
		return "ISO Forbidden QoS";
	case M4ES_ID_Overflow:
		return "ES_ID Overflow";


	/*BIFS Errors */
	case M4UnknownNode:
		return "Unknown BIFS Node";
	case M4InvalidNode:
		return "Invalid BIFS Node";
	case M4AnimNotSupported:
		return "BIFS-Anim Not Supported";
	case M4InvalidProto:
		return "Invalid Proto Interface";
	case M4UnknowBIFSVersion:
		return "Invalid BIFS version";
	case M4ScriptError:
		return "Invalid Script";

	/*MPEG-4 Errors */
	case M4BufferTooSmall:
		return "Bad Buffer size (too small)";
	case M4NonCompliantBitStream:
		return "BitStream Not Compliant";
	case M4CodecNotFound:
		return "Media Codec not found";
	
	/*DMIF errors - local and control plane */
	case M4InvalidURL:
		return "The specified URL is not a valid one";
	case M4UnsupportedURL:
		return "This URL is not supported";
	case M4URLNotFound:
		return "Cannot find requested URL";
	
	case M4InvalidPeerName:
		return "Peer Name is invalid ";
	case M4InvalidPeerAddress:
		return "Peer Address is invalid ";

	case M4ServiceNotFound:
		return "Requested Service couldn't be found";
	case M4ServiceError:
		return "Internal Service Error";
	case M4RemotePeerError:
		return "An Error occured at the server/peer side";
	case M4SignalingFailure:
		return "Dialog Failure with remote peer";
	case M4ControlNotSupported:
		return "Desired stream control is not supported by protocol";

	case M4ChannelNotFound:
		return "Media Channel couldn't be found";
	
	case M4AddressNotFound:
		return "IP Address Not Found";
	case M4ConnectionFailed:
		return "IP Connection Failed";
	case M4NetworkUnreachable:
		return "Network Unreachable";
	case M4NetworkFailure:
		return "Network Failure";
	
	case M4NetworkEmpty:
		return "Network Timeout";
	case M4SockWouldBlock:
		return "Socket Would Block";
	case M4ConnectionClosed:
		return "Connection to server closed";
	case M4UDPTimeOut:
		return "UDP traffic timeout";

	default:
		return "Unknown Error";
	}
}
