#ifndef __CVIDEOCONVERTER__H__
#define __CVIDEOCONVERTER__H__

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "localy.h"
#include "clog.h"
#include "utilities.h"

// --- below variable defined in localy.h, 
// --- which means include must be declared before this block
#ifndef	FFMPEG_DISABLE
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}
#endif


using namespace std;

#define	FEEDVIDEO_STDOUT							string(LOGDIR) + "ffmpeg.stdout"
#define	FEEDVIDEO_STDERR							string(LOGDIR) + "ffmpeg.stderr"
#define	FEEDVIDEO_NUMBER_OF_FOLDERS					512
#define	FEEDVIDEO_MAX_WIDTH							800
#define	FEEDVIDEO_MAX_HEIGHT						800

class CVideoConverter
{
	bool				_isSuccessConvert = false;
	string				_folderID;
	string				_filePrefix;
	string				_timestamp;
	string				_originalFileExtension;
	vector<string>		_exts = {".mp4", ".webm"};

	string				_stubImageFolderID;
	string				_stubImageFilename;

	int					_width = 0, _height = 0;
	string				_rotation = "";

	string				_metadataLocation = "";
	string				_metadataLocationAltitude = "";
	string				_metadataLocationLongitude = "";
	string				_metadataLocationLatitude = "";
	string				_metadataDateTime = "";
	string				_metadataMake = "";
	string				_metadataModel = "";
	string				_metadataSW = "";

private:
	bool	PickUniqPrefix(string srcFileName);
	bool	StubImagePickUniqPrefix();

	bool	VideoConvert(int dstIndex, char **argv);
	bool	ReadMetadataAndResolution();
	bool	ParseLocationToComponents(string src);

public:
			CVideoConverter();
			CVideoConverter(string originalFilename);

	string	GetStubImageFolderID();
	string	GetStubImageFilename();
	string 	GetStubImageFullFilename();
	string 	CopyStubImage();

	string	GetFinalFolder();

	string	GetTempFilename();
	string	GetTempFullFilename();

	//--- order: index of _exts vector
	string	GetPreFinalFilename(unsigned int order);
	string	GetPreFinalFullFilename(unsigned int order);

	//--- order: index of _exts vector
	string	GetFinalFilename(unsigned int order);
	string	GetFinalFullFilename(unsigned int order);

	//--- order: index of _exts vector
	string	GetStderrFullFilename(unsigned int order);
	string	GetStdoutFullFilename(unsigned int order);

	bool	FirstPhase();
	bool	SecondPhase();

	string	GetMetadataLocationAltitude() { return _metadataLocationAltitude; }
	string	GetMetadataLocationLongitude() { return _metadataLocationLongitude; }
	string	GetMetadataLocationLatitude() { return _metadataLocationLatitude; }
	string	GetMetadataDateTime() { return _metadataDateTime; }
	string	GetMetadataMake() { return _metadataMake; }
	string	GetMetadataModel() { return _metadataModel; }
	string	GetMetadataSW() { return _metadataSW; }

	void	Cleanup();
};

#endif
