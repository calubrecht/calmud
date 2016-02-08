/***************************************************************************
 *  Crimson Skies (CS-Mud) copyright (C) 1998-2016 by Blake Pell (Rhien)   *
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

/***************************************************************************
 *                                                                         *
 *  Rangers                                                                *
 *                                                                         *
 *  Rangers are warriors who are associated with the wisdom of nature.     *
 *  They tend to be wise, cunning, and perceptive in addition to being     *
 *  skilled woodsmen. Many are skilled in stealth, wilderness survival,    *
 *  beast-mastery, herbalism, and tracking.  They are adept at swordplay   *
 *  and fight much better in a wilderness setting.  Rangers typically have *
 *  keen eye sight and are excellent at hiding in forest areas.            *
 *                                                                         *
 *  Skills                                                                 *
 *                                                                         *
 *    - Acute Vision                                                       *
 *    - Butcher                                                            *
 *    - Bark Skin                                                          *
 *    - Self Growth                                                        *
 *    - Quiet Movement                                                     *
 *    - Camp                                                               *
 *    - Track (hunt.c)                                                     *
 *                                                                         *
 ***************************************************************************/

// General Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/*
 * Skill a ranger can use to butcher steaks from PC and NPC corpses.  Yum.
 * This will leverage the timer callback methods.
 */
void do_butcher(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    OBJ_DATA *steak;
    char buf[MAX_STRING_LENGTH];

    // What do they want to butcher?
    if (IS_NULLSTR(argument))
    {
        send_to_char("What do you want to butcher?\r\n", ch);
        return;
    }

    // Find it in the room
    obj = get_obj_list(ch, argument, ch->in_room->contents);

    if (obj == NULL)
    {
        send_to_char("You can't find it.\r\n", ch);
        return;
    }

    // Is it a corpse of some kind?
    if ((obj->item_type != ITEM_CORPSE_NPC) && (obj->item_type != ITEM_CORPSE_PC))
    {
        send_to_char("You can only butcher corpses.\r\n", ch );
        return;
    }

    // Instead of dumping items to the ground, we'll just make them clean the corpse first
    if (obj->contains)
    {
       send_to_char("You will need to remove all items from the corpse first.\r\n",ch);
       return;
    }

    switch (ch->substate)
    {
        default:
            add_timer(ch, TIMER_DO_FUN, number_range(12, 18), do_butcher, 1, argument);
            sprintf(buf, "You begin to butcher a steak from %s\r\n", obj->short_descr);
            send_to_char(buf, ch);
            act("$n begins to butcher a steak from $p.", ch, obj, NULL, TO_ROOM);
            return;
        case 1:
            break;
        case SUB_TIMER_DO_ABORT:
            ch->substate = SUB_NONE;
            send_to_char("You stop butchering.\r\n", ch);
            act("$n stops butchering.", ch, NULL, NULL, TO_ROOM);
            return;
    }

    ch->substate = SUB_NONE;

    // The moment of truth, do they fail and mutilate the corpse?
    if (!check_skill_improve(ch, gsn_butcher, 3, 3))
    {
        send_to_char("You fail your attempt to butcher the corpse.\r\n", ch);
        act( "$n has failed their attempt at butchering.", ch, NULL, NULL, TO_ROOM );
        separate_obj(obj);
        extract_obj(obj);
        return;
    }

    // Require that an object with a VNUM of 27 is created (as the steak), I didn't
    // feel the need to make a global constant when it's only used once here.
    int count = number_range(1, 4);
    int x = 0;

    for (x = 1; x <= count; x++)
    {
        steak = create_object(get_obj_index(27), 0);
        obj_to_char(steak, ch);
    }

    // Show the player and the room the spoils (not spoiled)
    sprintf(buf, "$n has prepared %d steak%s.", count, (count > 1 ? "s" : ""));
    act(buf, ch, NULL, NULL, TO_ROOM );
    sprintf(buf, "You have successfully prepared %d steak%s", count, (count > 1 ? "s" : ""));
    act(buf, ch, NULL, NULL, TO_CHAR );

    // Seprate and extract the corpse from the room.
    separate_obj(obj);
    extract_obj(obj);

    return;

} // end do_butcher

/*
 * The bandage skill will allow a ranger or healer to attempt to bandage up themselves
 * or another person/mob who is critically injured.  It will restore a small amount
 * of HP although it could cause a slight damage itself if done incorrectly.
 *
 * This skill orginated from from UltraEnvy/Magma Mud by Xangis.  Conversion to CS-Mud
 * and additional updates to the code are by Rhien.
 */
void do_bandage(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int chance;
    int change;

    if (IS_NPC(ch) || ch->level < skill_table[gsn_bandage]->skill_level[ch->class])
    {
        send_to_char("You don't know how to bandage!\r\n", ch);
        return;
    }

    if (IS_NULLSTR(argument))
    {
        send_to_char( "Bandage whom?\r\n", ch );
        return;
    }

    if (!(victim = get_char_room(ch, argument)))
    {
        send_to_char("They're not here.\r\n", ch);
        return;
    }

    // The original implementation was for > 0 which meant only stunned people
    // could be bandaged, we'll allow this to be used on people under 100 to
    // make it a little more useful, those people are still pretty hurt.
    if (victim->hit > 100)
    {
        if (victim == ch)
        {
            send_to_char("You do not need any bandages currently.\r\n", ch);
            return;
        }
        else
        {
            send_to_char("They do not need your help.\r\n", ch);
            return;
        }
    }

    // Make sure they have enough movement to perform this, even though it takes
    // less than 50 we'll make them have more than 50.
    if (ch->move < 50)
    {
        send_to_char("You are too tired.\r\n", ch);
        return;
    }

    chance = get_skill(ch, gsn_bandage);

    // Ranger's and Healer's get a bonus
    switch (ch->class)
    {
        default:
            break;
        case RANGER_CLASS_LOOKUP:
            chance += 4;
            break;
        case HEALER_CLASS_LOOKUP:
            chance += 6;
            break;
    }

    /* Don't allow someone doing more than 1 pt. of damage with bandage. */
    change = (UMAX(chance - number_percent(), -1) / 20) + 1;

    // The wait period for the user of the skill.
    WAIT_STATE(ch, skill_table[gsn_bandage]->beats);

    if(change < 0)
    {
        send_to_char("You just made the problem worse!\n\r", ch);
        act("$n tries bandage you but your condition only worsens.", ch, NULL, victim, TO_VICT);
        act("$n tries bandage $N but $S condition only worsens.", ch, NULL, victim, TO_NOTVICT);
        check_improve(ch, gsn_bandage, FALSE, 1);
    }
    else if(change > 0)
    {
        send_to_char("You manage to fix them up a bit.\n\r", ch);
        act("$n bandages you.", ch, NULL, victim, TO_VICT);
        act("$n bandages $N.", ch, NULL, victim, TO_NOTVICT);
        check_improve(ch, gsn_bandage, TRUE, 1);
    }
    else
    {
        send_to_char("Your bandaging attempt had no effect.\n\r", ch);
        act("$n tries to bandage you but the wounds are too great.", ch, NULL, victim, TO_VICT);
        act("$n tries to bandage $N but is unable to have any effect.", ch, NULL, victim, TO_NOTVICT);
    }

    // Add the additional health
    victim->hit += change;

    // Have the act of bandaging remove some move from the character using it.
    ch->move -= change;

    // Update their position in case they were stunned.
    update_pos(victim);

    return;

} // end do_bandage

/*
 * Bark skin - enhances a characters armor class.
 */
void spell_bark_skin(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

    if (is_affected(ch, sn))
    {
        if (victim == ch)
        {
            // Remove the affect so it can be re-added to yourself
            affect_strip(victim, sn);
        }
        else
        {
            act("$N's skin already has the toughness of bark.", ch, NULL, victim, TO_CHAR);
            return;
        }
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = UMIN (UMAX(15, level), 50);
    af.location = APPLY_AC;
    af.modifier = -10;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    act("$n's skin gains the texture and toughness of bark.", victim, NULL, NULL, TO_ROOM);
    send_to_char("Your skin gains the texture and toughness of bark.\r\n", victim);
    return;

} // end spell_bark_skin

/*
 * This spell allows the ranger to channel the powers of nature to promote growth.
 * This spell should be castable only on one's self.
 */
void spell_self_growth(int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

    if (is_affected(victim, sn))
    {
        if (victim == ch)
        {
            // Remove the affect so it can be re-added to yourself
            affect_strip(victim, sn);
        }
        else
        {
            act("$N is already instilled with self growth.", ch, NULL, victim, TO_CHAR);
            return;
        }
    }

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_CON;
    af.modifier = 1;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    send_to_char("You feel vitalized.\r\n", victim);
    act("$n's looks more vitalized.", victim, NULL, NULL, TO_ROOM);
    return;

} // end self growth

/*
 * Quiet movement - Will allow rangers to move mostly undetected through
 * various types of wilderness terrain.  Will not work in cities, water,
 * underwater, etc.
 */
void do_quiet_movement(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

    send_to_char("You attempt to move quietly.\n\r", ch );
    affect_strip(ch, gsn_quiet_movement);

    if (is_affected(ch, gsn_quiet_movement))
        return;

    if (CHANCE_SKILL(ch, gsn_quiet_movement))
    {
        af.where = TO_AFFECTS;
        af.type = gsn_quiet_movement;
        af.level = ch->level;
        af.duration = ch->level;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = 0;
        affect_to_char(ch, &af);

        check_improve(ch, gsn_quiet_movement, TRUE, 3);
    }
    else
    {
        check_improve(ch, gsn_quiet_movement, FALSE, 3);
    }

    return;
} // end do_quiet_movement

/*
 * Camping - allows a ranger to camp in certain wilderness type areas.  Will bring
 * increased regen to the ranger and anyone with them that is in their group while
 * camp is setup.  Camp will broken when a ranger leaves the room, breaks camp, is
 * attacked or attacks.
 */
void do_camp(CHAR_DATA *ch, char *argument)
{
    send_to_char("Coming soon...\r\n", ch);
}
