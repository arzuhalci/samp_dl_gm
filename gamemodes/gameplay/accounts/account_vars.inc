
enum E_PLAYERS
{
	ID,
	Name[MAX_PLAYER_NAME],
	Password[65], // the output of SHA256_PassHash function (which was added in 0.3.7 R1 version) is always 256 bytes in length, or the equivalent of 64 Pawn cells
	Salt[17],

	Interior,
	Float: X_Pos,
	Float: Y_Pos,
	Float: Z_Pos,
	Float: A_Pos,

	Cache: Cache_ID,
	bool: IsLoggedIn,
	LoginAttempts,
	LoginTimer,
	faction_id
};

new Player[MAX_PLAYERS][E_PLAYERS];
