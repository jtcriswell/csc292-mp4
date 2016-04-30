/*
 * Program: client
 *
 * Description:
 *	This program connects to the given network address and relays input
 *	and output between itself and the server.
 *
 * Copyright (c) 2003, 2004, 2016 John T Criswell
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>

/* Buffer for reading */
static char buffer[1024];

/* Function prototypes */
int usage (char * name);

void
handleConnection (int s) {
	/* Size of data read from local user*/
	int localsize;

	/* Size of data read from remote server */
	int remotesize;

	/*
	 * Loop until both sides have either closed their file descriptors or
	 * experience some sort of error.  In the meantime, read data from one
	 * end and print it on the other.
	 */
	unsigned char done = 0;
	do {
		/* Clear out the read set */
		fd_set readSet;
		FD_ZERO (&readSet);

		/*
		 * Set the socket and stdin as file descriptors on which we
  		 * will wait for data to be available for reading.
  		 */
		FD_SET (STDIN_FILENO, &readSet);
		FD_SET (s, &readSet);

		/* Wait for new data */
		if ((select (s + 1, &readSet, NULL, NULL, NULL)) == -1) {
			perror ("Select failed");
			exit (1);
		}

		/* Read data from stdin and write it to the socket */
		if (FD_ISSET (STDIN_FILENO, &readSet)) {
			localsize = read (STDIN_FILENO, buffer, 1024);
			if (localsize) {
				write (s, buffer, localsize);
			} else {
				shutdown (s, SHUT_WR);
			}
		}

		/* Read data from the socket and write it to stdout */
		if (FD_ISSET (s, &readSet)) {
			remotesize = read (s, buffer, 1024);
			write (STDOUT_FILENO, buffer, remotesize);
		}
	} while (remotesize > 0);

	return;
}

int
main (int argc, char ** argv)
{
	/* Network Addresses */
	struct sockaddr_in in_address;
	struct sockaddr *  address = NULL;

	/* Socket address sizes */
	socklen_t addrsize;

	/* Network port */
	unsigned short port = 0;

	/* Network address size */
	int address_size;

	/* Socket file descriptor */
	int s;

	/* New connection socket file descriptor */
	int connfd;

	/* Child process ID */
	int pid;

	/* Variable for setting socket options */
	int yes=1;

	/* Option argument */
	int option;

	/*
	 * Prevent creation of zombie processes.
	 */
	signal (SIGCHLD, SIG_IGN);

	/*
	 * Parse the command line options.
	 */
	while ((option = getopt (argc, argv, "p:")) != -1)
	{
		switch (option)
		{
			case 'p':
				port = atoi (optarg);
				break;

			default:
				usage(argv[0]);
				exit (1);
		}
	}

	/*
	 * Next, construct the network address.
	 */
	in_address.sin_family = AF_INET;
	in_address.sin_addr.s_addr = htonl (0x7f000001);
	in_address.sin_port = htons(port);
	address = (void *) &(in_address);
	address_size = sizeof (in_address);

	/*
	 * Create a socket and bind it to the specified address.
	 */
	if ((s = socket (address->sa_family, SOCK_STREAM, 0)) == -1)
	{
		perror (argv[0]);
		exit (2);
	}

	if ((connect (s, address, address_size)) == -1)
	{
		perror (argv[0]);
		exit (2);
	}

	/*
	 * Print the port on which the socket is listening.
	 */
	handleConnection(s);
	return 0;
}

int
usage (char * name)
{
	fprintf (stderr, "%s: <port> -p <port>", name);
	fprintf (stderr, "%s: \t<port> must be between 0 and 65535\n", name);
	return 0;
}

