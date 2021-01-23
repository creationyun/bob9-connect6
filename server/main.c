#include "server.h"


int main(int argc, char *argv[])
{
    int master_socket, new_socket, client_socket[MAX_PLAYER], activity,
        i, valread, sd, max_sd;
    struct sockaddr_in address;
    struct timeval timeout;
    int opt = 1;
    int addrlen = sizeof(address);

    unsigned char recv[1025], sending[1025];
    size_t sending_len;
    fd_set readfds;

    struct GameOverData god;

    // char *message = "Connect6 Server v0.1 (beta) \r\n";
    
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

        // wait for an activity on one of the sockets
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;
        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error\n");
            exit(EXIT_FAILURE);
        }
        else if (activity == 0)
        {
            if (game_started)
            {
                printf("TIMEOUT packet sent by player%d\n", player_turn);
                make_timeout_payload(sending, 1024, &sending_len, player_turn);
                send(client_socket[0], sending, sending_len, 0);
                send(client_socket[1], sending, sending_len, 0);
                
                printf("GAME_OVER packet sent - RESULT_TIMEOUT\n");
                god.coord_num = 0;
                god.result = RESULT_TIMEOUT;
                make_game_over_payload(sending, 1024, &sending_len, player_turn%MAX_PLAYER+1, god);
                send(client_socket[0], sending, sending_len, 0);
                send(client_socket[1], sending, sending_len, 0);

                // game end.
                game_started = 0;
                init_game();
            }
            
            continue;
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
            /*
            if (send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");
            */

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

            // but if there are no socket, send ERROR packet
            if (i == MAX_PLAYER) {
                make_error_payload(sending, 1024, &sending_len, 0, ERROR_EXCEED_CAPACITY);
                send(new_socket, sending, sending_len, 0);

                printf("Reject the socket %d because of player counts\n", new_socket);
            }
        }

        // else it's some IO operation on some other socket
        for (i = 0; i < MAX_PLAYER; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                // check if it was for closing, and also read the
                // incoming message
                if ((valread = read(sd, recv, 1024)) == 0)
                {
                    // somebody disconnected, get his details and print
                    getpeername(sd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen);
                    printf("Host disconnected, ip %s, port %d \n",
                        inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                    player_game_joined[i] = 0;

                    // announce connection error to another player
                    if (game_started && client_socket[(i+1)%MAX_PLAYER])
                    {
                        printf("ERROR packet sent - ERROR_OTHER_PLAYER_DISCONNECTED\n");
                        make_error_payload(sending, 1024, &sending_len, (i+1)%MAX_PLAYER+1,
                            ERROR_OTHER_PLAYER_DISCONNECTED);
                        send(client_socket[(i+1)%MAX_PLAYER], sending, sending_len, 0);

                        printf("GAME_OVER packet sent - RESULT_CONNECTION_ERROR\n");
                        god.coord_num = 0;
                        god.result = RESULT_CONNECTION_ERROR;
                        make_game_over_payload(sending, 1024, &sending_len, (i+1)%MAX_PLAYER+1, god);
                        send(client_socket[(i+1)%MAX_PLAYER], sending, sending_len, 0);

                        // game end.
                        game_started = 0;
                        init_game();
                    }
                }

                // echo back the message that came in
                else
                {
                    // get his details and print
                    getpeername(sd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen);
                    printf("[%s, player%d] ", inet_ntoa(address.sin_addr), i+1);
                    connect6_packet_process(i, client_socket, recv, 1024);

                    // set the string terminating NULL byte on the end
                    // of the data read
                    /*
                    if (sending_len > 0) {
                        send(sd, sending, sending_len, 0);
                    }
                    */
                }
            }
        }
    }

    return 0;
}
