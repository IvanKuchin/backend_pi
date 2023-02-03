#ifndef __CCTX__H__
#define __CCTX__H__

#include "c_ctx_request.h"
#include "ccgi.h"

using namespace std;

class c_ctx : public c_ctx_request
{
	private:
		CCgi		*cgi;

	public:
		c_ctx(CMysql *_db, CUser *_user, CCgi *_cgi, c_config *_config) : c_ctx_request(_db, _user, _config), cgi{_cgi} {};
		
		CCgi *		GetCgi()	{ return cgi; };

		void		SetCgi(CCgi *_cgi)				{ cgi = _cgi; };
};

#endif
