#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct s_client
{
	int 	fd;
	int 	id;
	char 	*buf;
} t_client;

t_client clients[FD_SETSIZE];
fd_set active_set, read_set, write_set;
int max_fd = 0;
int next_id = 0;


void err(char *msg, int exit_code)
{
	if (!msg)
		msg = "Error Fatal\n";
	write(2, msg, strlen(msg));
	exit(exit_code);
}

void send_all(int except, char *msg)
{
	for (int i = 0 ; i <= max_fd ; i++)
	{
		if ( (clients[i].fd == 0) || (i == except) )
			continue;
		send(clients[i].fd, msg, strlen(msg), 0);
	}
}



int	extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int		i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char	*str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int	main( int ac, char **av  )
{
	int					sockfd;
	int					connfd;
	struct sockaddr_in	servaddr;
	bzero(clients,sizeof(clients));

	if (ac != 2)
		err("Wrong number of arguments\n", 0);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		err(NULL, 1);
	max_fd= sockfd;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));
	if ((bind(sockfd,
				(const struct sockaddr *)&servaddr,
				sizeof(servaddr))) != 0)
		err(NULL,1);
	if (listen(sockfd, 10) != 0)
		err(NULL,1);

	FD_ZERO(&active_set);
	FD_SET(sockfd, &active_set);

	while (1)
	{
		read_set = write_set = active_set;
		if ( (select(max_fd + 1, &read_set, &write_set, NULL, NULL) < 0) )
			continue;
		for(int fd = 0 ; fd <= max_fd ; fd++)
		{
			if (!FD_ISSET(fd, &read_set) )
				continue;
			if (fd == sockfd)
			{
				connfd = accept(sockfd, NULL, NULL);
				if (connfd < 0) continue;
				if (connfd > max_fd) max_fd = connfd;
				clients[connfd].fd = connfd;
				clients[connfd].id = next_id++;
				clients[connfd].buf= NULL;
				FD_SET(connfd, &active_set);
				char msg[50];
				sprintf(msg, "server: client %d just arrived\n", clients[connfd].id);
				send_all(connfd, msg);
			} else {
				char buf[FD_SETSIZE];
				int r = recv(fd, buf, sizeof(buf) - 1, 0);
				if ( r <= 0)
				{
					char msg[50];
					sprintf(msg, "server: client %d just left\n", clients[fd].id);
					send_all(fd, msg);
					FD_CLR(fd, &active_set);
					free(clients[fd].buf);
					close(fd);
					clients[fd].fd = 0;
				} else {
					buf[r]= 0 ;
					clients[fd].buf = str_join(clients[fd].buf, buf);
					if (!clients[fd].buf)
						err(NULL, 1);
					char *msg;
					while ( ( extract_message(&clients[fd].buf, &msg) == 1))
					{
						char prefix[50];
						sprintf(prefix, "cliente %d: ", clients[fd].id);
						char *full;
						full = malloc(strlen(prefix) + strlen(msg) + 1);
						if (!full)
							err(NULL, 1);
						full[0] = 0;
						strcat(full, prefix);
						strcat(full, msg);
						send_all(fd, full);
						free(msg);
						free(full);
					}
				}
			}
		}
	}
}
