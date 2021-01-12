#include "server.h"

int main(int argc, char *argv[])
{
	int master_socket, new_socket, client_socket[MAX_PLAYER], activity,
	    i, valread, sd, max_sd;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	char buffer[1025];
	fd_set readfds;

	char *message = "Connect6 Server v0.1 (beta) \r\n";
	
	// Initialize all client_socket[] to 0 so not checked
	for (i = 0; i < MAX_PLAYER; i++)
	{
		client_socket[i] = 0;
	}

	// Create a master socket

	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Set master socket to allow multiple connections,
	// this is just a good habit, it will work without this
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
				(char *)&opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Bind the socket to any, port 8089
	if (bind(master_socket, (struct sockaddr *)&address,
				sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	printf("Listener on port %d \n", PORT);

	// Try to specify maximum of 3 pending connections for he master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// Accept the incoming connection
	puts("Waiting for connections ...");

	while (1)
	{
		// Clear the socket set
		FD_ZERO(&readfds);

		// Add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		for (i = 0; i < MAX_PLAYER; i++)
		{
			// socket descriptor
			sd = client_socket[i];

			// if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds);

			// highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}

		// wait for an activity on one of the sockets, timeout is NULL,
		// so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}

		// if something happened on the master socket,
		// then it's an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket,
				(struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			// inform user of socket number - used in send and receive commands
			printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
				new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			// send new connection greeting message
			if (send(new_socket, message, strlen(message), 0) != strlen(message))
			{
				perror("send");
			}

			puts("Welcome message sent successfully");

			// add new socket to array of sockets
			for (i = 0; i < MAX_PLAYER; i++)
			{
				// if position is empty
				if (client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n", i);

					break;
				}
			}
		}

		// else its some IO operation on some other socket
		for (i = 0; i < MAX_PLAYER; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds))
			{
				// check if it was for closing, and also read the
				// incoming message
				if ((valread = read(sd, buffer, 1024)) == 0)
				{
					// somebody disconneced, get his details and print
					getpeername(sd, (struct sockaddr *)&address,
							(socklen_t *)&addrlen);
					printf("Host disconnected, ip %s, port %d \n",
						inet_ntoa(address.sin_addr), ntohs(address.sin_port));

					// close the socket and mark as 0 in list for reuse
					close(sd);
					client_socket[i] = 0;
				}

				// echo back the message that came in
				else
				{
					// set the string terminating NULL byte on the end
					// of the data read
					buffer[valread] = '\0';
					send(sd, buffer, strlen(buffer), 0);
				}
			}
		}
	}

	return 0;
}
