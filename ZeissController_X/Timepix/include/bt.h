#ifndef BT_H
#define BT_H

#if defined(_WINDLL) || defined(WIN32)
/* Thread.hpp from boost 1.52 on windows has a warning */
#pragma warning( push )
#pragma warning( disable : 4913 )
#endif
//#include <boost/thread.hpp>
#if defined(_WINDLL) || defined(WIN32)
#pragma warning( pop )
#endif

#endif