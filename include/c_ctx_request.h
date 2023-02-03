#ifndef __CCTX_REQUEST__H__
#define __CCTX_REQUEST__H__

#include "cmysql.h"
#include "c_config.h"
#include "cuser.h"

using namespace std;

class c_ctx_request
{
	protected:
		CMysql		*db;
		CUser		*user;
		c_config	*config;

	public:
		c_ctx_request(CMysql *_db, CUser *_user, c_config *_config) : db{_db}, user{_user}, config{_config} {};
		
		CMysql *	GetDB()		{ return db; };
		CUser *		GetUset()	{ return user; };
		c_config * 	GetConfig()	{ return config; };

		void		SetDB(CMysql *_db)				{ db = _db; };
		void		SetUser(CUser *_user)			{ user = _user; };
		void	 	SetConfig(c_config *_config)	{ config = _config; };
};

#endif
