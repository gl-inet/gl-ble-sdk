/*****************************************************************************
 * @file  uart.c
 * @brief UART implementation for POSIX environment
 *******************************************************************************
 Copyright 2020 GL-iNet. https://www.gl-inet.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "gl_uart.h"

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static struct {
  uint32_t cbaud;
  uint32_t nspeed;
} speedTab[] = {
  { B300, 300    },
  { B1200, 1200   },
  { B2400, 2400   },
  { B4800, 4800   },
  { B9600, 9600   },
  { B19200, 19200  },
  { B38400, 38400  },
  { B57600, 57600  },
  { B115200, 115200 },
  { 0, 0 }
};

static int32_t serialHandle = -1;
static struct termios origTTYAttrs;

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/
static int32_t uartOpenSerial(int8_t* device, uint32_t bps, uint32_t dataBits, uint32_t parity,
                              uint32_t stopBits, uint32_t rtsCts, uint32_t xOnXOff,
                              int32_t timeout);
static int32_t uartCloseSerial(int32_t handle);

/***************************************************************************************************
   Public Function Definitions
 **************************************************************************************************/
int32_t uartOpen(int8_t* port, uint32_t baudRate, uint32_t rtsCts, int32_t timeout)
{
  uint8_t buf[4];

  serialHandle = uartOpenSerial(port, baudRate, 8, 0, 1, rtsCts, 0, timeout);

  if (-1 == serialHandle) {
    return -1;
  }

  // Flush all accumulated data in target
  usleep(50000);
  while (uartRxNonBlocking(4, buf) == 4) {
  }

  return serialHandle;
}

int32_t uartClose(void)
{
  return uartCloseSerial(serialHandle);
}

int32_t uartCacheClean(void)
{  
  uint8_t buf[4];
  while (uartRxNonBlocking(4, buf) == 4) {
  }

  return 0;
}


int32_t uartRx(uint32_t dataLength, uint8_t* data)
{
  /** The amount of bytes read. */
  size_t dataRead;
  /** The amount of bytes still needed to be read. */
  size_t dataToRead = dataLength;

  if (serialHandle == -1) {
    return -1;
  }

  while (dataToRead) {
    dataRead = read(serialHandle, (void*)data, dataToRead);
    if (-1 == dataRead) {
      if (EAGAIN == errno) {
        continue;
      } else {
        return -1;
      }
    } else {
      dataToRead -= dataRead;
      data += dataRead;
    }
  }
  return (int32_t)dataLength;
}

int32_t uartRxNonBlocking(uint32_t dataLength, uint8_t* data)
{
  /** The amount of bytes read. */
  size_t dataRead;

  if (serialHandle == -1) {
    return -1;
  }

  dataRead = read(serialHandle, (void*)data, (size_t)dataLength);
  if (-1 == dataRead) {
    return -1;
  }

  return (int32_t)dataRead;
}

int32_t uartRxPeek(void)
{
  int32_t bytesInBuf;

  if (serialHandle == -1) {
    return -1;
  }

  if (-1 == ioctl(serialHandle, FIONREAD, (int*)&bytesInBuf)) {
    return -1;
  }

  return bytesInBuf;
}

int32_t uartTx(uint32_t dataLength, uint8_t* data)
{
  /** The amount of bytes written. */
  size_t dataWritten;
  /** The amount of bytes still needed to be written. */
  size_t dataToWrite = dataLength;

  if (serialHandle == -1) {
    return -1;
  }

  while (dataToWrite) {
    dataWritten = write(serialHandle, (void*)data, dataToWrite);
    if (-1 == dataWritten) {
      if (EAGAIN == errno) {
        continue;
      } else {
        return -1;
      }
    } else {
      dataToWrite -= dataWritten;
      data += dataWritten;
    }
  }

  return (int32_t)dataLength;
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Open a serial port.
 *  \param[in] device Serial Port number.
 *  \param[in] bps Baud Rate.
 *  \param[in] dataBits Number of databits.
 *  \param[in] parity Parity bit used.
 *  \param[in] stopBits Stop bits used.
 *  \param[in] rtsCts Hardware handshaking used.
 *  \param[in] xOnXOff Software Handshaking used.
 *  \param[in] timeout Block until a character is received or for timeout milliseconds. If
 *             timeout < 0, block until character is received, there is no timeout.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
static int32_t uartOpenSerial(int8_t* device, uint32_t bps, uint32_t dataBits, uint32_t parity,
                              uint32_t stopBits, uint32_t rtsCts, uint32_t xOnXOff, int32_t timeout)
{
  uint32_t i;
  int32_t serial = -1;
  struct termios ttyAttrs = { 0 };

  /* Check if baud rate is supported. Return -1 if not. */
  for (i = 0; speedTab[i].nspeed != 0; i++) {
    if (bps == speedTab[i].nspeed) {
      break;
    }
  }
  if (speedTab[i].nspeed == 0) {
    fprintf(stderr, "Baud rate not supported %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }

  /* Open the serial port read/write, with no controlling terminal, and don't wait for a
   * connection. The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking. */
  serial = open((char*)device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (serial == -1) {
    fprintf(stderr, "Error opening serial port %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }

  /* Clean last time recevice message */
  if(-1 == tcflush(serial, TCIFLUSH))
  {
    fprintf(stderr, "Error clean serial port message %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
  }


  /* Note that open() follows POSIX semantics: multiple open() calls to the same file will succeed
   * unless the TIOCEXCL ioctl is issued. This will prevent additional opens except by root-owned
   * processes. */
  if (ioctl(serial, TIOCEXCL) == -1) {
    fprintf(stderr, "Error setting TIOCEXCL on %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }

  /* Get the current options and save them so we can restore the default settings later. */
  if (tcgetattr(serial, &origTTYAttrs) == -1) {
    fprintf(stderr, "Error getting tty attributes %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }
  /* The serial port attributes such as timeouts and baud rate are set by modifying the termios
   * structure and then calling tcsetattr to cause the changes to take effect. Note that the changes
   * will not take effect without the tcsetattr() call. */
  ttyAttrs = origTTYAttrs;

  /* Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block. */
  if (fcntl(serial, F_SETFL, 0) == -1) {
    fprintf(stderr, "Error clearing O_NONBLOCK %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }

  /* Configure baud rate. */
  if (cfsetspeed(&ttyAttrs, speedTab[i].cbaud) == -1) {
    fprintf(stderr, "Error setting baud rate %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }

  /* Set raw input (non-canonical) mode. */
  cfmakeraw(&ttyAttrs);

  /* Input modes. */
  /* If this bit is set, break conditions are ignored. */
  ttyAttrs.c_iflag = IGNBRK;

  /* Configure software flow control. */
  if (xOnXOff) {
    ttyAttrs.c_iflag |= IXON | IXOFF;
  } else {
    ttyAttrs.c_iflag &= ~(IXON | IXOFF);
  }

  /* Output modes. */
  ttyAttrs.c_oflag = 0;

  /* Control modes. */
  /* Ignore modem control lines. */
  ttyAttrs.c_cflag |= CLOCAL;
  /* Enable receiver. */
  ttyAttrs.c_cflag |= CREAD;

  /* Configure word length. Any number other than 5, 6 or 7 defaults to 8. */
  if (dataBits == 5) {
    ttyAttrs.c_cflag |= CS5;
  } else if (dataBits == 6) {
    ttyAttrs.c_cflag |= CS6;
  } else if (dataBits == 7) {
    ttyAttrs.c_cflag |= CS7;
  } else {
    ttyAttrs.c_cflag |= CS8;
  }

  /* Configure parity. Any number other than 1 or 2 defaults to 0. */
  if (parity == 1) {
    ttyAttrs.c_cflag |= PARENB | PARODD;
  } else if (parity == 2) {
    ttyAttrs.c_cflag |= PARENB;
  } else {
    /* No parity. */
    ttyAttrs.c_cflag &= ~PARENB;
  }

  /* Configure number of stop bits. Any number other than 2 defaults to 1. */
  if (stopBits == 2) {
    ttyAttrs.c_cflag |= CSTOPB;
  } else {
    /* 1 stop bit. */
    ttyAttrs.c_cflag &= ~CSTOPB;
  }

  /* Configure hardware flow control. */
  if (rtsCts) {
    ttyAttrs.c_cflag |= CRTSCTS;
  } else {
    ttyAttrs.c_cflag &= ~CRTSCTS;
  }

  /* Local modes. */
  ttyAttrs.c_lflag = 0;

  /* Special characters. */
  /* VMIN=0, VTIME=0: if data is available, read() returns immediately, with the lesser of the
   *                  number of bytes available, or the number of bytes requested. If no data is
   *                  available, read() returns 0.
   * VMIN=0, VTIME>0: VTIME specifies the limit for a timer in tenth of a second. The timer is
   *                  started when read() is called. read() returns either when at least one byte
   *                  of data is available, or when the timer expires. If the timer expires
   *                  without any input becoming available, read() returns 0.
   * VMIN>0, VTIME=0: read() blocks until the lesser of MIN bytes or the number of bytes requested
   *                  are available, and returns the lesser of these two values.
   * VMIN>0, VTIME>0: VTIME specifies the limit for a timer in tenth of a second. Once an initial
   *                  byte of input becomes available, the timer is restarted after each further
   *                  byte is received. read(2) returns either when the lesser of the number of
   *                  bytes requested or MIN byte have been read, or when the inter-byte timeout
   *                  expires. Because the timer is only started after the initial byte becomes
   *                  available, at least one byte will be read. */
  if (timeout < 0) {
    /* Block until character is received. No timeout configured. */
    ttyAttrs.c_cc[VMIN] = 1;
    ttyAttrs.c_cc[VTIME] = 0;
  } else {
    /* Block until character is received or timer expires. */
    ttyAttrs.c_cc[VMIN] = 0;
    ttyAttrs.c_cc[VTIME] = (cc_t)(timeout / 100); //Warning: Overflow may result
  }

  /* Cause the new options to take effect immediately. */
  if (tcsetattr(serial, TCSANOW, &ttyAttrs) == -1) {
    fprintf(stderr, "Error setting tty attributes %s - %s(%d).\n",
            (char*)device,
            strerror(errno), errno);
    goto error;
  }

  /* Success */
  return serial;

  /* Failure */
  error:
  if (serial != -1) {
    close(serial);
  }

  return -1;
}

/* Close a serial port. Return 0 on success, -1 on failure. */
static int32_t uartCloseSerial(int32_t handle)
{
  int32_t ret;

  if ((ret = close(handle))) {
    fprintf(stderr, "Error opening serial port - %s(%d).\n", strerror(errno), errno);
  }

  return ret;
}

int32_t setRtsStu(char stu)
{
  int32_t status;

  if (ioctl(serialHandle, TIOCMGET, &status) == -1)
  {
    goto error;
  }

  if (!stu)
  {
    status |= TIOCM_RTS;
  }
  else
  {
    status &= ~TIOCM_RTS;
  }

  if (ioctl(serialHandle, TIOCMSET, &status) == -1)
  {
    goto error;
  }

  return 0;

/* Failure */
error:
  return -1;
}

int32_t setDtrStu(char stu)
{
  int32_t status;

  if (ioctl(serialHandle, TIOCMGET, &status) == -1)
  {
    goto error;
  }

  if (!stu)
  {
    status |= TIOCM_DTR;
  }
  else
  {
    status &= ~TIOCM_DTR;
  }

  if (ioctl(serialHandle, TIOCMSET, &status) == -1)
  {
    goto error;
  }

  return 0;

/* Failure */
error:
  return -1;
}

int32_t getDsrStu(void)
{
  int32_t status;

  if (ioctl(serialHandle, TIOCMGET, &status) == -1)
  {
    goto error;
  }

  status &= TIOCM_DSR;

  if (status == TIOCM_DSR)
    return 1;
  else
    return 0;

/* Failure */
error:
  return -1;
}

int32_t getCtsStu(void)
{
  int32_t status;

  if (ioctl(serialHandle, TIOCMGET, &status) == -1)
  {
    goto error;
  }

  status &= TIOCM_CTS;

  if (status == TIOCM_CTS)
    return 1;
  else
    return 0;

/* Failure */
error:
  return -1;
}
