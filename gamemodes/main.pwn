#include <a_samp>

#define COLOR_BLUE 0xFFFFFF
#include <a_mysql>
#include <sscanf2>

#include	<YSI_Coding/y_inline>
#include	<YSI_Extra/y_inline_mysql>
#include	<YSI_Visual/y_dialog>
#include    <YSI_Visual/y_commands>



#include <gamemodes\gameplay\data\data_main>
#include <gamemodes\gameplay/account_main>
#include <gamemodes\gameplay/faction_main>



#include <gamemodes\gameplay\birlik_commands>


main(){
    printf("%d", Faction_PlayerHasFaction(0));
}
