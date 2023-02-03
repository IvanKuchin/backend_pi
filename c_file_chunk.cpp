#include "c_file_chunk.h"


auto c_file_chunk::craft_temp_filename(unsigned int start_pos, unsigned int finish_pos) -> string
{
	return random_reference + "-" + name + "-" + to_string(start_pos) + "-" + to_string(finish_pos);
}

auto c_file_chunk::is_full_file() -> pair<bool, string>
{
	MESSAGE_DEBUG("", "", "start(" + name + ")");

	auto	result = false;
	auto	error_message = ""s;

	auto	num_rows	= ctx->GetDB()->Query("SELECT SUM(`finish`-`start` + 1) FROM `temp_file_chunks` WHERE `file_name`=" + quoted(name) + " AND `random`=" + quoted(random_reference));
	if(num_rows)
	{
		auto	chunks_size = ctx->GetDB()->Get(0, 0);
		
		num_rows	= ctx->GetDB()->Query("SELECT `size` FROM `temp_file_chunks` WHERE `file_name`=" + quoted(name) + " AND `random`=" + quoted(random_reference) + " LIMIT 0,1");
		if(num_rows)
		{
			auto	file_size = ctx->GetDB()->Get(0, 0);

			result = (chunks_size == file_size);
		}
		else
		{
			error_message = gettext("SQL syntax error");
			MESSAGE_ERROR("", "", error_message);
		}
	}
	else
	{
		error_message = gettext("SQL syntax error");
		MESSAGE_ERROR("", "", error_message);
	}


	MESSAGE_DEBUG("", "", "finish (" + to_string(result) + ", " + error_message + ")");

	return make_pair(result, error_message);
}

auto c_file_chunk::get_file_content() -> pair<char *, string>
{
	MESSAGE_DEBUG("", "", "start(" + name + ")");

	auto	error_message = ""s;
	auto	db = ctx->GetDB();
	auto	temp_dir		= ctx->GetConfig()->GetFromFile("image_folders", "temp");

	if(temp_dir.length())
	{
		auto	num_rows	= db->Query("SELECT * FROM `temp_file_chunks` WHERE `id` IN (" + Get_FileChunksByNameAndRandom_sqlquery(name, random_reference) + ") ORDER BY `start`");
		if(num_rows)
		{
			auto	fsize = stol(db->Get(0, "size"));
			content = (char *)malloc(fsize);

			if(content != nullptr)
			{
				for(auto i = 0, current_pos = 0; i < num_rows; ++i)
				{
					auto	start = stol(db->Get(i, "start"));
					auto	finish = stol(db->Get(i, "finish"));
					auto	fname = temp_dir + "/" + craft_temp_filename(start, finish);

					if(isFileExists(fname))
					{
						ifstream	f(fname, ios::binary|ios::in);
						if(f.is_open())
						{
							f.read(&content[current_pos], finish - start + 1);   /* Flawfinder: ignore */
							f.close();
						}
						else
						{
							error_message = gettext("can't open file") + ": "s + fname;
							MESSAGE_ERROR("", "", error_message);
						}
					}
					else
					{
						error_message = gettext("file not found") + ": "s + fname;
						MESSAGE_ERROR("", "", error_message);
					}

					current_pos += finish - start + 1;
				}
			}
			else
			{
				error_message = gettext("memory allocation error");
				MESSAGE_ERROR("", "", error_message);
			}

		}
		else
		{
			error_message = gettext("SQL syntax error");
			MESSAGE_ERROR("", "", error_message);
		}
	}
	else
	{
		error_message = gettext("temp entry missed in image_folders config");
		MESSAGE_ERROR("", "", error_message);
	}


	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return make_pair(content, error_message);
}

auto c_file_chunk::save_metadata(unsigned int start_pos, unsigned int finish_pos, unsigned int file_size) -> string
{
	MESSAGE_DEBUG("", "", "start(" + name + ", " + to_string(start_pos) + ", " + to_string(finish_pos) + ", " + to_string(file_size) + ")");

	auto	error_message = ""s;
	auto	query = "INSERT INTO `temp_file_chunks` (`id`, `file_name`, `random`, `start`, `finish`, `size`, `eventTimestamp`) "
					"VALUES "
					"(NULL, " + quoted(name) + ", " + quoted(random_reference) + ", '" + to_string(start_pos) + "', '" + to_string(finish_pos) + "', '" + to_string(file_size) + "', UNIX_TIMESTAMP());";

	auto	id = ctx->GetDB()->InsertQuery(query);

	if(id)
	{

	}
	else
	{
		error_message = gettext("SQL syntax error");
		MESSAGE_ERROR("", "", error_message);
	}

	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return error_message;
}

auto c_file_chunk::save_to_file_system(char *content, unsigned int start_pos, unsigned int finish_pos) -> string
{
	MESSAGE_DEBUG("", "", "start(" + name + ", " + to_string(start_pos) + ", " + to_string(finish_pos) + ")");

	auto	error_message	= ""s;
	auto	temp_dir		= ctx->GetConfig()->GetFromFile("image_folders", "temp");

	if(temp_dir.length())
	{
		if(!isDirExists(temp_dir))
		{
			if(!CreateDir(temp_dir))
			{
				error_message = gettext("fail to create dir") + " "s + temp_dir;
				MESSAGE_ERROR("", "", error_message);
			}
		}

		if(error_message.empty())
		{
			auto	fname = craft_temp_filename(start_pos, finish_pos);

			if(!isFileExists(temp_dir + "/" + fname))
			{
				auto	f	= fopen((temp_dir + "/" + fname).c_str(), "w");   /* Flawfinder: ignore */

				if(f == NULL)
				{
					error_message = gettext("fail to write a file to a disk") + " "s + fname;
					MESSAGE_ERROR("", "", error_message);
				}
				else
				{
					fwrite(content, finish_pos - start_pos + 1, 1, f);
					fclose(f);
				}
			}
			else
			{
				error_message = "file is about to save already exists, means random reference is not good enough";
				MESSAGE_ERROR("", "", error_message);
			}
		}
	}
	else
	{
		error_message = gettext("temp entry missed in image_folders config");
		MESSAGE_ERROR("", "", error_message);
	}


	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return error_message;
}

auto c_file_chunk::save(char *content, unsigned int start_pos, unsigned int finish_pos, unsigned int file_size) -> string
{
	MESSAGE_DEBUG("", "", "start(" + name + ", " + to_string(start_pos) + ", " + to_string(finish_pos) + ", " + to_string(file_size) + ")");

	auto	error_message = ""s;

	error_message = save_to_file_system(content, start_pos, finish_pos);

	if(error_message.empty())
	{
		error_message = save_metadata(start_pos, finish_pos, file_size);
		if(error_message.empty())
		{
		}
		else
		{
			MESSAGE_DEBUG("", "", error_message);
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", error_message);
	}

	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return error_message;
}


auto	c_file_chunk::cleanup_memory() -> string
{
	MESSAGE_DEBUG("", "", "start");
	auto	error_message = ""s;

	if(content != nullptr) free(content);

	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return error_message;
}

auto	c_file_chunk::cleanup_filesystem() -> string
{
	MESSAGE_DEBUG("", "", "start");
	auto	error_message = ""s;
	auto	db = ctx->GetDB();
	auto	temp_dir		= ctx->GetConfig()->GetFromFile("image_folders", "temp");

	if(temp_dir.length())
	{

		auto	num_rows	= db->Query("SELECT * FROM `temp_file_chunks` WHERE `id` IN (" + Get_FileChunksByNameAndRandom_sqlquery(name, random_reference) + ")");
		if(num_rows)
		{
			for(auto i = 0; i < num_rows; ++i)
			{
				auto	start = stol(db->Get(i, "start"));
				auto	finish = stol(db->Get(i, "finish"));
				auto	fname = temp_dir + "/" + craft_temp_filename(start, finish);

				if(isFileExists(fname))
				{
					unlink(fname.c_str());
					MESSAGE_DEBUG("", "", "removed chunk: " + fname);
				}
				else
				{
					error_message = gettext("file not found") + ": "s + fname;
					MESSAGE_ERROR("", "", error_message);
				}
			}
		}
		else
		{
			error_message = gettext("SQL syntax error");
			MESSAGE_ERROR("", "", error_message);
		}
	}
	else
	{
		error_message = gettext("temp entry missed in image_folders config");
		MESSAGE_ERROR("", "", error_message);
	}

	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return error_message;
}

auto	c_file_chunk::cleanup_db() -> string
{
	MESSAGE_DEBUG("", "", "start");
	auto	error_message = ""s;

	ctx->GetDB()->Query("DELETE FROM `temp_file_chunks` WHERE `file_name`=" + quoted(name) + " AND `random`=" + quoted(random_reference));

	MESSAGE_DEBUG("", "", "finish (" + error_message + ")");

	return error_message;
}

c_file_chunk::~c_file_chunk()
{
	auto	error_message = cleanup_memory();
	auto	full_file = false;

	tie(full_file, error_message) = is_full_file();
	if(error_message.length())
	{
		MESSAGE_DEBUG("", "", error_message);
	}

	if(full_file)
	{
		if(error_message.length())
		{
			MESSAGE_ERROR("", "", "memory leak name(" + name + "), random(" + random_reference + ")");
		}

		error_message = cleanup_filesystem();
		if(error_message.length())
		{
			MESSAGE_ERROR("", "", "manual filesystem cleanup required file name(" + name + "), random(" + random_reference + ")");
		}

		error_message = cleanup_db();
		if(error_message.length())
		{
			MESSAGE_ERROR("", "", "manual DB cleanup required file name(" + name + "), random(" + random_reference + ")");
		}
	}
}
