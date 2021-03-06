#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

ssize_t readn(int fd, void * buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char *)buf;

	while (nleft > 0)
	{
		if ((nread = read(fd, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}

		else if (nread == 0) //peer closed the conn
		{
			return count - nleft;
		}

		bufp += nread;
		nleft -= nread;
	}

	return count;
}

ssize_t writen(int fd, const void * buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwritten;
	char *bufp = (char *)buf;

	while (nleft > 0)
	{
		if ((nwritten = write(fd, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			return -1;
		}
		else if (nwritten == 0)
		{
			continue;
		}

		bufp += nwritten;
		nleft -= nwritten;
	}

	return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	while (1)
	{
		int ret = recv(sockfd, buf, len, MSG_PEEK); // MSG_PEEK: read but does not purge what was read from the buf
		if (ret == -1 && errno == EINTR)
			continue;
		return ret;
	}
}

// read till '\n', max maxline characters
// return <0: failed, 0: peer closed, >0: success
ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = buf;
	int nleft = maxline;
	int count = 0;

	while (1)
	{
		ret = recv_peek(sockfd, bufp, nleft);
		if (ret < 0)
			return ret; // failed 
		else if (ret == 0)
			return ret; // peer closed the connection

		nread = ret;
		int i;
		for (i = 0; i < nread; i++)
		{
			// look for '\n', if found, return (include the '\n')
			if (bufp[i] == '\n')
			{
				ret = readn(sockfd, bufp, i + 1);
				if (ret != i + 1)
				{
					exit(EXIT_FAILURE);
				}

				return ret + count;
			}
		}

		// should never more that I want(exceed maxline)
		if (nread > nleft)
		{
			exit(EXIT_FAILURE);
		}

		nleft -= nread;
		ret = readn(sockfd, bufp, nread);
		if (ret != nread) // should return exactly what I ask
		{
			exit(EXIT_FAILURE);
		}

		bufp += nread;
		count += nread;
	}

	return -1;
}
