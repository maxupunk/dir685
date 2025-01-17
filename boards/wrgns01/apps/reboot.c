#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>
#include <sys/syslog.h>

const char * const bb_shutdown_format = "\r%s\n";

int kill_init(int sig)
{
#ifdef CONFIG_FEATURE_INITRD
	/* don't assume init's pid == 1 */
	long *pid = find_pid_by_name("init");
	if (!pid || *pid<=0) {
		pid = find_pid_by_name("linuxrc");
		if (!pid || *pid<=0)
			bb_error_msg_and_die("no process killed");
	}
	return(kill(*pid, sig));
#else
	return(kill(1, sig));
#endif
}

int bb_shutdown_system(unsigned long magic)
{
	int pri = LOG_KERN|LOG_NOTICE|LOG_FACMASK;
	const char *message;

	/* Don't kill ourself */
	signal(SIGTERM,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	setpgrp();

	/* Allow Ctrl-Alt-Del to reboot system. */
#ifndef RB_ENABLE_CAD
#define RB_ENABLE_CAD	0x89abcdef
#endif
	reboot(RB_ENABLE_CAD);

//	openlog(bb_applet_name, 0, pri);//int openlog(string ident, int option, int facility);

	message = "\nThe system is going down NOW !!";
	syslog(pri, "%s", message);
	printf(bb_shutdown_format, message);

	sync();

	/* Send signals to every process _except_ pid 1 */
	message = "Sending SIGTERM to all processes.";
	syslog(pri, "%s", message);
	printf(bb_shutdown_format, message);

	kill(-1, SIGTERM);
	sleep(1);
	sync();

	message = "Sending SIGKILL to all processes.";
	syslog(pri, "%s", message);
	printf(bb_shutdown_format, message);

	kill(-1, SIGKILL);
	sleep(1);

	sync();

	reboot(magic);
	return 0; /* Shrug */
}

int main(void)
{
	sleep(1);

#ifndef CONFIG_INIT
	#ifndef RB_AUTOBOOT
		#define RB_AUTOBOOT		0x01234567 //hard rest
	#endif
	return(bb_shutdown_system(RB_AUTOBOOT));
#else
	return kill_init(SIGTERM);
#endif
}

