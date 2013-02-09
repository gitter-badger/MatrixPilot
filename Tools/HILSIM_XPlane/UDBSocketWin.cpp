//
//  UDBSocketWin.c
//  MatrixPilot-SIL
//
//  Created by Ben Levitt on 2/1/13.
//  Copyright (c) 2013 MatrixPilot. All rights reserved.
//

#include "UDBSocket.h"

#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
//#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <arpa/inet.h>
//#include <netdb.h>
#include <errno.h>
#include <string.h>


//#include <termios.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>


#define LOCALHOST_IP "127.0.0.1"


struct SILSocket_t {
	HANDLE				hComms;
	
	int					fd;
	struct sockaddr_in	si_other;
	SILSocketType		type;
	long				UDP_port;
	char*				serial_port;
	long				serial_baud;
} SILSocket_t;


SILSocket SILSocket_init(SILSocketType type, long UDP_port, char *serial_port, long serial_baud)
{
	SILSocket newSocket = (SILSocket)malloc(sizeof(SILSocket_t));
	if (!newSocket) {
		return NULL;
	}
	
	memset((char *)newSocket, 0, sizeof(SILSocket_t));
	newSocket->type = type;
	newSocket->UDP_port = UDP_port;
	newSocket->serial_port = (serial_port) ? strdup(serial_port) : NULL;
	newSocket->serial_baud = serial_baud;
	
	switch (newSocket->type) {
		case SILSocketUDPClient:
		{
			if ((newSocket->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
				perror("socket() failed");
				free(newSocket);
				return NULL;
			}
			
			int on = 1;
			if (ioctl(newSocket->fd, FIONBIO, (char *)&on) < 0)
			{
				perror("ioctl() failed");
				SILSocket_close(newSocket);
				return NULL;
			}
			
			memset((char *) &newSocket->si_other, 0, sizeof(newSocket->si_other));
			newSocket->si_other.sin_family = AF_INET;
			newSocket->si_other.sin_port = htons(newSocket->UDP_port);
			if (inet_aton(LOCALHOST_IP, &newSocket->si_other.sin_addr) == 0) {
				fprintf(stderr, "inet_aton() failed\n");
				SILSocket_close(newSocket);
				return NULL;
			}
			
			SILSocket_write(newSocket, (unsigned char*)"", 0); // Initiate connection
			
			break;
		}
			
		case SILSocketUDPServer:
		{
			struct sockaddr_in si_me;
			
			if ((newSocket->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
				perror("socket");
				free(newSocket);
				return NULL;
			}
			
			int on = 1;
			if (ioctl(newSocket->fd, FIONBIO, (char *)&on) < 0)
			{
				perror("ioctl() failed");
				SILSocket_close(newSocket);
				return NULL;
			}
			
			newSocket->si_other.sin_family = AF_INET;
			
			memset((char *) &si_me, 0, sizeof(si_me));
			si_me.sin_family = AF_INET;
			si_me.sin_port = htons(newSocket->UDP_port);
			si_me.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(newSocket->fd, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
				perror("bind");
				SILSocket_close(newSocket);
				return NULL;
			}
			break;
		}
		case SILSocketSerial:
		{
			DCB Dcb;
			COMMTIMEOUTS CommTimeouts;
			DWORD dwRetFlag;
			char ErrorString[80];
			
			if (newSocket->hComms == 0)
			{
				string port = "\\\\.\\";
				port.append(newSocket->serial_port);
				
				newSocket->hComms = CreateFile(port.data(),
									GENERIC_READ | GENERIC_WRITE,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
									NULL);
				
				if (newSocket->hComms == INVALID_HANDLE_VALUE)
				{
					sprintf(ErrorString, "Could not open comm port");
					ShowMessage(ErrorString);
					LoggingFile.mLogFile << "Could not open Com Port";
					LoggingFile.mLogFile << endl;
					return NULL;
				}
				else
				{
					
					dwRetFlag = GetCommState(newSocket->hComms, &Dcb);
					
					if (!dwRetFlag)
					{
						sprintf(ErrorString, "GetCommState Error = %d", GetLastError());
						ShowMessage(ErrorString);
						return NULL;
					}
					
					Dcb.DCBlength = sizeof(Dcb);
					
					Dcb.BaudRate = newSocket->serial_baud;
					
					Dcb.ByteSize = 8;
					Dcb.Parity = NOPARITY;
					Dcb.StopBits = ONESTOPBIT;
					Dcb.fTXContinueOnXoff = true;
					
					Dcb.fOutxCtsFlow = false;//true;                  // disable CTS output flow control
					Dcb.fOutxDsrFlow = false;                  // disable DSR output flow control
					Dcb.fDtrControl = DTR_CONTROL_HANDSHAKE  /*DTR_CONTROL_DISABLE DTR_CONTROL_ENABLE*/;
					Dcb.fDsrSensitivity = false;               // enable DSR sensitivity
					
					Dcb.fOutX = false;                        // disable XON/XOFF out flow control
					Dcb.fInX = false;                         // disable XON/XOFF in flow control
					Dcb.fErrorChar = false;                   // disable error replacement
					Dcb.fNull = false;                        // disable null stripping
					Dcb.fRtsControl = RTS_CONTROL_HANDSHAKE	  /* RTS_CONTROL_ENABLE  RTS_CONTROL_DISABLE*/;   //  enable RTS line
					Dcb.fAbortOnError = true;                 // don't abort reads/writes on error
					
					dwRetFlag = SetCommState(newSocket->hComms, &Dcb);
					if (!dwRetFlag)
					{
						sprintf(ErrorString, "SetCommState Error = %d", GetLastError());
						ShowMessage(ErrorString);
					}
					
					dwRetFlag = GetCommTimeouts(newSocket->hComms, &CommTimeouts);
					if (!dwRetFlag)
					{
						sprintf(ErrorString, "GetCommTimeouts Error = %d", GetLastError());
						ShowMessage(ErrorString);
					}
					
					CommTimeouts.ReadIntervalTimeout         = -1;    //Don't use interval timeouts
					CommTimeouts.ReadTotalTimeoutMultiplier  = 0;			//Don't use multipliers
					CommTimeouts.ReadTotalTimeoutConstant    = 0;			//150ms total read timeout
					CommTimeouts.WriteTotalTimeoutMultiplier = 0;			//Don't use multipliers
					CommTimeouts.WriteTotalTimeoutConstant   = 0;			//2200ms total write timeout
					
					dwRetFlag = SetCommTimeouts(newSocket->hComms, &CommTimeouts);
					if (!dwRetFlag)
					{
						sprintf(ErrorString, "SetCommTimeouts Error = %d", GetLastError());
						ShowMessage(ErrorString);
					}
				}
			}
			else
			{
				ShowMessage("Comm port already open");
			}
			
			break;
		}
		default:
			break;
	}
	
	return newSocket;
}

void SILSocket_close(SILSocket socket)
{
	switch (socket->type) {
		case SILSocketUDPClient:
		case SILSocketUDPServer:
		{
			close(socket->fd);
			if (socket->serial_port) free(socket->serial_port);
			free(socket);
			break;
		}
		case SILSocketSerial:
		{
			if (socket->hComms != 0)
			{
				CloseHandle(socket->hComms);
				socket->hComms = 0;
			}
			else
			{
				ShowMessage("Comm port already closed");
			}
			break;
		}
		default:
			break;
	}
}

int SILSocket_read(SILSocket socket, unsigned char *buffer, int bufferLength)
{
	switch (socket->type) {
		case SILSocketUDPClient:
		case SILSocketUDPServer:
		{
			struct sockaddr_in from;
			socklen_t fromLength = sizeof(from);
			
			int received_bytes = (int)recvfrom(socket->fd, (char*)buffer, bufferLength, 0,
											   (struct sockaddr*)&from, &fromLength);
			
			if ( received_bytes < 0 ) {
				if (errno != EWOULDBLOCK) {
					SILSocket_close(socket);
					return -1;
				}
			}
			
			if (socket->type == SILSocketUDPServer) {
				socket->si_other.sin_port = from.sin_port;
				socket->si_other.sin_addr = from.sin_addr;
			}
			
			if (received_bytes < 0) return 0;
			
			return (int)received_bytes;
		}
		case SILSocketSerial:
		{
			unsigned long bytesTransferred = 0;
			DWORD dwRetFlag;
			char ErrorString[80];
			
			if(socket->hComms != INVALID_HANDLE_VALUE)
			{
				// Read the data from the serial port.
				if (ReadFile(socket->hComms, buffer, bufferLength, &bytesTransferred, 0) == 0)
				{
					COMSTAT comStat;
					unsigned long   dwErrors;
					
					sprintf(ErrorString, "ReadFile Error = %d", GetLastError());
					ShowMessage(ErrorString);
					ClearCommError(socket->hComms, &dwErrors, &comStat);
					SILSocket_close(socket);
					return -1;
				}
			}
			return bytesTransferred;
		}
		default:
			break;
	}
	return -1;
}


int SILSocket_write(SILSocket socket, unsigned char *data, int dataLength)
{
	switch (socket->type) {
		case SILSocketUDPClient:
		case SILSocketUDPServer:
		{
			if (socket->type == SILSocketUDPServer && socket->si_other.sin_port == 0) {
				SILSocket_read(socket, NULL, 0);
				if (socket->si_other.sin_port == 0) {
					return 0;
				}
			}
			
			int bytesWritten = (int)sendto(socket->fd, data, dataLength, 0, (const struct sockaddr*)&socket->si_other, sizeof(socket->si_other));
			if (bytesWritten < 0) {
				perror("sendto()");
				SILSocket_close(socket);
				return -1;
			}
			return bytesWritten;
		}
		case SILSocketSerial:
		{
			unsigned long bytesWritten;
			DWORD dwRetFlag;
			if (socket->hComms != 0) {
				dwRetFlag = WriteFile(socket->hComms, data, dataLength, &bytesWritten, NULL);
				if (!dwRetFlag) {
					sprintf(ErrorString, "WriteFile Error = %d", GetLastError());
					ShowMessage(ErrorString);
					SILSocket_close(socket);
					return -1;
				}
			}
			else {
				ShowMessage("Comm port not open");
			}
			return bytesWritten;
		}
		default:
			break;
	}
	return -1;
}