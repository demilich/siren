#pragma once


#ifdef __cplusplus
#  define SIREN__NOEXCEPT noexcept
extern "C" {
#else
#  define SIREN__NOEXCEPT
#endif

#ifdef _FCNTL_H
int siren_open(const char *, int, ...) SIREN__NOEXCEPT;
int siren_fs_open(const char *, int, ...) SIREN__NOEXCEPT;
int siren_fcntl(int, int, ...) SIREN__NOEXCEPT;
#endif

#ifdef _UNISTD_H
int siren_pipe(int [2]) SIREN__NOEXCEPT;
int siren_pipe2(int [2], int) SIREN__NOEXCEPT;
ssize_t siren_read(int, void *, size_t) SIREN__NOEXCEPT;
ssize_t siren_fs_read(int, void *, size_t) SIREN__NOEXCEPT;
ssize_t siren_write(int, const void *, size_t) SIREN__NOEXCEPT;
ssize_t siren_fs_write(int, const void *, size_t) SIREN__NOEXCEPT;
ssize_t siren_readv(int, const struct iovec *, int) SIREN__NOEXCEPT;
ssize_t siren_fs_readv(int, const struct iovec *, int) SIREN__NOEXCEPT;
ssize_t siren_writev(int, const struct iovec *, int) SIREN__NOEXCEPT;
ssize_t siren_fs_writev(int, const struct iovec *, int) SIREN__NOEXCEPT;
int siren_close(int) SIREN__NOEXCEPT;
int siren_fs_close(int) SIREN__NOEXCEPT;
int siren_usleep(useconds_t) SIREN__NOEXCEPT;
#endif

#ifdef _SYS_SOCKET_H
int siren_socket(int, int, int) SIREN__NOEXCEPT;
int siren_getsockopt(int, int, int, void *, socklen_t *) SIREN__NOEXCEPT;
int siren_setsockopt(int, int, int, const void *, socklen_t) SIREN__NOEXCEPT;
int siren_accept(int, struct sockaddr *, socklen_t *) SIREN__NOEXCEPT;
int siren_accept4(int, struct sockaddr *, socklen_t *, int) SIREN__NOEXCEPT;
int siren_connect(int, const struct sockaddr *, socklen_t) SIREN__NOEXCEPT;
ssize_t siren_recv(int, void *, size_t, int) SIREN__NOEXCEPT;
ssize_t siren_send(int, const void *, size_t, int) SIREN__NOEXCEPT;
ssize_t siren_recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *) SIREN__NOEXCEPT;
ssize_t siren_sendto(int, const void *, size_t, int, const struct sockaddr *
                     , socklen_t) SIREN__NOEXCEPT;
#endif

#ifdef _NETDB_H
int siren_getaddrinfo(const char *, const char *, const struct addrinfo *
                      , struct addrinfo **) SIREN__NOEXCEPT;
int siren_getnameinfo(const struct sockaddr *, socklen_t, char *, socklen_t, char *, socklen_t
                      , int) SIREN__NOEXCEPT;
#endif

#ifdef _SYS_POLL_H
int siren_poll(struct pollfd *, nfds_t, int) SIREN__NOEXCEPT;
#endif

#ifdef __cplusplus
} // extern "C"
#endif
#undef SIREN__NOEXCEPT
