#ifndef PLATFORM_H_
#define PLATFORM_H_

#ifdef __WIN32__ // or whatever
#define SIZE_T_FMT "%ld"
#else
#define SIZE_T_FMT "%zd"
#endif

#endif /* PLATFORM_H_ */
