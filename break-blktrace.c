#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/loop.h>
#include <linux/fs.h>
#include <linux/blktrace_api.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define DEFAULT_DEBUGFS_DIR "/sys/kernel/debug/block"

static char *blktrace_debugfs_files[] = {
	"msg",
	"dropped",
};

static void usage(char *progname)
{
	printf("Usage: %s [ -s | -h | -c <count> | -d ]\n", progname);
	printf("-a                Assume that the respective loop device exists\n");
	printf("-c <count>        Number of times to loop\n");
	printf("-h                Prints help menu\n");
	printf("-s                Skips BLKTRACETEARDOWN\n");
	printf("-d                Checks if debugfs files are present\n");
	printf("-z                Sleep for this amount of usecs before teardown\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int loop_ctl_fd, loop_dev_fd;
	int debugfs_fd;
	pid_t pid;
	struct stat statbuf;
	struct blk_user_trace_setup buts;
	bool assume_loop_exists = false;
	bool skip_teardown = false;
	bool check_debugfs = false;
	int c;
	unsigned int count = 10000, sleep = 0;
	int ret;
	unsigned int x;
	char fcheck[PATH_MAX];

	while ((c = getopt(argc, argv, "ashc:dz:")) != EOF) {
		switch (c) {
		case 'a':
			assume_loop_exists = true;
			break;
		case 'c':
			count = atoi(optarg);
			break;
		case 'd':
			check_debugfs = true;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 's':
			skip_teardown = true;
			break;
		case 'z':
			sleep = atoi(optarg);
			break;
		case '?':
			fprintf(stderr, "Unknown parameter: %c\n", optopt);
			usage(argv[0]);
		}
	}

	if (argc - optind > 1) {
		fprintf(stderr, "extra arguments\n");
		usage(argv[0]);
        }

	printf("Loop count: %d\n", count);

	if (assume_loop_exists)
		printf("Assuming loop device exists\n");
	else
		printf("Not assuming loop device exists\n");

	if (skip_teardown)
		printf("Skiping teardown\n");
	else
		printf("Not skiping teardown\n");

	if (!check_debugfs)
		printf("Skiping debugfs check\n");
	else
		printf("Checking debugfs\n");

	if (sleep)
		printf("Sleep %d usecs\n", sleep);

	loop_ctl_fd = open("/dev/loop-control", 3);
	if (loop_ctl_fd < 0) {
		perror("open(/dev/loop-control)");
		return 1;
	}

	ret = stat("/dev/loop0", &statbuf);
	if (!ret && assume_loop_exists) {
		perror("stat(/dev/loop0) tells us this loop device already exists");
		close(loop_ctl_fd);
		return 1;
	}

	pid = getpid();
	for (unsigned int i = 0; i < count; ++i) {
		printf("Iteration %d\n", i);

		ret = stat("/dev/loop0", &statbuf);
		if (ret == 0) {
			if (ioctl(loop_ctl_fd, LOOP_CTL_REMOVE, (unsigned long)0)) {
				perror("ioctl(/dev/loop-control, LOOP_CTL_REMOVE, 0)");
				close(loop_ctl_fd);
				return 1;
			}
		}

		if (ioctl(loop_ctl_fd, LOOP_CTL_ADD, (unsigned long)0)) {
			perror("ioctl(/dev/loop-control, LOOP_CTL_ADD, 0)");
			close(loop_ctl_fd);
			return 1;
		}


		loop_dev_fd = open("/dev/loop0", 3);
		if (loop_dev_fd < 0) {
			perror("open(/dev/loop0)");
			close(loop_ctl_fd);
			return 1;
		}

		buts = (struct blk_user_trace_setup) {
			.act_mask = 0,
			.buf_size = 1024,
			.buf_nr = 1,
			.start_lba = 0,
			.end_lba = 0,
			.pid = (uint32_t)pid,
		};
		if (ioctl(loop_dev_fd, BLKTRACESETUP, &buts)) {
			perror("ioctl(/dev/loop0, BLKTRACESETUP)");
			close(loop_dev_fd);
			close(loop_ctl_fd);
			return 1;
		}

		if (check_debugfs) {
			for (x = 0 ; x < ARRAY_SIZE(blktrace_debugfs_files); x++) {
				snprintf(fcheck, PATH_MAX, "%s/loop%d/%s", DEFAULT_DEBUGFS_DIR, 0, blktrace_debugfs_files[x]);
				debugfs_fd = open(fcheck, O_RDONLY);
				if (debugfs_fd == -1) {
					fprintf(stderr, "%s - fail, not present\n", fcheck);
				} else {
					printf("%s - OK\n", fcheck);
					close(debugfs_fd);
				}
			}

		}

		if (sleep)
			usleep(sleep);

		if (!skip_teardown && ioctl(loop_dev_fd, BLKTRACETEARDOWN, &buts)) {
			perror("ioctl(/dev/loop0, BLKTRACETEARDOWN)");
			close(loop_dev_fd);
			close(loop_ctl_fd);
		}

		close(loop_dev_fd);
	}

	close(loop_ctl_fd);
	return 0;
}
