#ifndef __ANSI_H__
#define __ANSI_H__

#ifndef __MINGW32__

#define CSI     "\033["

#define RESET           CSI "0m"
#define BOLD            CSI "1m"
#define UNDERLINE       CSI "4m"

#define RED     CSI "31m"
#define GREEN   CSI "32m"
#define YELLOW  CSI "33m"
#define BLUE    CSI "34m"
#define MAGENTA CSI "35m"
#define CYAN    CSI "36m"
#define WHITE   CSI "37m"

#else

#define RESET ""
#define BOLD ""
#define UNDERLINE ""

#define RED ""
#define GREEN ""
#define YELLOW ""
#define BLUE ""
#define MAGENTA ""
#define CYAN ""
#define WHITE ""

#endif

#endif
