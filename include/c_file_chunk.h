#ifndef __C_FILE_CHUNK__H__
#define	__C_FILE_CHUNK__H__

#include "c_ctx_request.h"
#include "utilities.h"
#include "sql_queries.h"

using namespace std;

class c_file_chunk 
{
	private:
		c_ctx_request	*ctx;
		string			name = ""s;
		// --- Below variable is just a string that is consistent across all file chunks
		// --- it might be sessionID or imageTempSet or some other random parameter.
		// --- ImageTempSet has chosen due to it is already defined at this stage,
		// --- while sessionID - not yet
		// --- Filename is not enough to keep uniqness between users, 
		// --- this is the reason of having additional random string.
		string			random_reference = ""s;
		char			*content = nullptr;

		auto	craft_temp_filename(unsigned int start_pos, unsigned int finish_pos) -> string;
		auto 	save_to_file_system(char *content, unsigned int start_pos, unsigned int finish_pos) -> string;
		auto	save_metadata(unsigned int start_pos, unsigned int finish_pos, unsigned int file_size) -> string;

		auto	cleanup_memory() -> string;
		auto	cleanup_filesystem() -> string;
		auto	cleanup_db() -> string;
	public:
				c_file_chunk(const string &_fname, const string &_random_reference, c_ctx_request *_ctx) : ctx{_ctx}, name{_fname}, random_reference{_random_reference} {};
		auto	save(char *content, unsigned int start_pos, unsigned int finish_pos, unsigned int file_size) -> string;
		auto	is_full_file() -> pair<bool, string>;
		auto	get_file_content() -> pair<char *, string>;
				~c_file_chunk();
};

#endif
