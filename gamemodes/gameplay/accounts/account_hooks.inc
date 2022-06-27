#define 	DEFAULT_POS_X 		1958.3783
#define 	DEFAULT_POS_Y 		1343.1572
#define 	DEFAULT_POS_Z 		15.3746
#define 	DEFAULT_POS_A 		270.1425

#include <YSI_Coding\y_hooks>

new g_MysqlRaceCheck[MAX_PLAYERS];

 
hook OnGameModeInit(){
    new MySQLOpt: option_id = mysql_init_options();
	mysql_set_option(option_id, AUTO_RECONNECT, true); // it automatically reconnects when loosing connection to mysql server
    g_SQL = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, option_id);

    if (g_SQL == MYSQL_INVALID_HANDLE || mysql_errno(g_SQL) != 0)
	{
		print("MySQL connection failed. Server is shutting down.");
		SendRconCommand("exit"); // close the server if there is no connection
		return 1;
	}
	print("mysql connection succesful");

	SetupPlayerTable();
} 

hook OnGameModeExit()
{
	// save all player data before closing connection
	for (new i = 0, j = GetPlayerPoolSize(); i <= j; i++) // GetPlayerPoolSize function was added in 0.3.7 version and gets the highest playerid currently in use on the server
	{
		if (IsPlayerConnected(i))
		{
			// reason is set to 1 for normal 'Quit'
			
		}
	}

	mysql_close(g_SQL);
	return 1;
}

hook OnPlayerConnect(playerid){
    g_MysqlRaceCheck[playerid]++;
    static const empty_player[E_PLAYERS];
	Player[playerid] = empty_player;
    GetPlayerName(playerid, Player[playerid][Name], MAX_PLAYER_NAME);
    new query[103];
    mysql_format(g_SQL, query, sizeof query, "SELECT * FROM `players` WHERE `username` = '%e' LIMIT 1", Player[playerid][Name]);
	mysql_tquery(g_SQL, query, "OnPlayerDataLoaded", "dd", playerid, g_MysqlRaceCheck[playerid]);


	
}

hook OnPlayerDisconnect(playerid, reason){
	g_MysqlRaceCheck[playerid]++;

	UpdatePlayerData(playerid, reason);

	// if the player was kicked (either wrong password or taking too long) during the login part, remove the data from the memory
	if (cache_is_valid(Player[playerid][Cache_ID]))
	{
		cache_delete(Player[playerid][Cache_ID]);
		Player[playerid][Cache_ID] = MYSQL_INVALID_CACHE;
	}

}

forward OnPlayerDataLoaded(playerid, race_check);
public OnPlayerDataLoaded(playerid, race_check){
	if (race_check != g_MysqlRaceCheck[playerid]) return Kick(playerid);
	if(cache_num_rows() > 0)
	{
		cache_get_value(0, "password", Player[playerid][Password], 65);
		cache_get_value(0, "salt", Player[playerid][Salt], 17);

		Player[playerid][Cache_ID] = cache_save();

		inline LoginDialog(response, listitem, string:inputtext[]){
			new hashed_pass[65];
			SHA256_PassHash(inputtext, Player[playerid][Salt], hashed_pass, 65);
			if(strcmp(hashed_pass, Player[playerid][Password]) == 0){
				cache_set_active(Player[playerid][Cache_ID]);

				AssignPlayerData(playerid);

				cache_delete(Player[playerid][Cache_ID]);
				Player[playerid][Cache_ID] = MYSQL_INVALID_CACHE;

				SetSpawnInfo(playerid, NO_TEAM, 0, Player[playerid][X_Pos], Player[playerid][Y_Pos], Player[playerid][Z_Pos], Player[playerid][A_Pos], 0, 0, 0, 0, 0, 0);
				SpawnPlayer(playerid);
			}
			else{
				//Kick(playerid);
				Dialog_ShowCallback(playerid, using inline LoginDialog, DIALOG_STYLE_PASSWORD, "Giriş", "Hesap kayıatlı, lütfen şifrenizi girin.", "Giriş");
			}
		}
		Dialog_ShowCallback(playerid, using inline LoginDialog, DIALOG_STYLE_PASSWORD, "Giriş", "Hesap kayıatlı, lütfen şifrenizi girin.", "Giriş");
	}
	else{
		inline RegisterDialog(response, listitem, string:inputtext[]){
			if (!response) return Kick(playerid);
			if (strlen(inputtext) < 5) Dialog_ShowCallback(playerid, using inline RegisterDialog, DIALOG_STYLE_PASSWORD, "Kayıt", "Şifreniz 5'ten fazla sayıda karakter içermelidir.", "Kaydol");
			for (new i = 0; i < 16; i++) Player[playerid][Salt][i] = random(94) + 33;
			SHA256_PassHash(inputtext, Player[playerid][Salt], Player[playerid][Password], 65);
			new query[221];
			mysql_format(g_SQL, query, sizeof query, "INSERT INTO `players` (`username`, `password`, `salt`) VALUES ('%e', '%s', '%e')", Player[playerid][Name], Player[playerid][Password], Player[playerid][Salt]);
			mysql_tquery(g_SQL, query, "OnPlayerRegister", "d", playerid);
			
		}
		Dialog_ShowCallback(playerid, using inline RegisterDialog, DIALOG_STYLE_PASSWORD, "Title", "register dialog", "register");
		

	}

	
} 
forward OnPlayerRegister(playerid);
public OnPlayerRegister(playerid)
{
	Player[playerid][ID] = cache_insert_id();
	Player[playerid][faction_id] = -1;
	inline RegCompleted(playerid, response, string:inputtext){

	}

	Dialog_ShowCallback(playerid, using inline RegCompleted, DIALOG_STYLE_MSGBOX, "Registration", "Account successfully registered, you have been automatically logged in.", "Okay", "");

	Player[playerid][IsLoggedIn] = true;

	Player[playerid][X_Pos] = DEFAULT_POS_X;
	Player[playerid][Y_Pos] = DEFAULT_POS_Y;
	Player[playerid][Z_Pos] = DEFAULT_POS_Z;
	Player[playerid][A_Pos] = DEFAULT_POS_A;

	SetSpawnInfo(playerid, NO_TEAM, 0, Player[playerid][X_Pos], Player[playerid][Y_Pos], Player[playerid][Z_Pos], Player[playerid][A_Pos], 0, 0, 0, 0, 0, 0);
	SpawnPlayer(playerid);
	return 1;
}

UpdatePlayerData(playerid, reason)
{
	
	GetPlayerPos(playerid, Player[playerid][X_Pos], Player[playerid][Y_Pos], Player[playerid][Z_Pos]);
	GetPlayerFacingAngle(playerid, Player[playerid][A_Pos]);


	new query[145];
	mysql_format(g_SQL, query, sizeof(query), "UPDATE `players` SET `x` = %f, `y` = %f, `z` = %f, `angle` = %f, `interior` = %d WHERE `player_id` = %d LIMIT 1", Player[playerid][X_Pos], Player[playerid][Y_Pos], Player[playerid][Z_Pos], Player[playerid][A_Pos], GetPlayerInterior(playerid), Player[playerid][ID]);
	mysql_tquery(g_SQL, query);
	return 1;
}

AssignPlayerData(playerid)
{
	cache_get_value_int(0, "player_id", Player[playerid][ID]);
	cache_get_value_int(0, "faction_id", Player[playerid][faction_id]);

	cache_get_value_float(0, "x", Player[playerid][X_Pos]);
	cache_get_value_float(0, "y", Player[playerid][Y_Pos]);
	cache_get_value_float(0, "z", Player[playerid][Z_Pos]);
	cache_get_value_float(0, "angle", Player[playerid][A_Pos]);
	cache_get_value_int(0, "interior", Player[playerid][Interior]);
	return 1;
}

SetupPlayerTable()
{
	mysql_tquery(g_SQL, "CREATE TABLE IF NOT EXISTS `players` (`player_id` int(11) NOT NULL AUTO_INCREMENT,`username` varchar(24) NOT NULL,`password` char(64) NOT NULL,`salt` char(16) NOT NULL,`kills` mediumint(8) NOT NULL DEFAULT '0',`deaths` mediumint(8) NOT NULL DEFAULT '0',`x` float NOT NULL DEFAULT '0',`y` float NOT NULL DEFAULT '0',`z` float NOT NULL DEFAULT '0',`angle` float NOT NULL DEFAULT '0',`interior` tinyint(3) NOT NULL DEFAULT '0', PRIMARY KEY (`id`), UNIQUE KEY `username` (`username`))");
	return 1;
}

forward errormsg(playerid, message[]);
public errormsg(playerid, message[]){
	new tmp[128];
	format(tmp, sizeof tmp, "%d", message);
	return SendClientMessage(playerid, 0xFFFFFF, tmp);
}