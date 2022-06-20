#ifndef __PI_SQL_QUERIES__H__
#define __PI_SQL_QUERIES__H__

#include <string>


inline auto Check_isUserExists_sqlquery(const string &login, const string &pass) -> string
{

	return (
"SELECT `users`.`id` FROM `users` "
"INNER JOIN `users_passwd` ON `users_passwd`.`userID`=`users`.`id` "
	"WHERE (`users`.`login`=\"" + login + "\"  OR  `users`.`email`=\"" + login + "\")  AND (`users_passwd`.`isActive`='true') AND `users_passwd`.`passwd`=\"" + pass + "\""
	);
}	

#endif
