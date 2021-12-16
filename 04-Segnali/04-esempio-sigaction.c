#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> // sigaction(), sig*()
#include <errno.h>


/*
 * Handler of a signal is a function taking an integer as input and
 * producing no output
 */
void handle_signal(int signal);

/*
 * Run the program and send to the process signals from another
 * terminal
 */

int main() {
	struct sigaction sa;
	sigset_t  my_mask;
	int i;

	// Print pid, so that we can send signals from other shells
	printf("My pid is: %d\n", getpid());

	/*
	 * Setup the signal handler. The handler is passed as pointer
	 * to a function with the syntax below.
	 *
	 * (In C, there are pointers to functions. It is an advanced
	 * topic, not discudded any further here)
	 */
	sa.sa_handler = handle_signal;
	sa.sa_flags = 0; 	// No special behaviour
	// Create an empty mask
	sigemptyset(&my_mask);        // Do not mask any signal
	sa.sa_mask = my_mask;

	// Set this handler for all signals
	for (i=0; i<NSIG; i++) {
		if (sigaction(i, &sa, NULL) == -1) {
			fprintf(stderr,
				"Cannot set a user-defined handler for Signal #%d: %s\n",
				i, strsignal(i));
		}
	}

	errno = 0;
	for (;;) {
		printf("PID %d - Sleeping for ~3 seconds\n", getpid());
		sleep(3);
	}
}

void handle_signal(int signal) {

	printf("Got signal #%d: %s\n", signal, strsignal(signal));
}
