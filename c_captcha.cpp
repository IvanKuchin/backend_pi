#include "c_captcha.h"

//==============================================================================
struct  MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;

    MESSAGE_DEBUG("", "", "curl received " + to_string(realsize) + " bytes of content");

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
//    printf("%d", mem->memory);
    if (mem->memory == NULL)
    {
		/* out of memory! */ 
		MESSAGE_ERROR("", "", "not enough memory (realloc returned NULL)")
		return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);    /* Flawfinder: ignore */
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
//==============================================================================

tuple<string, string> c_captcha::ReadConfig() {
	MESSAGE_DEBUG("", "", "start");

	auto	url = cfg->GetFromFile(CONFIG, "URL");
	auto	error_message = ""s;
	auto	result = ""s;

	if(url.length()) {
		result = url;
	} else {
		error_message = "captcha config file has no URL-key"s;
		MESSAGE_ERROR("", "", error_message);
	}

	MESSAGE_DEBUG("", "", "finish (" + result + ")");

	return make_tuple(result, error_message);
}

tuple<bool, string> c_captcha::Verify(const string &solution) {
	MESSAGE_DEBUG("", "", "start");

	auto	error_message = ""s;
	auto	result = false;
	auto	url = ""s;

	tie(url, error_message) = ReadConfig();
	if(error_message.empty()) {
		auto	data = ""s;

		tie(data, error_message) = Fetch(url, solution);
		if(error_message.empty()) {
			result = (data == "true" ? true : false);
		} else {
			MESSAGE_ERROR("", "", error_message);
		}
	} else {
		MESSAGE_ERROR("", "", error_message);
	}


	MESSAGE_DEBUG("", "", "finish(" + to_string(result) + ")");

	return make_tuple(result, error_message);
}

tuple<string, string> c_captcha::Fetch(const string &url, const string &solution) {
	MESSAGE_DEBUG("", "", "start(" + url + ", " + solution + ")");

	auto	error_message	= ""s;
	auto	result			= ""s;
	auto	full_url		= url + VERIFY_PATH + solution;
	auto	cookie			= "sessid=" + sessid;

    struct MemoryStruct chunk;
    chunk.memory = (char *)malloc(1); // will be grown as needed by the realloc above
    chunk.memory[0] = 0;
    chunk.size = 0; // no data at this point  

	curl = curl_easy_init();
	if(curl)
	{
		// --- specify URL to get
		curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());

		// --- send all data to variable
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);

		// --- enable HTTP redirects 3xx
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		/* some servers don't like requests that are made without a user-agent
		 field, so we provide one */
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		curlRes = curl_easy_perform(curl);
		if(curlRes == CURLE_OK)
		{
			result = chunk.memory;
            free(chunk.memory);

			MESSAGE_DEBUG("", "", "fetched data(" + result + ")");			
		}
		else
		{
			error_message = "curl_easy_perform() returned error ("s + curl_easy_strerror(curlRes) + ")";
			MESSAGE_ERROR("", "", error_message)
		}

		curl_easy_cleanup(curl);
	}
	else
	{
		error_message = "curl_easy_init() returned null"s;
		MESSAGE_ERROR("", "", error_message);
	}


	MESSAGE_DEBUG("", "", "finish(" + result + ")");

	return make_tuple(result, error_message);
}
