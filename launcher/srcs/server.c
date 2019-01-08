/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   server.c                                                                 */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/07 10:45:39 by Mateo                                    */
/*   Updated: 2019/01/08 10:18:06 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.h"

void init_server(server_t *srv)
{
	if((srv->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			dprintf(2,"Error: opening the socket\n");
			exit(EXIT_FAILURE);
		}
	if (fcntl(srv->server_fd, F_SETFL, O_NONBLOCK) < 0)
		{
			dprintf(2,"Error: setting fd flag\n");
			exit(EXIT_FAILURE);
		}
	//to close the socket when program is finished
	if (setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
		{
			dprintf(2,"Error: setting fd optinon\n");
			exit(EXIT_FAILURE);
		}
	srv->server_data.sin_family = AF_INET;
	srv->server_data.sin_addr.s_addr = htons(ADDR);
	srv->server_data.sin_port = htons(PORT);
	srv->server_data_len = sizeof(srv->server_data);
	if ((bind(srv->server_fd, (struct sockaddr *)&srv->server_data,
						srv->server_data_len)) < 0)
		{
			dprintf(2, "Error: binding the socket\n");
			exit(EXIT_FAILURE);
		}
	if((listen(srv->server_fd, MAX_CONNECTIONS)) < 0)
		{
			dprintf(2, "Error: listening the clients\n");
			exit(EXIT_FAILURE);
		}
}

void accept_client(const server_t *srv, client_t **cli)
{
	int fd;
	client_t *new_cli;

	while ((fd = accept(srv->server_fd, (struct sockaddr *)&srv->server_data,
											(socklen_t *)&srv->server_data_len)) >= 0)
		{
			if (!(new_cli = (client_t *)malloc(sizeof(client_t))))
				{
					dprintf(2, "Error: in malloc accept_client\n");
					exit(EXIT_FAILURE);
				}
			if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
				{
					dprintf(2,"Error: setting fd flag\n");
					exit(EXIT_FAILURE);
				}
			new_cli->client_fd = fd;
			new_cli->next = *cli;
			*cli = new_cli;
			printf("A client has made a connection\n");
		}
	if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			printf("Error: in accept connection\n");
			exit(EXIT_FAILURE);
		}
}

void disconnect_client(client_t *prev, client_t **cli, client_t **head)
{
	client_t *tmp;

	if (*cli == *head)
		*head = (*cli)->next;
	else if ((*cli)->next == NULL)
		prev->next = NULL;
	else
		prev->next = (*cli)->next;
	close((*cli)->client_fd);
	tmp = *cli;
	*cli = prev->next;
	free(tmp);

}

void read_clients(client_t **head)
{
	client_t *cli;
	client_t *prev;
	char buff[BUFF_SIZE];
	ssize_t readed;

	prev = *head;
	cli = *head;
	while (cli)
		{
			while ((readed = read(cli->client_fd, buff, BUFF_SIZE)) > 0)
				{
					decode_data(buff, readed);
				}
			if (readed == 0 || (readed == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)))
				{
					disconnect_client(prev, &cli, head);
					printf("Client disconnected\n");
					continue;
				}
			prev = cli;
			cli = cli->next;
		}
}