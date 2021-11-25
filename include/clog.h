#ifndef __CLOG__H__
#define __CLOG__H__

#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <fstream>

using namespace std;

#include "localy.h"

#define DEBUG		0
#define	INFO		1	// --- this severity is the lowest recommended for production
#define	WARNING		2
#define	ERROR		3
#define	PANIC		4

#define	CURRENT_LOG_LEVEL			DEBUG

#define	LOG_FILE_MAX_LENGTH			500 // user to strip long output to log file
#define	LOG_FILE_NAME				string(LOGDIR) + "backend.log"
// #define	LOG_FILE_NAME				string(LOGDIR) + DOMAIN_NAME + ".log"


#define	LOG_MESSAGE(severity, classname_legacy, action, mess) \
{ \
	CLog	obj; \
	string	pretty = __PRETTY_FUNCTION__;  \
    auto    classname = ""s; \
	auto    action_output = ""s; \
    auto    func_name_finish = pretty.find("(");\
    \
    if(func_name_finish != string::npos)\
    {\
        auto    func_name_start = pretty.find_last_of(" :", func_name_finish);\
        auto    class_name_start = pretty.find_last_of(" ", func_name_finish);\
        auto    class_separator = pretty.find("::");\
\
    	if(\
    	    (func_name_start != string::npos) &&\
    	    (class_name_start != string::npos) &&\
    	    (class_name_start < func_name_finish - 3)\
    	    ) \
    	{ classname = pretty.substr(class_name_start + 1, func_name_start - class_name_start); } \
    	else if(\
    	    (class_separator != string::npos) &&\
    	    (class_name_start == string::npos)\
    	    )\
    	{ classname = pretty.substr(0, class_separator + 2); /* --- this case address constructors / destructors "Class1::~Class1()"  */ } \
    }\
	if(string(action).length()) action_output = " "s + action + ":"; \
\
	obj.Write(severity, classname + string(__func__) + "[" + to_string(__LINE__) + "]" + action_output + " " + mess); \
}

#define	MESSAGE_DEBUG(classname_legacy, action, mess)	LOG_MESSAGE(DEBUG,		classname_legacy, action, mess)
#define	MESSAGE_INFO(classname_legacy, action, mess)	LOG_MESSAGE(INFO,		classname_legacy, action, mess)
#define	MESSAGE_WARNING(classname_legacy, action, mess)	LOG_MESSAGE(WARNING,	classname_legacy, action, mess)
#define	MESSAGE_ERROR(classname_legacy, action, mess)	LOG_MESSAGE(ERROR,		classname_legacy, action, mess)
#define	MESSAGE_PANIC(classname_legacy, action, mess)	LOG_MESSAGE(PANIC,		classname_legacy, action, mess)

using namespace std::chrono;

class CLog
{
    private:
    	string	fileName;
//	CLock	lock;
    	
		string	SpellLogLevel(int level)
		{
			return  (level == DEBUG		? "DEBUG" :
				    (level == INFO		? "INFO" :
				    (level == WARNING	? "WARNING" :
				    (level == ERROR		? "ERROR" :
				    (level == PANIC		? "PANIC" :
				    			 		  "UNKNOWN")))));
		};

    public:
	CLog(): fileName (LOG_FILE_NAME)
	{
//		lock.Get("log");
	};

	CLog(string fName): fileName (fName)
	{
//		lock.Get("log");
	};

	void Write(int level, const string &mess)
	{
	    if(level < CURRENT_LOG_LEVEL) 
	    {
	    	return;
	    }
	    else
	    {
			fstream	fs;

			fs.open(fileName, std::fstream::out | std::fstream::app);    /* Flawfinder: ignore */
			if(fs.is_open())
			{

			    time_t			binTime		= time((time_t *)NULL);
			    struct tm		*timeInfo	= localtime(&binTime);
			    pid_t			processID	= getpid();
			    microseconds	ms			= duration_cast< microseconds >(system_clock::now().time_since_epoch());

			    if(timeInfo)
			    {
			    	string			msCount = to_string(ms.count());
			    	char			localtimeBuffer[80];   /* Flawfinder: ignore */
				    auto			curr_locale	= string(setlocale(LC_ALL, NULL));	// --- DO NOT remove string() !
				    																// --- otherwise following setlocale call will change memory content where curr_locale pointing out
				    																// --- string() copies memory content to local stack therefore it could be reused later.

				    setlocale(LC_ALL, LOCALE_ENGLISH.c_str());
				    // --- do NOT change time format !!!
				    // --- fluent-bit parses it as a timestamp regexp
			    	strftime(localtimeBuffer, 80 - 1, "%b %d %Y %T", timeInfo);
				    setlocale(LC_ALL, curr_locale.c_str());

			    	if(msCount.length() > 6) msCount = msCount.substr(msCount.length() - 6, 6);

					fs << localtimeBuffer << "." << msCount << "";
			    }

			    // --- do NOT change log format !!!
			    // --- logmessages parsed by fluent-bit_app-log_parser with regexp
				fs << "[" << processID << "] " << SpellLogLevel(level) << ": " << mess << endl;

			    fs.close();
			}
			else
			{
				cout << "CLog::" << __func__ << "[" << __LINE__ << "]: ERROR: opening log file [" << fileName << "] " << "<br>\n";
			}
	    }

//	    lock.UnLock();
	}

	void Write(int level, const string &mess, const string &p1)
	{
		Write(level, mess + " " + p1);
	}

	void Write(int level, const string &mess, const string &p1, const string &p2)
	{
		Write(level, mess + " " + p1 + " " + p2);
	}

	void Write(int level, const string &mess, const string &p1, const string &p2, const string &p3)
	{
		Write(level, mess + " " + p1 + " " + p2 + " " + p3);
	}

	inline void Debug(string action, string message)
	{
		Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:" + action + ": " + message);
	}

	~CLog()
	{
	}

};

#endif 