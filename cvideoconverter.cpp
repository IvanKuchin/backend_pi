#include "cvideoconverter.h"

CVideoConverter::CVideoConverter()
{
	MESSAGE_DEBUG("", "", "start");
	

	MESSAGE_DEBUG("", "", "finish");
}

CVideoConverter::CVideoConverter(string originalFilename)
{
	MESSAGE_DEBUG("", "", "start");
	
	PickUniqPrefix(originalFilename);
	StubImagePickUniqPrefix();

	MESSAGE_DEBUG("", "", "finish");
}

bool CVideoConverter::PickUniqPrefix(string srcFileName)
{
	bool	result = false;
	bool	tryAgain;

	MESSAGE_DEBUG("", "", "start");

	do
	{
		ostringstream   ost;
		std::size_t	 foundPos;

		tryAgain = false;

		_folderID = to_string((int)(rand()/(RAND_MAX + 1.0) * FEEDVIDEO_NUMBER_OF_FOLDERS) + 1);
		_filePrefix = GetRandom(20);

		if((foundPos = srcFileName.rfind(".")) != string::npos) 
		{
			_originalFileExtension = toLower(srcFileName.substr(foundPos, srcFileName.length() - foundPos));
		}
		else
		{
			_originalFileExtension = ".avi";
		}


		// for(auto &item: _exts)
		for(unsigned int i = 0; i < _exts.size(); ++i)
		{
			//--- test final file
			if(isFileExists(GetFinalFullFilename(i))) tryAgain = true;
			//--- test final file in /tmp dir
			if(isFileExists(GetPreFinalFilename(i))) tryAgain = true;
			//--- test stdout file
			if(isFileExists(GetStdoutFullFilename(i))) tryAgain = true;
			//--- test stderr file
			if(isFileExists(GetStderrFullFilename(i))) tryAgain = true;
			
		} 
		if(isFileExists("/tmp/tmp_" + _filePrefix + _originalFileExtension)) tryAgain = true;

	} while(tryAgain);

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result) + ")");

	return  result;
}

bool CVideoConverter::StubImagePickUniqPrefix()
{
	MESSAGE_DEBUG("", "", "start");

	auto	result = false;
	auto	tryAgain = false;

	do
	{
		_stubImageFolderID = "1";
		_stubImageFilename = GetRandom(20) + ".png";
		auto _stubImageFileName = IMAGE_FEED_DIRECTORY + "/" + _stubImageFolderID + "/" + _stubImageFilename;

		if(isFileExists(_stubImageFileName)) tryAgain = true; else tryAgain = false;

	} while(tryAgain);

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result) + ")");

	return  result;
}

string CVideoConverter::GetStubImageFolderID()
{
	return _stubImageFolderID;
}

string CVideoConverter::GetStubImageFilename()
{
	return _stubImageFilename;
}

string CVideoConverter::GetStubImageFullFilename()
{
	return IMAGE_FEED_DIRECTORY + "/" + GetStubImageFolderID() + "/" + GetStubImageFilename();
}

string CVideoConverter::CopyStubImage() 
{
	MESSAGE_DEBUG("", "", "start");

	auto stub_image = IMAGE_FEED_DIRECTORY + "/1/video_stub.png";

	auto error_message = CopyFile(stub_image, GetStubImageFullFilename());

	MESSAGE_DEBUG("", "", "finish");

	return error_message;
}

string CVideoConverter::GetFinalFolder()
{
	string	result = "";
	MESSAGE_DEBUG("", "", "start");

	result = _folderID;

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");

	return result;
}

string CVideoConverter::GetTempFilename()
{
	string	result;

	result = _filePrefix + _originalFileExtension;

	return result;
}

string CVideoConverter::GetPreFinalFilename(unsigned int order)
{
	string	result = "";

	if(order < _exts.size())
		result = _filePrefix + _exts[order];
	else
	MESSAGE_DEBUG("", "", "order[" + to_string(order) + "] can't be bigger than _exts.size() [" + to_string(_exts.size()) + "])");

	return result;
}

string CVideoConverter::GetFinalFilename(unsigned int order)
{
	string	result = "";

	if(order < _exts.size())
		result = _filePrefix + _exts[order];
	else
	MESSAGE_DEBUG("", "", "order[" + to_string(order) + "] can't be bigger than _exts.size() [" + to_string(_exts.size()) + "])");

	return result;
}

string CVideoConverter::GetTempFullFilename()
{
	string	result;
	MESSAGE_DEBUG("", "", "start");

	result = "/tmp/tmp_" + GetTempFilename();

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");
	return result;
}

string CVideoConverter::GetPreFinalFullFilename(unsigned int order)
{
	string	result = "";
	MESSAGE_DEBUG("", "", "start");

	if(order < _exts.size())
		result = "/tmp/" + GetPreFinalFilename(order);
	else
	MESSAGE_DEBUG("", "", "order[" + to_string(order) + "] can't be bigger than _exts.size() [" + to_string(_exts.size()) + "])");

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");
	return result;
}

string CVideoConverter::GetFinalFullFilename(unsigned int order)
{
	string	result = "";
	MESSAGE_DEBUG("", "", "start");

	if(order < _exts.size())
		result = VIDEO_FEED_DIRECTORY + "/" + _folderID + "/" + GetFinalFilename(order);
	else
	MESSAGE_DEBUG("", "", "order[" + to_string(order) + "] can't be bigger than _exts.size() [" + to_string(_exts.size()) + "])");

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");
	return result;
}

string CVideoConverter::GetStdoutFullFilename(unsigned int order)
{
	string	result = "";
	MESSAGE_DEBUG("", "", "start");

	if(order < _exts.size())
		result = FEEDVIDEO_STDOUT + string("_") + _filePrefix + _exts[order];
	else
	MESSAGE_DEBUG("", "", "order[" + to_string(order) + "] can't be bigger than _exts.size() [" + to_string(_exts.size()) + "])");

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");
	return result;
}

string CVideoConverter::GetStderrFullFilename(unsigned int order)
{
	string	result = "";
	MESSAGE_DEBUG("", "", "start");

	if(order < _exts.size())
		result = FEEDVIDEO_STDERR + string("_") + _filePrefix + _exts[order];
	else
	MESSAGE_DEBUG("", "", "order[" + to_string(order) + "] can't be bigger than _exts.size() [" + to_string(_exts.size()) + "])");

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");
	return result;
}

bool CVideoConverter::ParseLocationToComponents(string src)
{
	bool		result = false;
	regex	 positionRegex(".*([+-][[:digit:]]+\\.[[:digit:]]+)([+-][[:digit:]]+\\.[[:digit:]]+)([+-][[:digit:]]+\\.[[:digit:]]+).*");
	smatch	matchResult;

	MESSAGE_DEBUG("", "", "start");

	if(regex_search(src, matchResult, positionRegex))
	{
		_metadataLocationLatitude = matchResult[1];
		_metadataLocationLongitude = matchResult[2];
		_metadataLocationAltitude = matchResult[3];

		result = true;
	}

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result) + ")");
	return result;
}

bool CVideoConverter::ReadMetadataAndResolution()
{
	bool				result = false;

	MESSAGE_DEBUG("", "", "start");

#ifndef FFMPEG_DISABLE

	AVFormatContext	 *fmt_ctx = NULL;
	AVDictionaryEntry   *tag = NULL;
	int				 ret;

	// av_register_all();
	ret = avformat_open_input(&fmt_ctx, GetTempFullFilename().c_str(), NULL, NULL);
	if(ret == 0)
	{
		result = true;

		// --- check file meta-data
		tag = nullptr;
		while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
		{
			// printf("%s=%s\n", tag->key, tag->value);
			string	key = tag->key;
			string	value = tag->value;

			if(key.find("creationdate") != string::npos) _metadataDateTime = value;
			if(key.find("make") != string::npos) _metadataMake = value;
			if(key.find("model") != string::npos) _metadataModel = value;
			if(key.find("software") != string::npos) _metadataSW = value;
			if(key.find("location") != string::npos) ParseLocationToComponents(value);
			if(key.find("rotate") != string::npos) _rotation = value;
		}

		for(unsigned int i = 0; i < fmt_ctx->nb_streams; ++i)
		{
			auto	stream = fmt_ctx->streams[i];

			if(stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				// --- define video dimensions
				try
				{
					_width = stream->codecpar->width;
					_height = stream->codecpar->height;
				} 
				catch (...)
				{
					MESSAGE_ERROR("", "", "convert video stream dimensions to numbers [" + to_string(stream->codecpar->width) + "x" + to_string(stream->codecpar->height) + "]");
				}

				MESSAGE_DEBUG("", "", "video stream dimensions [" + to_string(_width) + "x" + to_string(_height) + "]");

				// --- check video stream meta data
				// --- is it rotated dimensions
				tag = nullptr;
				while ((tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
				{
					string	key = tag->key;
					string	value = tag->value;

					if(key.find("rotate") != string::npos) _rotation = value;
				}
			}
		}


		avformat_close_input(&fmt_ctx);

		// --- rotate if needed
		if((_rotation == "90") || (_rotation == "270")) swap(_width, _height);

		result = true;
	}
	else
	{
		static char error_buffer[255];   /* Flawfinder: ignore */

		av_strerror(ret, error_buffer, sizeof(error_buffer) - 1);
		MESSAGE_ERROR("", "", "can't open " + GetTempFilename() + " (error: " + string(error_buffer) + ")");
	}

#else
	// --- if FFMPEG-DEV is not compiled
	result = true;
#endif

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result) + ")");
	return result;

}

bool CVideoConverter::FirstPhase()
{
	bool	result = false;
	char	scaleArg[30];    /* Flawfinder: ignore */
	// char	add_overlay_and_blur[512];
	char	*argv[10];   /* Flawfinder: ignore */

	string	tmpSrcFile, tmpDstFile;

	MESSAGE_DEBUG("", "", "start");

	if(ReadMetadataAndResolution())
	{

		tmpSrcFile = GetTempFullFilename();
		tmpDstFile = GetPreFinalFullFilename(0);

		if(_width && _height)
		{
			double	videoScaleWidth			= _width / FEEDVIDEO_MAX_WIDTH;
			double	videoScaleHeight		= _height / FEEDVIDEO_MAX_HEIGHT;
			double	videoScaleMax			= (videoScaleWidth > videoScaleHeight ? videoScaleWidth : videoScaleHeight);
			double	videoScaleFinalWidth	= videoScaleMax > 1 ? _width / videoScaleMax : _width;
			double	videoScaleFinalHeight	= videoScaleMax > 1 ? _height / videoScaleMax : _height;

			memset(scaleArg, 0, sizeof(scaleArg));
			sprintf(scaleArg, "scale=%d:%d", (int)videoScaleFinalWidth, (int)videoScaleFinalHeight);   /* Flawfinder: ignore */

			argv[0] = const_cast<char *>("ffmpeg");
			argv[1] = const_cast<char *>("-i");
			argv[2] = const_cast<char *>(tmpSrcFile.c_str());
			argv[3] = const_cast<char *>("-pix_fmt");
			argv[4] = const_cast<char *>("yuv420p");
			argv[5] = const_cast<char *>("-vf");
			argv[6] = const_cast<char *>(scaleArg);
			argv[7] = const_cast<char *>(tmpDstFile.c_str());
			argv[8] = NULL;

			MESSAGE_DEBUG("", "", "video scaled down to " + scaleArg);
		}
		else
		{
			argv[0] = const_cast<char *>("ffmpeg");
			argv[1] = const_cast<char *>("-i");
			argv[2] = const_cast<char *>(tmpSrcFile.c_str());
			argv[3] = const_cast<char *>("-pix_fmt");
			argv[4] = const_cast<char *>("yuv420p");
			argv[5] = const_cast<char *>(tmpDstFile.c_str());
			argv[6] = NULL;

			MESSAGE_DEBUG("", "", "video not found, probably music only");
		}


		if(VideoConvert(0, argv))
		{
			CopyFile(GetPreFinalFullFilename(0), GetFinalFullFilename(0));
			result = true;
		}
		else
		{
			MESSAGE_ERROR("", "", "fail to convert " + GetTempFullFilename() + " -> " + GetPreFinalFullFilename(0));
		}
	}
	else
	{
		MESSAGE_ERROR("", "", "fail to extract metadata or video resolution");
	}

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result));

	return result;
}

bool CVideoConverter::SecondPhase()
{
	bool	result = false;
	char	*argv[10];   /* Flawfinder: ignore */

	string	tmpSrcFile, tmpDstFile;

	MESSAGE_DEBUG("", "", "start");

	tmpSrcFile = GetTempFullFilename();
	tmpDstFile = GetPreFinalFullFilename(1);

	argv[0] = const_cast<char *>("ffmpeg");
	argv[1] = const_cast<char *>("-i");
	argv[2] = const_cast<char *>(tmpSrcFile.c_str());
	argv[3] = const_cast<char *>("-quality");
	argv[4] = const_cast<char *>("good");
	argv[5] = const_cast<char *>(tmpDstFile.c_str());
	argv[6] = NULL;

	if(VideoConvert(1, argv))
	{
		CopyFile(GetPreFinalFullFilename(1), GetFinalFullFilename(1));
		result = true;
		_isSuccessConvert = true;
	}
	else
	{
		MESSAGE_ERROR("", "", "fail to convert " + GetTempFullFilename() + " -> " + GetPreFinalFullFilename(0));
	}

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result) + ")");
	return result;
}

bool	CVideoConverter::VideoConvert(int dstIndex, char **argv)
{
	bool		result = false;
	void		(*del)(int), (*quit)(int);
	int		 pid;

	MESSAGE_DEBUG("", "", "start");

	del = signal(SIGINT, SIG_IGN); quit = signal(SIGQUIT, SIG_IGN);
	if( ! (pid = fork()))
	{   
		/* ветвление */
		/* порожденный процесс (сын) */
		signal(SIGINT, SIG_DFL); /* восстановить реакции */
		signal(SIGQUIT,SIG_DFL); /* по умолчанию		 */
		{
			CLog		log;

			/* getpid() выдает номер (идентификатор) данного процесса */
			pid = getpid();
			log.Write(DEBUG, "CVideoConverter::" + string(__func__) + "[" + to_string(__LINE__) + "]: child process: pid=" + to_string(pid) + " run");
		}

		{
			int	 ret;
			setpriority(PRIO_PROCESS, getpid(), 5);
			ret = getpriority(PRIO_PROCESS, getpid());
			if((ret > -20) and (ret < 19))
			{
				MESSAGE_DEBUG("", "", "process priority lowered down to " + to_string(ret));
			}
			else
			{
				MESSAGE_ERROR("", "", "getting process priority (errno = " + to_string(errno) + ")");
			}
		}

		/* Перенаправить ввод-вывод */
		if(RedirStderrToFile(GetStderrFullFilename(dstIndex)) && RedirStdoutToFile(GetStdoutFullFilename(dstIndex)))
		{
			string  tmpDestFile = GetPreFinalFullFilename(dstIndex);

			execvp(argv[0], argv);  /* Flawfinder: ignore */
		}
		else
		{
			MESSAGE_ERROR("", "", "can't redirect stdout or stderr to files");
		}

		// --- child process will stop here
		// --- execvp completely replace run-time environment to new process

		// we get here just in case Input return false 
		// при неудаче печатаем причину и завершаем порожденный процесс 
		MESSAGE_ERROR("", "", "redirecting input: " + strerror(errno));

		/* Мы не делаем free(firstfound),firstfound = NULL
		* потому что данный процесс завершается (поэтому ВСЯ его
		* память освобождается) :
		*/
		exit(1); //--- exit with error code 1
	}
	/* процесс - отец */
	/* Сейчас сигналы игнорируются, wait не может быть оборван
	* прерыванием с клавиатуры */

	// ожидание завершения дочернего процесса
	{
		int ws;
		int pid;
		ostringstream   ost;

		ost.str("");
		while((pid = wait( &ws)) > 0 )
		{
			if( WIFEXITED(ws))
			{
				ost << "pid=" << pid << " died, code " << WEXITSTATUS(ws);
				if(WEXITSTATUS(ws) == 0) result = true;
			}
			else if( WIFSIGNALED(ws))
			{
				ost << "pid=" << pid << " killed signal " << WTERMSIG(ws);
				if(WCOREDUMP(ws)) ost << " core dumped";
			}
			else if( WIFSTOPPED(ws))
			{
				ost << "pid=" << pid << " stopped signal " << WSTOPSIG(ws);
			}
		}
		if(!result)
		{
			MESSAGE_ERROR("", "", "child process: " + ost.str());
		}
	}

	/* восстановить реакции на сигналы от клавиатуры */
	signal(SIGINT, del); signal(SIGQUIT, quit);

	MESSAGE_DEBUG("", "", "finish (result = " + to_string(result) + ")");
	return result;	
}


void CVideoConverter::Cleanup()
{
	MESSAGE_DEBUG("", "", "start (_isSuccessConvert = " + to_string(_isSuccessConvert) + ")");

	if(_isSuccessConvert)
	{
		unlink(GetPreFinalFullFilename(0).c_str());
		unlink(GetPreFinalFullFilename(1).c_str());
		unlink(GetStderrFullFilename(0).c_str());
		unlink(GetStderrFullFilename(1).c_str());
		unlink(GetStdoutFullFilename(0).c_str());
		unlink(GetStdoutFullFilename(1).c_str());
		unlink(GetTempFullFilename().c_str());
	}

	MESSAGE_DEBUG("", "", "finish");
}

