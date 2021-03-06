#include "cactivator.h"

CActivator::CActivator() : cgi(NULL), db(NULL)
{
	struct	timeval	tv;

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);  /* Flawfinder: ignore */

	actID = GetRandom(ACTIVATOR_LEN);
}

void CActivator::Save()
{
	if(!db)
	{
		MESSAGE_ERROR("", "", "connect to database in CActivator module");

		throw CExceptionHTML("error db");
	}

	if(GetUser().empty())
	{
		MESSAGE_ERROR("", "", "user name must be set in activator");

		throw CExceptionHTML("activator error");
	}
	if(GetID().empty())
	{
		MESSAGE_ERROR("", "", "id must be set in activator");

		throw CExceptionHTML("activator error");
	}
	if(GetType().empty())
	{
		MESSAGE_ERROR("", "", "type must be set in activator");

		throw CExceptionHTML("activator error");
	}

	db->Query("INSERT INTO `activators` (`id` , `user` , `type` , `date` ) VALUES ('" + GetID() + "', '" + GetUser() + "', '" + GetType() + "', NOW());");
}

bool CActivator::Load(string id)
{
	if(!db)
	{
		MESSAGE_ERROR("", "", "connect to database in CActivator module");

		throw CExceptionHTML("error db");
	}
	if(id.empty())
	{
		MESSAGE_ERROR("", "", "id must be set in activator");

		throw CExceptionHTML("activator error");
	}

	if(db->Query("SELECT * FROM `activators` WHERE `id`=\"" + id + "\";") == 0)
	{
		MESSAGE_DEBUG("", "", "there is no such activator");

		return false;
	}
	SetID(id);
	SetUser(db->Get(0, "user"));
	SetType(db->Get(0, "type"));
	SetTime(db->Get(0, "date"));

	return true;
}

void CActivator::Delete(string id)
{
	ostringstream	ost;

	if(!db)
	{
		MESSAGE_ERROR("", "", "connect to database in CActivator module");

		throw CExceptionHTML("error db");
	}
	if(id.empty())
	{
		MESSAGE_ERROR("", "", "id must be set in activator");

		throw CExceptionHTML("activator error");
	}

	db->Query("DELETE FROM `activators` WHERE `id`='" + GetID() + "';");
}

void CActivator::DeleteByUser(string userToDelete)
{
	if(!db)
	{
		MESSAGE_ERROR("", "", "error connect to database in CActivator module");

		throw CExceptionHTML("error db");
	}
	if(userToDelete.empty())
	{
		MESSAGE_ERROR("", "", "userToDelete must be set in activator");

		throw CExceptionHTML("activator error");
	}

	db->Query("DELETE FROM `activators` WHERE `user`=\"" + userToDelete + "\" and `type`='regNewUser';");
}

void CActivator::Activate()
{
	int		affected;
	string		u, t;

	if(GetID().empty())
	{
		MESSAGE_ERROR("", "", "Activator ID is empty while trying activate event");

		throw CExceptionHTML("no activator");
	}

	if(db == NULL)
	{
		MESSAGE_ERROR("", "", "error connect DB in CActivator module");

		throw CExceptionHTML("error db");
	}
	affected = db->Query("select * from `activators` where `id`='" + GetID() + "' and `date`>=(now() - INTERVAL " + to_string(ACTIVATOR_SESSION_LEN) + " MINUTE);");
	if(affected == 0) throw CExceptionHTML("no activator");

	u = db->Get(0, "user");
	t = db->Get(0, "type");

	if(t == "regNewUser")
	{
		db->Query("UPDATE `users` SET `isactivated`='Y',`activated`=NOW() WHERE `login`='" + u + "';");

		if(cgi == NULL)
		{
			MESSAGE_ERROR("", "", "CCgi is empty in module CActivator::ActivateAction");

			throw CExceptionHTML("cgi error");
		}
		if(!cgi->SetProdTemplate("activate_user_complete.htmlt"))
		{
			MESSAGE_ERROR("", "", "template file was missing");
			throw CException("Template file was missing");
		}

		cgi->RegisterVariableForce("login", u);
		Delete(GetID());
	}
}

void CActivator::Delete()
{
	Delete(GetID());
}





