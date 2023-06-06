/**
 *  @brief 
 *  info of the system
 */
#define maxlen (65)
struct utsname {
	char sysname[maxlen];
	char nodename[maxlen];
	char release[maxlen];
	char version[maxlen];
	char machine[maxlen];
	char domainname[maxlen];
};

int uname(struct utsname *uts);