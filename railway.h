/*
 * RAILWAY v0.1
 *
 * Copyright (c) 2015, Jonas Weber
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*/

#ifndef RAILWAY_H
#define RAILWAY_H 

#if defined(__has_include)
// clang compatible compiler

#if __has_include(<sys/un.h>)
#define RAILWAY_HAS_UNIX_SOCKET
#endif

#if __has_include(<winsock2.h>)
#define RAILWAY_USE_WINSOCK
#endif

#else

// lets guess

#if defined(__MINGW32__) || defined(_MSC_VER)
#define RAILWAY_USE_WINSOCK
#else
#define RAILWAY_HAS_UNIX_SOCKET
#endif

#endif

#ifdef RAILWAY_USE_WINSOCK
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "advapi32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef RAILWAY_HAS_UNIX_SOCKET
#include <sys/un.h>
#endif

#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#endif

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace railway {
  /**
   * An abstract class representing a network socket.
   *
   * This class has two constructors: One accepting
   * a path, and one accepting a hostname plus port.
   * Note that the constructor with the path only works
   * on systems that support unix filesystem sockets. The
   * constructor throws an exception on Windows platforms.
   * Support for unix sockets can be queried with warp::Socket::supports_unix_sockets.
   *
   * \author Jonas Weber
   * \since 0.1
   */
  class track {
    private:
      /**
       * Convenience method to simplify throw exceptions (of type std::string)
       * including the error text from strerror().
       *
       * The thrown string is of the form: <code>[prefix]: [error]</code>
       *
       * @param prefix The string to show before the actual error
       * @return Never.
       */
      static inline void throw_error_message(const std::string& prefix) {
        throw prefix + ": " + strerror(errno);
      }

      class Socket_RAII {
        public:
#ifdef RAILWAY_USE_WINSOCK
          using socket_t = SOCKET;
          static const socket_t INVALID = INVALID_SOCKET;
#else
          using socket_t = int;
          static const socket_t INVALID = -1;
#endif
        private:
          socket_t sock;
        public:
          Socket_RAII() : sock(0) {}

          Socket_RAII(int domain, int type, int protocol) {
            sock = socket(domain, type, protocol);
            if (sock == INVALID) {
              throw_error_message("Unable to create socket");
            }
          }

          ~Socket_RAII() {
            this->close();
          }
 
          // Copy constructor is not supported
          Socket_RAII(const Socket_RAII& other) = delete;

          Socket_RAII(Socket_RAII&& other) noexcept : sock(other.sock){
            other.sock = 0;
          }

          Socket_RAII& operator= (const Socket_RAII& other) = delete;
          Socket_RAII& operator= (Socket_RAII&& other) noexcept {
            std::swap(sock, other.sock);
            return *this;
          }

          operator socket_t () const {
            return sock;
          }

          bool is_valid() const {
            return sock != INVALID;
          }

          void close() {
            if (is_valid()) {
#ifdef RAILWAY_USE_WINSOCK
              closesocket(sock);
#else
              ::close(sock);
#endif
              sock = INVALID;
            }
          }
      };
      Socket_RAII sock;

#ifdef RAILWAY_USE_WINSOCK
      class WSA_RAII {
        private:
          WSADATA wsa_data;
        public:
          WSA_RAII() {
            int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
            if (result != 0) {
              WSACleanup();
              throw std::string("Unable to initialize Winsock");
            }
          }
          ~WSA_RAII() {
            WSACleanup();
          }
      };

      WSA_RAII winsocks;
#endif

      class AddrInfo_RAII {
        private:
          struct addrinfo* addr;
        public:
          AddrInfo_RAII(const std::string& host, const std::string& port) {
            int result = getaddrinfo(host.c_str(), port.c_str(), NULL, &addr);
            if (result != 0) {
              throw std::string("Unable to get address info: ") + gai_strerror(result);
            }
          }
          AddrInfo_RAII(const AddrInfo_RAII& other) = delete;
          AddrInfo_RAII(AddrInfo_RAII&& other) = delete;
          ~AddrInfo_RAII() {
            if (addr) {
              freeaddrinfo(addr);
            }
          }
          AddrInfo_RAII& operator=(const AddrInfo_RAII& other) = delete;
          AddrInfo_RAII& operator=(AddrInfo_RAII&& other) = delete;
          operator struct addrinfo* () {
            return addr;
          }
      };


    public:
      enum Mode {
        Connect
      };
  /**
   * Tells whether the system supports unix sockets.
   *
   * \return true, if it supports them.
   */
    static bool supports_unix_sockets() {
#if defined(RAILWAY_HAS_UNIX_SOCKET)
      return true;
#else
      return false;
#endif
    }

      /**
       * Constructs a Socket using a filesystem path to connect to
       * a unix socket.
       *
       * \param unix_path The filename to use, for example <code>/var/lib/service.sock</code>.
       */
      track(const std::string& unix_path, Mode mode = Connect) {
#if defined(RAILWAY_HAS_UNIX_SOCKET)
        struct sockaddr_un un_addr;
        memset(&un_addr, 0, sizeof(struct sockaddr_un));

        const size_t UNIX_PATH_MAX = sizeof(un_addr.sun_path);
        assert(UNIX_PATH_MAX == 108);

        if (unix_path.size() >= UNIX_PATH_MAX) {
          throw std::string("Path name too long");
        }

        sock = Socket_RAII { AF_UNIX, SOCK_STREAM, 0 };

        un_addr.sun_family = AF_UNIX;
        strncpy(un_addr.sun_path, unix_path.c_str(), UNIX_PATH_MAX);
        un_addr.sun_path[UNIX_PATH_MAX - 1] = '\0';

        switch (mode) {
          case Connect:
            int result = ::connect(sock, (const sockaddr*) &un_addr, sizeof(struct sockaddr_un));
            if (result != 0) {
              throw_error_message("Socket not connectable");
            }
            break;
        }
#else
        throw std::string("Unix socket not supported on this platform");
#endif
      }
      track(const std::string& host, const std::string& port, Mode mode = Connect) {
        AddrInfo_RAII addr(host, port);

        for (const struct addrinfo* runner = addr; runner != (const struct addrinfo*) 0; runner = runner->ai_next) {
          sock = Socket_RAII {runner->ai_family, runner->ai_socktype, runner->ai_protocol};
          size_t result = ::connect(sock, runner->ai_addr, runner->ai_addrlen);
          if (result == 0) {
            break;
          }
        }

        if (!sock.is_valid()) {
          throw_error_message("Unable to connect");
        }
      }

      int read(char* out_buf, int nBytes) {
        return ::recv(sock, out_buf, nBytes, 0);
      }

      int write(char* in_buf, int nBytes) {
        return ::send(sock, in_buf, nBytes, 0);
      }
  };
}


/**
 * \mainpage <code>Railway</code> - Transmit via Sockets, portably
 *
 * Rationale
 * ---------
 *
 * Railway provides a simple interface to a common task in software:
 * Communication across a network infrastructure.  Unfortunately, different
 * platforms provide different APIs to access the underlying networking stacks.
 * <code>Railway</code> solves this by unifying the different APIs under one
 * that works portably on many devices.
 *
 */

#endif
