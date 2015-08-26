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

#ifndef __M4_TOOLS_H_
#define __M4_TOOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_config.h>

typedef double Double;
typedef float Float;
/* 128 bit IDs */
typedef u8 bin128[16];

#define M4_MAX_FLOAT		FLT_MAX
#define M4_MIN_FLOAT		-M4_MAX_FLOAT
#define M4_EPSILON_FLOAT	FLT_EPSILON
#define M4_SHORT_MAX		SHRT_MAX
#define M4_SHORT_MIN		SHRT_MIN

/*build version - KEEP SPACE SEPARATORS FOR MAKE / GREP (SEE MAIN MAKEFILE)!!!, and NO SPACE in M4_VERSION for proper install*/
#define M4_VERSION       "0.2.3"
#define M4_VERSION_INT	0x00000203

#define ABSDIFF(a, b)	( ( (a) > (b) ) ? ((a) - (b)) : ((b) - (a)) )
#ifndef MIN
#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#endif
#ifndef MAX
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#endif

#define FOUR_CHAR_INT( a, b, c, d ) (((a)<<24)|((b)<<16)|((c)<<8)|(d))
void MP4TypeToString(u32 type, char *name);

#define SAFEALLOC(__ptr, __size) __ptr = malloc(__size); if (__ptr) memset(__ptr, 0, __size);

/**********************************************************************
					ERROR DEFINITIONS
**********************************************************************/

typedef s32 M4Err;
enum
{
	/*positive values are warning/info*/
	M4CommandSkipped					= 4,
	M4ScriptInfo						= 3,
	M4FieldNotQuantized					= 2,
	M4EOF								= 1,

	M4OK								= 0,

	/* General errors */
	M4BadParam							= -10,
	M4OutOfMem							= -11,
	M4IOErr								= -13,
	M4NotSupported						= -14,
	M4CorruptedData						= -16,
	M4InvalidPlugin						= -18,

	/* File Format Errors */
	M4ReadAtomFailed					= -30,
	M4WriteAtomFailed					= -31,
	M4InvalidAtom						= -32,
	M4UncompleteFile					= -33,
	M4MP4ReadFailed						= -34,
	M4InvalidMP4File					= -35,
	M4InvalidMP4Media					= -36,
	M4InvalidMP4Mode					= -37,
	M4InvalidRTPHint					= -38,
	M4HintPresent						= -39,
	M4DataRefNotFound					= -40,
	M4InvalidTrackID					= -41,


	/* Object Descriptor Errors */
	M4ReadDescriptorFailed				= -50,
	M4WriteDescriptorFailed				= -51,
	M4InvalidDescriptor					= -52,
	M4DescriptorNotAllowed				= -53,
	M4PrivateDescriptorUnknown			= -54,
	M4DescSizeOutOfRange				= -55,
	M4ReadODCommandFailed				= -56,
	M4CommandNotAllowed					= -57,
	M4ISOForbiddenDescriptor			= -58,
	M4ISOForbiddenQoS					= -59,
	M4ES_ID_Overflow					= -60,

	/* BIFS Errors */
	M4UnknownNode						= -70,
	M4InvalidNode						= -71,
	M4AnimNotSupported					= -72,
	M4InvalidProto						= -73,
	M4UnknowBIFSVersion					= -74,
	M4ScriptError						= -75,

	/* MPEG-4 Errors */
	M4BufferTooSmall					= -101,
	M4NonCompliantBitStream				= -102,
	M4CodecNotFound						= -103,
	/*signal a framed AU has several AU packed (not compliant) used by decoders*/
	M4PackedFrames						= -104,
	

	/* Net errors - local and remote */
	M4InvalidURL						= -200,
	M4UnsupportedURL					= -201,
	M4URLNotFound						= -202,
	M4InvalidPeerName					= -203,
	M4InvalidPeerAddress				= -204,
	M4ServiceNotFound					= -205,
	M4ServiceError						= -206,
	M4RemotePeerError					= -207,
	M4SignalingFailure					= -208,
	M4ControlNotSupported				= -209,

	M4ChannelNotFound					= -213,

	M4AddressNotFound					= -215,
	M4ConnectionFailed					= -216,
	M4NetworkUnreachable				= -217,
	M4NetworkFailure					= -218,

	/* IP specific for sockets */
	M4NetworkEmpty						= -250,
	M4SockWouldBlock					= -251,
	M4ConnectionClosed					= -252,
	M4PacketTooBig						= -253,
	/*set when UDP traffic doesn't seem to pass*/
	M4UDPTimeOut						= -254,

	M4_LAST_DEFINED						= -400
};


const char *M4ErrToString(M4Err e);

/*supported pixel formats for everything using video*/
enum M4PixelFormats
{
	/*8 bit GREY */
	M4PF_GREYSCALE	=	FOUR_CHAR_INT('G','R','E','Y'),
	/*16 bit greyscale*/
	M4PF_ALPHAGREY	=	FOUR_CHAR_INT('G','R','A','L'),
	/*15 bit RGB*/
	M4PF_RGB_555	=	FOUR_CHAR_INT('R','5','5','5'),
	/*16 bit RGB*/
	M4PF_RGB_565	=	FOUR_CHAR_INT('R','5','6','5'),
	/*24 bit RGB*/
	M4PF_RGB_24		=	FOUR_CHAR_INT('R','G','B','3'),
	/*24 bit BGR - used for graphics cards video format signaling*/
	M4PF_BGR_24		=	FOUR_CHAR_INT('B','G','R','3'),
	/*32 bit RGB*/
	M4PF_RGB_32		=	FOUR_CHAR_INT('R','G','B','4'),
	/*32 bit BGR - used for graphics cards video format signaling*/
	M4PF_BGR_32		=	FOUR_CHAR_INT('B','G','R','4'),

	/*32 bit ARGB.
	
	  Note on textures using 32 bit ARGB: 
		on little endian machines, shall be ordered in memory as BGRA, 
		on big endians, shall be ordered in memory as ARGB
	  so that *(u32*)pixel_mem is always ARGB (0xAARRGGBB).
	*/
	M4PF_ARGB		=	FOUR_CHAR_INT('A','R','G','B'),
	/*32bit RGBA (openGL like)*/
	M4PF_RGBA		=	FOUR_CHAR_INT('R','G','B', 'A'),

	/*	YUV packed formats sampled 1:2:2 horiz, 1:1:1 vert*/
	M4PF_YUY2		=	FOUR_CHAR_INT('Y','U','Y','2'),
	M4PF_YVYU		=	FOUR_CHAR_INT('Y','V','Y','U'),
	M4PF_UYVY		=	FOUR_CHAR_INT('U','Y','V','Y'),
	M4PF_VYUY		=	FOUR_CHAR_INT('V','Y','U','Y'),
	M4PF_Y422		=	FOUR_CHAR_INT('Y','4','2','2'),
	M4PF_UYNV		=	FOUR_CHAR_INT('U','Y','N','V'),
	M4PF_YUNV		=	FOUR_CHAR_INT('Y','U','N','V'),
	M4PF_V422		=	FOUR_CHAR_INT('V','4','2','2'),
	
	/*	YUV planar formats sampled 1:2:2 horiz, 1:2:2 vert*/
	M4PF_YV12		=	FOUR_CHAR_INT('Y','V','1','2'),
	M4PF_IYUV		=	FOUR_CHAR_INT('I','Y','U','V'),
	M4PF_I420		=	FOUR_CHAR_INT('I','4','2','0'),

	/*YV12 + Alpha plane*/
	M4PF_YUVA		=	FOUR_CHAR_INT('Y', 'U', 'V', 'A')

};

/**********************************************************************
					CHAIN: list tool
**********************************************************************/
typedef struct tagChain Chain;

Chain *NewChain();
/* don't forget to delete YOUR objects */
void DeleteChain(Chain *ptr);
/* Get the number of items */
u32 ChainGetCount(Chain *ptr);
/* Add an item at the end of the list */
M4Err ChainAddEntry(Chain *ptr, void* item);
/* insert an item at the specified position (between 0 and ChainGetCount() - 1) */
M4Err ChainInsertEntry(Chain *ptr, void *item, u32 position);
/* Delete an entry from the list (between 0 and ChainGetCount() - 1)
Don't forget to delete your object as well ;) */
M4Err ChainDeleteEntry(Chain *ptr, u32 itemNumber);
/* get the specified entry (between 0 and ChainGetCount() - 1) */
void *ChainGetEntry(Chain *ptr, u32 itemNumber);
/*returns entry index if found, -1 otherwise*/
s32 ChainFindEntry(Chain *ptr, void *item);
/*removes item by index. returns entry index if found, -1 otherwise*/
s32 ChainDeleteItem(Chain *ptr, void *item);
/*empty list content*/
void ChainReset(Chain *ptr);


/**********************************************************************
					BitStream: a bit reader/writer in memory or file
**********************************************************************/
enum
{
	BS_READ = 0,
	BS_WRITE,
	BS_FILE_READ,
	BS_FILE_WRITE_ONLY,
	/* used for editing */
	BS_FILE_READ_WRITE
};

typedef struct BS BitStream, *LPBITSTREAM;

/* Constructs a bitstream from a buffer (READ or WRITE)
in WRITE mode, you can specify a NULL buffer as an input and any size (this
is usefull when you know the max size of an incoming buffer). This is refered
as dynamic Write */
BitStream *NewBitStream(unsigned char *buffer, u64 BufferSize, u32 mode);
/* creates a bitstream from a file handle. You have to open your file in the appropriated mode . 
RESULTS ARE UNEXPECTED IF YOU TOUCH THE FILE WHILE USING THE BITSTREAM */
BitStream *NewBitStreamFromFile(FILE *f, u32 mode);
/* delete the bitstream. If the buffer was created by the bitstream, it is deleted if still present
*/
void DeleteBitStream(BitStream *bs);

u8 BS_ReadBit(BitStream *bs);
u32 BS_ReadInt(BitStream *bs, u32 nBits);
u64 BS_ReadLongInt(BitStream *bs, u32 nBits);
Float BS_ReadFloat(BitStream *bs);
Double BS_ReadDouble(BitStream *bs);
u32 BS_ReadData(BitStream *bs, unsigned char *data, u32 nbBytes);


void BS_WriteInt(BitStream *bs, s32 value, s32 nBits);
void BS_WriteLongInt(BitStream *bs, s64 value, s32 nBits);
void BS_WriteFloat(BitStream *bs, Float value);
void BS_WriteDouble (BitStream *bs, Double value);
u32 BS_WriteData(BitStream *bs, unsigned char *data, u32 nbBytes);

void BS_SetEOSCallback(BitStream *bs, void (*EndOfStream)(void *par), void *par);

/* Align the buffer to the next byte boundary - returns NbBits stuffed */
u8 BS_Align(BitStream *bs);
/* get the number of bytes available till end of buffer (READ modes) or -1 (WRITE modes) */
u64 BS_Available(BitStream *bs);
/* used in WRITE mode to resize the buffer to its current writen size */
u32 BS_CutBuffer(BitStream *bs);
/* used in dynamic WRITE mode to retrieve the content of the buffer */
void BS_GetContent(BitStream *bs, unsigned char **output, u32 *outSize);
/* skips nb bytes */
void BS_SkipBytes(BitStream *bs, u64 nbBytes);
/* rewind nb bytes (files only) */
void BS_Rewind(BitStream *bs, u64 nbBytes);
/*seek to offset after the begining of the stream */
M4Err BS_Seek(BitStream *bs, u64 offset);

/* READ modes only: peek numBits as a UINT from orig position */
u32 BS_PeekBits(BitStream *bs, u32 numBits, u64 orig);

/* Gets the size of a bitstream */
u64 BS_GetPosition(BitStream *bs);
/* Gets the size of a bitstream */
u64 BS_GetSize(BitStream *bs);
/* peeks bytes ahead - bitstream shall be aligned */
M4Err BS_PeekData(BitStream *bs, u64 offset, char *data, u32 size);
/* for FILE mode, performs a seek till end of file before returning the size */
u64 BS_GetRefreshedSize(BitStream *bs);


/*********************************************************************
					Thread Object
**********************************************************************/

#define THREAD_STATUS_STOP		0
#define THREAD_STATUS_PLAY		1
#define THREAD_STATUS_DEAD		2

typedef struct _tagM4Thread M4Thread;
typedef struct _tagM4Thread *LPTHREAD;

/* creates a new thread object */
M4Thread *NewThread();
/* kill the thread if active and delete the object */
void TH_Delete(M4Thread *t);
/* run the thread */
M4Err TH_Run(M4Thread *t, u32 (*Run)(void *param), void *param);
/* stop the thread and wait for its exit */
void TH_Stop(M4Thread *t);
/*get the thread status */
u32 TH_GetStatus(M4Thread *t);

/* thread priority */

enum
{
	TH_PRIOR_IDLE=0,
	TH_PRIOR_LESS_IDLE,
	TH_PRIOR_LOWEST,
	TH_PRIOR_LOW,
	TH_PRIOR_NORMAL,
	TH_PRIOR_HIGH,
	TH_PRIOR_HIGHEST,
	TH_PRIOR_REALTIME,
	/*this is where real-time priorities stop*/
	TH_PRIOR_REALTIME_END=255
};

void TH_SetPriority(M4Thread *t, s32 priority);
/* get the current threadID */
u32 TH_GetID();

/*********************************************************************
					Mutex Object
**********************************************************************/
typedef struct tagM4Mutex M4Mutex;
typedef struct tagM4Mutex *LPMUTEX;

M4Mutex *NewMutex();
void MX_Delete(M4Mutex *mx);
void MX_V(M4Mutex *mx);
u32 MX_P(M4Mutex *mx);


/*********************************************************************
					Semaphore Object
**********************************************************************/
typedef struct tagM4Semaphore M4Sema;
typedef struct tagM4Semaphore *LPM4SEMA;


M4Sema *NewSemaphore(u32 MaxCount, u32 InitCount);
void SEM_Delete(M4Sema *sm);

/* return the number of previous count in the semaphore */
u32 SEM_Notify(M4Sema *sm, u32 NbRelease);
/*infinite wait on a semaphore */
void SEM_Wait(M4Sema *sm);
/* timely wait on a semaphore, return 0 if couldn't get control before TimeOut
	NOTE: On POSIX, the timeout is implemented via a Sleep() till the notif is done
	or an error occurs
*/
Bool SEM_WaitFor(M4Sema *sm, u32 TimeOut);


/*********************************************************************
					Socket Object
**********************************************************************/

#define MAX_IP_NAME_LEN	516

/* socket types */
#define SK_TYPE_UDP		0x01
#define SK_TYPE_TCP		0x02

/* socket status */
#define SK_STATUS_CREATE	0x01
#define SK_STATUS_BIND		0x02
#define SK_STATUS_CONNECT	0x03
#define SK_STATUS_LISTEN	0x04

typedef struct _tagSock M4Socket, *LPSOCKET;

M4Socket *NewSocket(u32 SocketType);
void SK_Delete(M4Socket *sock);

/* forces a reset of the socket buffer */
void SK_Reset(M4Socket *sock);
/* set the buffer size for the socket. If SendBuffer is 0, set the recieving buffer */
M4Err SK_SetBufferSize(M4Socket *sock, Bool SendBuffer, u32 NewSize);

/* set the blocking mode of a socket on or off. By default, sockets
are created in blocking mode (BSD style) */
M4Err SK_SetBlockingMode(M4Socket *sock, u32 NonBlockingOn);
/* binds the given socket to the specified port. If ReUse is true
this will enable reuse of ports on a single machine */
M4Err SK_Bind(M4Socket *sock, u16 PortNumber, Bool reUse);
/*connects a socket to a remote peer on a given port */
M4Err SK_Connect(M4Socket *sock, char *PeerName, u16 PortNumber);
/* send a buffer on the socket */
M4Err SK_Send(M4Socket *sock, unsigned char *buffer, u32 length);
/* fetch nb bytes on a socket and fill the buffer from startFrom */
M4Err SK_Receive(M4Socket *sock, unsigned char *buffer, u32 length, u32 startFrom, u32 *BytesRead);
/* make the specified socket listen. This socket MUST have been bound to a port before */
Bool SK_Listen(M4Socket *sock, u32 MaxConnection);
/* accept an incomming connection */
M4Err SK_Accept(M4Socket *sock, M4Socket **newConnection);

/* disable the Nable algo (aka set TCP_NODELAY) and set the KEEPALIVE on */
M4Err SK_SetServerMode(M4Socket *sock, Bool serverOn);

/* buffer must be MAX_IP_NAME_LEN long */
M4Err SK_GetHostName(char *buffer);
/* Get local IP for connected sockets (typically used for server after an ACCEPT */
M4Err SK_GetSocketIP(M4Socket *sock, char *buffer);
M4Err SK_GetLocalInfo(M4Socket *sock, u16 *Port, u32 *Familly);

/* get the remote address of a peer. Socket MUST be connected
buffer must be MAX_IP_NAME_LEN long */
M4Err SK_GetRemoteAddress(M4Socket *sock, char *buffer);

/* Connection-less sockets (UDP server side) to use with SendTo and RecieveFrom */
/* Set the remote address of a socket */
M4Err SK_SetRemoteAddress(M4Socket *sock, char *address);
/* Set the remote port for a socket */
M4Err SK_SetRemotePort(M4Socket *sock, u16 RemotePort);

/* send data to the specified host. If no host is specified (NULL), the
default host (SK_SetRemoteAddress ...) is used */
M4Err SK_SendTo(M4Socket *sock, unsigned char *buffer, u32 length, unsigned char *remoteHost, u16 remotePort);


/* Performs multicast BIND and JOIN */
M4Err SK_MulticastSetup(M4Socket *sock, char *multi_IPAdd, u16 MultiPortNumber, u32 TTL, Bool NoBind);
/* returns 1 if multicast address, 0 otherwise*/
u32 SK_IsMulticastAddress(char *multi_IPAdd);

/* send data with a max wait delay of Second - used for http / ftp sockets mainly*/
M4Err SK_SendWait(M4Socket *sock, unsigned char *buffer, u32 length, u32 Second );
/* recieve data with a max wait delay of Second - used for http / ftp sockets mainly*/
M4Err SK_ReceiveWait(M4Socket *sock, unsigned char *buffer, u32 length, u32 startFrom, u32 *BytesRead, u32 Second );

/*********************************************************************
					Socket Group Object
**********************************************************************/
#define SK_GROUP_READ		0x01
#define SK_GROUP_WRITE		0x02
#define SK_GROUP_ERROR		0x03

/* Socket Group for select(). The group is a collection of sockets
ready for reading / writing */
typedef struct _tagSockGroup SockGroup;
typedef struct _tagSockGroup *LPSOCKGROUP;
SockGroup *NewSockGroup();
void SKG_Delete(SockGroup *group);
void SKG_SetWatchTime(SockGroup *group, u32 DelayInS, u32 DelayInMicroS);

/* call this to add the socket into a group */
void SKG_AddSocket(SockGroup *group, M4Socket *sock, u32 GroupType);
/* call this to remove the socket from a group */
void SKG_RemoveSocket(SockGroup *group, M4Socket *sock, u32 GroupType);


/* call this after a SKG_Select to know if the socket is ready */
Bool SKG_IsSocketIN(SockGroup *group, M4Socket *sock, u32 GroupType);
/* select the socket(s) watched in this group. Return the number of pending socket
or 0 if none are ready */
u32 SKG_Select(SockGroup *group);


/*********************************************************************
					URL Manipulation
**********************************************************************/

/* our supported protocol types */
enum
{
	/*absolute path to file*/
	URL_TYPE_FILE = 0,
	/*relative URL*/
	URL_TYPE_RELATIVE ,
	/*any other URL*/
	URL_TYPE_ANY
};

/*gets protocol type*/
u32 URL_GetProtocolType(const char *pathName);
/*gets absolute file path - returned string must be freed by user*/
char *URL_GetAbsoluteFilePath(const char *pathName, const char *parentPath);
/*concatenates URL and gets back full URL - returned string must be freed by user*/
char *URL_Concatenate(const char *parentName, const char *pathName);


/*********************************************************************
					Base64 encoding / decoding
**********************************************************************/
u32 Base64_enc(unsigned char *in, u32 inSize, unsigned char *out, u32 outSize);
u32 Base64_dec(unsigned char *in, u32 inSize, unsigned char *out, u32 outSize);

/*********************************************************************
					Base16 encoding / decoding
**********************************************************************/
u32 Base16_enc(unsigned char *in, u32 inSize, unsigned char *out, u32 outSize);
u32 Base16_dec(unsigned char *in, u32 inSize, unsigned char *out, u32 outSize);

/*********************************************************************
					a simple string parser
**********************************************************************/
#define	CASE_SENSITIVE		0
#define CASE_INSENSITIVE	1
/*get string component 
	@Buffer: src string
	@start: start offset in src
	@SeparatorSet: separator characters used
	@Container, @ContainerSize: output
*/
s32 SP_GetComponent(unsigned char *Buffer, s32 Start, unsigned char *SeparatorSet, unsigned char *Container, s32 ContainerSize);
/*line delimeters checked: \r\n, \n and \r*/
s32 SP_GetOneLine(unsigned char	*Buffer, u32 Start, u32 Size, unsigned char *LineBuffer, u32 LineBufferSize);
/*locates pattern in buffer*/
s32 SP_FindPattern(unsigned char *Buffer, u32 Start, u32 Size, unsigned char *Pattern);



/*********************************************************************
			a simple conf file formatted as .ini of Win32
**********************************************************************/

typedef struct _iniFile *LPINIFILE;

/* load a ini file. If the file is not found the object is still created - filePath may be NULL*/
LPINIFILE NewIniFile(const char *filePath, const char *fileName);
/* delete and save the ini file if needed */
void IF_Delete(LPINIFILE iniFile);
/* return the curent key - do NOT delete it */
char *IF_GetKey(LPINIFILE iniFile, const char *secName, const char *keyName);
/* set a key value - create section and/or key if needed*/
M4Err IF_SetKey(LPINIFILE iniFile, const char *secName, const char *keyName, const char *keyValue);
/*get number of keys in a section*/
u32 IF_GetSectionCount(LPINIFILE iniFile);
/*get section - secIndex is 0-based*/
const char *IF_GetSectionName(LPINIFILE iniFile, u32 secIndex);
/*get number of keys in a section*/
u32 IF_GetKeyCount(LPINIFILE iniFile, const char *secName);
/*get key in a section - keyIndex is 0-based*/
const char *IF_GetKeyName(LPINIFILE iniFile, const char *secName, u32 keyIndex);


/*********************************************************************
					a generic plugin manager
**********************************************************************/

/*****************************************************
Each plugin must export the following functions:

	-- returns non zero if plugin handle interface , 0 otherwise-- 
	Bool QueryInterface(u32 interface_type);
	-- returns interface object -- 
	void *LoadInterface(u32 interface_type);
	-- destroy interface object -- 
	void ShutdownInterface(void *interface);

Each interface must begin with the interface macro def and shall assign its interface type. Ex:
struct {
	DECL_PLUGIN_INTERFACE

	extensions;
};

  so that each interface can be type-casted to the BaseInterface structure
*****************************************************/

typedef struct _tagPlugMan *LPPLUGMAN;

/*common plugin interface - 
- the HPLUG handle is private to the app and shall not be touched
*/
#define M4_DECL_PLUGIN_INTERFACE	\
	u32 InterfaceType;				\
	char *plugin_name;		\
	char *author_name;		\
	u32 version;				\
	void *HPLUG;					\

typedef struct
{
	M4_DECL_PLUGIN_INTERFACE
} BaseInterface;

#define M4_REG_PLUG(__plug, type, dr_name, author, vers) \
	__plug->InterfaceType = type;	\
	__plug->plugin_name = dr_name ? dr_name : "unknown";	\
	__plug->author_name = author ? author : "gpac distribution";	\
	__plug->version = vers ? vers : M4_VERSION_INT;	\
	
/*
	Plugin Manager Constructor in specified directory (ABSOLUTE PATH)
*/
LPPLUGMAN NewPluginManager(const unsigned char *directory, LPINIFILE cfg);
void PM_Delete(LPPLUGMAN pm);

/*refresh all plugins and load unloaded ones*/
u32 PM_RefreshPlugins(LPPLUGMAN pm);
/* return the number of available plugins*/
u32 PM_GetPluginsCount(LPPLUGMAN pm);
/* return the file name of the given plugin*/
const char *PM_GetFileName(LPPLUGMAN pm, u32 i);
/* get first available interface - return 0 if no interface present. otherwise interface is set to newly created interface*/
Bool PM_LoadInterface(LPPLUGMAN pm, u32 whichplug, u32 InterfaceFamily, void **interface_obj);
/* get interface on specified plugin file - return 0 if no interface present. otherwise interface is set to newly created interface*/
Bool PM_LoadInterfaceByName(LPPLUGMAN pm, const char *plug_name, u32 InterfaceFamily, void **interface_obj);
/* shutdown interface */
M4Err PM_ShutdownInterface(void *interface_obj);
/*get/set/enum option for plugin - plugins should use these functions in order to use only one config file*/
char *PMI_GetOpt(void *interface_obj, const char *secName, const char *keyName);
M4Err PMI_SetOpt(void *interface_obj, const char *secName, const char *keyName, const char *keyValue);
/*same as above but callable by plugins to load other plugins*/
u32 PMI_GetPluginsCount(BaseInterface *parent);
Bool PMI_LoadInterface(BaseInterface *parent, u32 whichplug, u32 InterfaceFamily, void **interface_obj);
Bool PMI_LoadInterfaceByName(BaseInterface *parent, const char *plug_name, u32 InterfaceFamily, void **interface_obj);


/* Random Generator inits (and reset if needed)*/
void M4_RandInit(Bool Reset);
/*get random*/
u32 M4_Rand();


/*gets UTC time since midnight Jan 1970*/
void M4_GetUTCTimeSince1970(u32 *sec, u32 *msec);
/*Get NTP time in sec + fractional side ( in 1 / (1<<32 - 1) sec units)*/
void M4_GetNTP(u32 *sec, u32 *frac);
/*get reduced NTP time on 32 bits (used in RTP a lot)*/
u32 M4_GetNTP32(u32 sec, u32 frac);

/*gets current user name*/
void M4_GetUserName(char *buf, u32 buf_size);


/*enumerate direcory content. 
	@enum_directory: if set, only directories are enumerated. Otherwise only files are
	@enum_dir_item: callback function for enumeration. if function returns TRUE enumeration is aborted. item_path is the full
	address of the enumerated item (Path + name)
	@cbck: private user callback data
*/
M4Err DIR_Enum(const char *dir, Bool enum_directory, Bool (*enum_dir_item)(void *cbck, char *item_name, char *item_path), void *cbck);


/*UTF-8 basic routines*/
/*converts wide-char string to multibyte string - returns (-1) if error. set @srcp to next char to be
converted if not enough space*/
size_t utf8_wcstombs(char* dest, size_t len, const unsigned short** srcp);
/*converts UTF8 string to wide char string - returns (-1) if error. set @srcp to next char to be
converted if not enough space*/
size_t utf8_mbstowcs(unsigned short* dest, size_t len, const char** srcp);
/*returns size in characters of the wide-char string*/
size_t utf8_wcslen(const unsigned short *s);


/**/
#ifndef M4_READ_ONLY
FILE *M4NewTMPFile();
#endif

/* the DeleteFile function */
void M4_DeleteFile(char *fileName);

/* the time functions*/
void M4_InitClock();
void M4_StopClock();
u32 M4_GetSysClock();


#ifdef __cplusplus
}
#endif


#endif		/*__M4_TOOLS_H_*/

