/*
 * Program: server
 *
 * Description:
 *	This program binds to the given network address and executes the
 *	given command upon accepting a connection.
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
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>

/* Buffer for reading */
static char buffer[1024];

/* Function prototypes */
int usage (char * name);
void handleConnection (void) __attribute__ ((noinline));

/* Size of data read */
int size;

void
handleConnection (void) {
	/*
	 * Make some space in the executable for code to be injected.
	 */
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");
	__asm__ __volatile__ ("xor %rax, %rax");

	/*
	 * Loop forever and echo back what we read.
	 */
	while ((size = read (STDIN_FILENO, buffer, 1024)) > 0) {
		write (STDOUT_FILENO, buffer, size);
	}
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

	setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes));
	if ((bind (s, address, address_size)) == -1)
	{
		perror (argv[0]);
		exit (2);
	}


	/*
	 * Print the port on which the socket is listening.
	 */
	addrsize = sizeof (struct sockaddr_in);
	if ((getsockname (s, address, &addrsize)) == -1)
		perror ("Failed to get port number\n");
	printf ("Listening on port %d\n", ntohs(in_address.sin_port));

	if (listen (s, 5))
	{
		perror (argv[0]);
		exit (2);
	}

	while (1)
	{
		if ((connfd = accept (s, NULL, NULL)) == -1)
		{
			perror (argv[0]);
			exit (3);
		}

		/*
		 * Create a new child process.
		 */
		switch (pid = fork())
		{
			case -1:
				perror (argv[0]);
				exit (4);
				break;

			/* Child Process: Go do stuff */
			case 0:
				break;

			/* Parent Process: Go wait for another connection */
			default:
				close (connfd);
				continue;
		}

		/*
		 * Close the parent's accepting socket.
		 */
		close (s);

		/*
		 * Print out message that we have a new connection.
		 */
		printf ("Handling new connection\n");

		/*
		 * Close stdin and stdout and make them the new socket.
		 */
		close (STDIN_FILENO);
		close (STDOUT_FILENO);
		close (STDERR_FILENO);

		dup (connfd);
		dup (connfd);
		dup (connfd);

		handleConnection();
	}
}

int
usage (char * name)
{
	fprintf (stderr, "%s: <port> -p <port>", name);
	fprintf (stderr, "%s: \t<port> must be between 0 and 65535\n", name);
	return 0;
}

