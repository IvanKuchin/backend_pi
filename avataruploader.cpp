#include "avataruploader.h"	 

static bool ImageSaveAsJpg(const string src, const string dst, c_config *config)
{
	MESSAGE_DEBUG("", "", "start (" + src + ", " + dst + ")");

#ifndef IMAGEMAGICK_DISABLE
	// Construct the image object. Separating image construction from the
	// the read operation ensures that a failure to read the image file
	// doesn't render the image object useless.
	try {
		Magick::Image		   image;
		Magick::OrientationType imageOrientation;
		Magick::Geometry		imageGeometry;

		auto	avatar_max_width	= stod_noexcept(config->GetFromFile("image_max_width", "avatar"));
		auto	avatar_max_height	= stod_noexcept(config->GetFromFile("image_max_height", "avatar"));

		// Read a file into image object
		image.read( src );   /* Flawfinder: ignore */

		imageGeometry = image.size();
		imageOrientation = image.orientation();

		{
			ostringstream   ost;

			ost.str("");
			ost << "ImageSaveAsJpg (" << src << ", " << dst << "): imageOrientation = " << imageOrientation << ", xRes = " << imageGeometry.width() << ", yRes = " << imageGeometry.height();
			MESSAGE_DEBUG("", "", ost.str());
		}

		if(imageOrientation == Magick::TopRightOrientation) image.flop();
		if(imageOrientation == Magick::BottomRightOrientation) image.rotate(180);
		if(imageOrientation == Magick::BottomLeftOrientation) { image.flop(); image.rotate(180); }
		if(imageOrientation == Magick::LeftTopOrientation) { image.flop(); image.rotate(-90); }
		if(imageOrientation == Magick::RightTopOrientation) image.rotate(90);
		if(imageOrientation == Magick::RightBottomOrientation) { image.flop(); image.rotate(90); }
		if(imageOrientation == Magick::LeftBottomOrientation) image.rotate(-90);

		if((imageGeometry.width() > avatar_max_width) || (imageGeometry.height() > avatar_max_height))
		{
			int   newHeight, newWidth;
			if(imageGeometry.width() >= imageGeometry.height())
			{
				newWidth = avatar_max_width;
				newHeight = newWidth * imageGeometry.height() / imageGeometry.width();
			}
			else
			{
				newHeight = avatar_max_height;
				newWidth = newHeight * imageGeometry.width() / imageGeometry.height();
			}

			image.resize(Magick::Geometry(newWidth, newHeight, 0, 0));
		}

		// Write the image to a file
		image.write( dst );
	}
	catch( Magick::Exception &error_ )
	{
		MESSAGE_ERROR("", "", "exception in read/write operation [" + error_.what() + "]")

		return false;
	}

	MESSAGE_DEBUG("", "", "image has been successfully converted to .jpg format")

	return true;
#else

	MESSAGE_DEBUG("", "", "simple file coping cause ImageMagick++ is not activated");

	CopyFile(src, dst);
	return  true;
#endif
}

int main()
{
	CStatistics		appStat;  // --- CStatistics must be a first statement to measure end2end param's
	CCgi			indexPage(EXTERNAL_TEMPLATE);
	CUser			user;
	string		  	action, partnerID;
	CMysql		  	db;
	c_config		config(CONFIG_DIR);
	struct timeval  tv;
	ostringstream   ostJSONResult/*(static_cast<ostringstream&&>(ostringstream() << "["))*/;


	MESSAGE_DEBUG("", "", __FILE__)

	signal(SIGSEGV, crash_handler); 

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);  /* Flawfinder: ignore */
	ostJSONResult.clear();
		
	try
	{
		indexPage.ParseURL();

		if(!indexPage.SetTemplate("index.htmlt"))
		{

			MESSAGE_ERROR("", "", "template file was missing");
			throw CException("Template file was missing");
		}

		if(db.Connect(&config) < 0)
		{

			MESSAGE_ERROR("", "", "Can not connect to mysql database");
			throw CException("MySql connection");
		}

		indexPage.SetDB(&db);

#ifndef IMAGEMAGICK_DISABLE
		Magick::InitializeMagick(NULL);
#endif
			
		action = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("action"));
		MESSAGE_DEBUG("", "", "action taken from HTTP is " + action);

		// --- generate common parts
		{
			// --- it has to be run before session, because session relay upon Apache environment variable
			if(RegisterInitialVariables(&indexPage, &db, &user)) {}
			else
			{
				MESSAGE_ERROR("", "", "RegisterInitialVariables failed, throwing exception");
				throw CExceptionHTML("environment variable error");
			}

			//------- Generate session
			action = GenerateSession(action, &config, &indexPage, &db, &user);
		}
		// ------------ end generate common parts

		MESSAGE_DEBUG("", "", "pre-condition if(action == \"" + action + "\")");


		if((user.GetID().length()) && (user.GetName() != "Guest"))
		{

			for(int filesCounter = 0; filesCounter < indexPage.GetFilesHandler()->Count(); filesCounter++)
			{
				FILE			*f;
				auto			number_of_folders	= stod_noexcept(config.GetFromFile("number_of_folders", "avatar"));
				auto			file_size_limit		= stod_noexcept(config.GetFromFile("max_file_size", "avatar"));
				int				folderID = (int)(rand()/(RAND_MAX + 1.0) * number_of_folders) + 1;
				string			filePrefix = GetRandom(20);
				string			file2Check, tmpFile2Check, tmpImageJPG, fileName, fileExtension;
				ostringstream   ost;
				int			 affected;

				// --- Check the number of files for that user
				affected = db.Query("select id from `users_avatars` where `userid`=" + quoted(user.GetID()) + ";");
				if(affected < 4) // --- 1 text avatar and 3 image avatar
				{ 
					MESSAGE_DEBUG("", "", "number of files POST'ed = " + to_string(indexPage.GetFilesHandler()->Count()))

					if(indexPage.GetFilesHandler()->GetSize(filesCounter) > file_size_limit) 
					{
						ostringstream	ost;

						ost.str("");
						ost << string(__func__) + ": ERROR avatar file [" << indexPage.GetFilesHandler()->GetName(filesCounter) << "] size exceed permitted maximum: " << indexPage.GetFilesHandler()->GetSize(filesCounter) << " > " << file_size_limit;

						MESSAGE_ERROR("", "", ost.str());
						throw CExceptionHTML("avatar file size exceed", indexPage.GetFilesHandler()->GetName(filesCounter));
					}

					//--- check avatar file existing
					do
					{
						ostringstream	ost;
						string		  tmp;
						std::size_t  foundPos;

						folderID = (int)(rand()/(RAND_MAX + 1.0) * number_of_folders) + 1;
						filePrefix = GetRandom(20);
						tmp = indexPage.GetFilesHandler()->GetName(filesCounter);

						if((foundPos = tmp.rfind(".")) != string::npos) 
						{
							fileExtension = tmp.substr(foundPos, tmp.length() - foundPos);
						}
						else
						{
							fileExtension = ".jpg";
						}

						file2Check = config.GetFromFile("image_folders", "avatar") + "/avatars" + to_string(folderID) + "/" + filePrefix + ".jpg";
						tmpFile2Check = "/tmp/tmp_" + filePrefix + fileExtension;
						tmpImageJPG = "/tmp/" + filePrefix + ".jpg";

					} while(isFileExists(file2Check) || isFileExists(tmpFile2Check) || isFileExists(tmpImageJPG));

					MESSAGE_DEBUG("", "", "Save file to /tmp for checking of image validity [" + tmpFile2Check + "]");

					// --- Save file to "/tmp/" for checking of image validity
					f = fopen(tmpFile2Check.c_str(), "w");   /* Flawfinder: ignore */
					if(f == NULL)
					{
						{

							MESSAGE_ERROR("", "", "writing file: " + tmpFile2Check);
							throw CExceptionHTML("avatar file write error", indexPage.GetFilesHandler()->GetName(filesCounter));
						}
					}
					fwrite(indexPage.GetFilesHandler()->Get(filesCounter), indexPage.GetFilesHandler()->GetSize(filesCounter), 1, f);
					fclose(f);

					if(ImageSaveAsJpg(tmpFile2Check, tmpImageJPG, &config))
					{

						MESSAGE_DEBUG("", "", "chosen filename for avatar [" + file2Check + "]");

						// --- remove previous logo
						if(db.Query("SELECT * FROM `users_avatars` WHERE `userid`=\"" + user.GetID() + "\" AND `isActive`=\"1\";"))
						{
							auto	currLogo = config.GetFromFile("image_folders", "avatar") + "/" + db.Get(0, "folder") + "/" + db.Get(0, "filename");
							auto	id = db.Get(0, "id");

							if(isFileExists(currLogo)) 
							{
								MESSAGE_DEBUG("", "", "remove current logo (" + currLogo + ")");
								unlink(currLogo.c_str());
							}

							db.Query("DELETE FROM `users_avatars` WHERE `id`=" + quoted(id) + ";");
						}
						else
						{
							MESSAGE_DEBUG("", "", gettext("no active avatar found"));
						}

						CopyFile(tmpImageJPG, file2Check);

						if(auto avatarID = db.InsertQuery("insert into `users_avatars` set `userid`='" + user.GetID() + "', `folder`='" + to_string(folderID) + "', `filename`='" + filePrefix + ".jpg', `isActive`=\"1\";"))
						{
							if(filesCounter == 0) ostJSONResult << "[" << std::endl;
							if(filesCounter  > 0) ostJSONResult << ",";
							ostJSONResult << "{" << std::endl;
							ostJSONResult << "\"result\": \"success\"," << std::endl;
							ostJSONResult << "\"textStatus\": \"\"," << std::endl;
							ostJSONResult << "\"fileName\": \"" << indexPage.GetFilesHandler()->GetName(filesCounter) << "\" ," << std::endl;
							ostJSONResult << "\"jqXHR\": \"\"" << std::endl;
							ostJSONResult << "}" << std::endl;
							if(filesCounter == (indexPage.GetFilesHandler()->Count() - 1)) ostJSONResult << "]";

							// --- Update live feed
							if(!db.InsertQuery("insert into `feed` (`title`, `userId`, `actionTypeId`, `actionId`, `eventTimestamp`) values(\"\",\"" + user.GetID() + "\", \"10\", \"" + to_string(avatarID) + "\", NOW())"))
							{
								MESSAGE_ERROR("", "", gettext("SQL syntax error"))
							}
						}
						else
						{
							MESSAGE_ERROR("", "", gettext("SQL syntax error"))

							if(filesCounter == 0) ostJSONResult << "[" << std::endl;
							if(filesCounter  > 0) ostJSONResult << ",";
							ostJSONResult << "{" << std::endl;
							ostJSONResult << "\"result\": \"error\"," << std::endl;
							ostJSONResult << "\"textStatus\": \"ERROR inserting into `user_avatars` table\"," << std::endl;
							ostJSONResult << "\"fileName\": \"\" ," << std::endl;
							ostJSONResult << "\"jqXHR\": \"\"" << std::endl;
							ostJSONResult << "}" << std::endl;
							if(filesCounter == (indexPage.GetFilesHandler()->Count() - 1)) ostJSONResult << "]";

							// --- Delete temporarily files
							unlink(tmpFile2Check.c_str());
							unlink(tmpImageJPG.c_str());
						}

					}
					else
					{
						MESSAGE_DEBUG("", "", "avatar [" + indexPage.GetFilesHandler()->GetName(filesCounter) + "] is not valid image");

						if(filesCounter == 0) ostJSONResult << "[" << std::endl;
						if(filesCounter  > 0) ostJSONResult << ",";
						ostJSONResult << "{" << std::endl;
						ostJSONResult << "\"result\": \"error\"," << std::endl;
						ostJSONResult << "\"textStatus\": \"wrong format\"," << std::endl;
						ostJSONResult << "\"fileName\": \"" << indexPage.GetFilesHandler()->GetName(filesCounter) << "\" ," << std::endl;
						ostJSONResult << "\"jqXHR\": \"\"" << std::endl;
						ostJSONResult << "}" << std::endl;
						if(filesCounter == (indexPage.GetFilesHandler()->Count() - 1)) ostJSONResult << "]";
					}
					// --- Delete temporarily files
					unlink(tmpFile2Check.c_str());
					unlink(tmpImageJPG.c_str());

				} //--- Check that number of avatars <= 4
				else
				{
					MESSAGE_DEBUG("", "", "number of avatars for user " + user.GetLogin() + " exceeds 4");

					if(filesCounter == 0) ostJSONResult << "[" << std::endl;
					if(filesCounter  > 0) ostJSONResult << ",";
					ostJSONResult << "{" << std::endl;
					ostJSONResult << "\"result\": \"error\"," << std::endl;
					ostJSONResult << "\"textStatus\": \"number of avatars exceeds\"," << std::endl;
					ostJSONResult << "\"fileName\": \"" << indexPage.GetFilesHandler()->GetName(filesCounter) << "\" ," << std::endl;
					ostJSONResult << "\"jqXHR\": \"\"" << std::endl;
					ostJSONResult << "}" << std::endl;
					if(filesCounter == (indexPage.GetFilesHandler()->Count() - 1)) ostJSONResult << "]";
				}

			} // --- Loop through all uploaded files

			indexPage.RegisterVariableForce("result", ostJSONResult.str());

		}
		else
		{
			ostringstream   ost;
			
			ost.str("");
			ost << "{" << std::endl;
			ost << "\"result\": \"error\"," << std::endl;
			ost << "\"textStatus\": \"" << indexPage.GetVarsHandler()->Get("ErrorDescription") << "\"," << std::endl;
			ost << "\"fileName\": \"\" ," << std::endl;
			ost << "\"jqXHR\": \"\"" << std::endl;
			ost << "}" << std::endl;

			indexPage.RegisterVariableForce("result", ost.str());
		}

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{

			MESSAGE_ERROR("", "", "template file is missing");
			throw CException("Template file was missing");
		}

		indexPage.OutTemplate();

	}
	catch(CExceptionHTML &c)
	{

		c.SetLanguage(indexPage.GetLanguage());
		c.SetDB(&db);

		MESSAGE_ERROR("", "", "catch CExceptionHTML: exception reason: [" + c.GetReason() + "]");

		if(c.GetReason() == "avatar file write error")
		{
			ostringstream   ost;

			ost.str("");
			ost << "\"result\": \"error\"," << std::endl;
			ost << "\"textStatus\": \"" << c.GetReason() << ", file: (" << c.GetParam1() << ")\"," << std::endl;
			ost << "\"jqXHR\": \"\"" << std::endl;
			indexPage.RegisterVariableForce("result", ost.str());
		}
		if(c.GetReason() == "avatar file size exceed")
		{
			ostringstream   ost;

			ost.str("");
			ost << "\"result\": \"error\"," << std::endl;
			ost << "\"textStatus\": \"" << c.GetReason() << ", file: (" << c.GetParam1() << ")\"," << std::endl;
			ost << "\"jqXHR\": \"\"" << std::endl;
			indexPage.RegisterVariableForce("result", ost.str());
		}


		if(!indexPage.SetTemplate(c.GetTemplate()))
		{
			MESSAGE_ERROR("", "", "template not found");
			return(-1);
		}
		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();
		
		return(-1);
	}
	catch(CException &c)
	{
		MESSAGE_ERROR("", action, "catch CException: exception: ERROR  " + c.GetReason());

		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			MESSAGE_ERROR("", "", "template not found");
			return(-1);
		}

		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();

		return(-1);
	}
	catch(exception& e)
	{
		MESSAGE_ERROR("", action, "catch(exception& e): catch standard exception: ERROR  " + e.what());

		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			MESSAGE_ERROR("", "", "template not found");
			return(-1);
		}
		
		indexPage.RegisterVariable("content", e.what());
		indexPage.OutTemplate();

		return(-1);
	}

	return(0);
}
