/***************************************************************************
 *  Crimson Skies (CS-Mud) copyright (C) 1998-2015 by Blake Pell (Rhien)   *
 ***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  ROM 2.4 improvements copyright (C) 1993-1998 Russ Taylor, Gabrielle    *
 *  Taylor and Brian Moore                                                 *
 *                                                                         *
 *  In order to use any part of this Diku Mud, you must comply with        *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt' as well as the ROM license.  In particular,   *
 *  you may not remove these copyright notices.                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 **************************************************************************/

// System Specific Includes
#if defined(_WIN32)
    #include <sys/types.h>
    #include <time.h>
#else
    #include <sys/types.h>
    #include <sys/time.h>
    #include <time.h>
#endif

// General Includes
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "tables.h"

int flag_lookup(const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
            && !str_prefix(name, flag_table[flag].name))
            return flag_table[flag].bit;
    }

    return NO_FLAG;
}

int clan_lookup(const char *name)
{
    int clan;

    for (clan = 0; clan < MAX_CLAN; clan++)
    {
        if (LOWER(name[0]) == LOWER(clan_table[clan].name[0])
            && !str_prefix(name, clan_table[clan].name))
            return clan;
    }

    return 0;
}

int position_lookup(const char *name)
{
    int pos;

    for (pos = 0; position_table[pos].name != NULL; pos++)
    {
        if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
            && !str_prefix(name, position_table[pos].name))
            return pos;
    }

    return -1;
}

int sex_lookup(const char *name)
{
    int sex;

    for (sex = 0; sex_table[sex].name != NULL; sex++)
    {
        if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
            && !str_prefix(name, sex_table[sex].name))
            return sex;
    }

    return -1;
}

int size_lookup(const char *name)
{
    int size;

    for (size = 0; size_table[size].name != NULL; size++)
    {
        if (LOWER(name[0]) == LOWER(size_table[size].name[0])
            && !str_prefix(name, size_table[size].name))
            return size;
    }

    return -1;
}

/* returns race number */
int race_lookup(const char *name)
{
    int race;

    for (race = 0; race_table[race].name != NULL; race++)
    {
        if (LOWER(name[0]) == LOWER(race_table[race].name[0])
            && !str_prefix(name, race_table[race].name))
            return race;
    }

    return 0;
}

int item_lookup(const char *name)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(item_table[type].name[0])
            && !str_prefix(name, item_table[type].name))
            return item_table[type].type;
    }

    return -1;
}

int liq_lookup(const char *name)
{
    int liq;

    for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
        if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
            && !str_prefix(name, liq_table[liq].liq_name))
            return liq;
    }

    return -1;
}

HELP_DATA *help_lookup(char *keyword)
{
    HELP_DATA *pHelp;
    char temp[MIL], argall[MIL];

    argall[0] = '\0';

    while (keyword[0] != '\0')
    {
        keyword = one_argument(keyword, temp);
        if (argall[0] != '\0')
            strcat(argall, " ");
        strcat(argall, temp);
    }

    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
        if (is_name(argall, pHelp->keyword))
            return pHelp;

    return NULL;
}

HELP_AREA *had_lookup(char *arg)
{
    HELP_AREA *temp;
    extern HELP_AREA *had_list;

    for (temp = had_list; temp; temp = temp->next)
        if (!str_cmp(arg, temp->filename))
            return temp;

    return NULL;
}

/*
 * Looks up the key for the continent.
 */
int continent_lookup(const char *name)
{
    register int continent;

    for (continent = 0; continent_table[continent].name != NULL; continent++)
    {
        if (LOWER(name[0]) == LOWER(continent_table[continent].name[0])
            && !str_prefix(name, continent_table[continent].name))
            return continent;
    }

    return 0;
} // end int continent_lookup
