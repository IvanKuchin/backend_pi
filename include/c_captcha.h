#ifndef __C_CAPTCHA__H__
#define __C_CAPTCHA__H__

#include <string>
#include <tuple>
#include <curl/curl.h>
#include "c_config.h"

using namespace std;

class c_captcha {
private:
	const string 	CONFIG 		{"captcha/config"};
	const string 	VERIFY_PATH	{"/captcha/verify/"};
	string			sessid;
	c_config		*cfg;
	CURL			*curl;
	CURLcode		curlRes;


	tuple<string, string> ReadConfig();
	tuple<string, string> Fetch(const string &url, const string &solution);

public:
						c_captcha(const string &param1, c_config *param2) : sessid(param1), cfg(param2) {
							curl_global_init(CURL_GLOBAL_ALL);
						};
	tuple<bool, string>	Verify(const string &solution);
};

#endif