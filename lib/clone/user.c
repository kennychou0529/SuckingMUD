/* vim:set ft=lpc: */
/* Last Indented: 1360770374 */
// mudlib:   Lil 
// file:     user.c
// purpose:  is the representation of an interactive (user) in the MUD

#include <globals.h>

inherit BASE;

private string name;

// Implements the actual tell to the user

void catch_tell(string str) {
    receive(str);
}

// replace this with a functioning version.

string query_cwd()
{
    return "";
}

// logon: move this to /daemon/login.c when login.c gets written.

void logon()
{
    printf("Welcome to %s.\n\n> ", MUD_NAME);
}

// query_name: called by various objects needing to know this user's name.

string query_name()
{
    return name;
}

void set_name(string arg)
{
    //  may wish to add security to prevent just anyone from changing
    //  someone else's name.
    name = arg;
}

// called by the present() efun (and some others) to determine whether
// an object is referred as an 'arg'.

int id(string arg)
{
    return (arg == query_name()) || base::id(arg);
}

void write_prompt()
{
    switch (query_ed_mode()) {
        case 0:
        case -2:
            write(":");
            break;

        case -1:
            write("> ");
            break;

        default:
            write("*\b");
            break;
    }
}

void start_ed(string file)
{
    write(ed_start(file, 0));
}

string process_input(string arg)
{
    if (query_ed_mode() != -1) {
        if (arg[0] != '!') {
            write(ed_cmd(arg));
            return 0;
        }
        arg = arg[1..];
    }
    // possible to modify player input here before driver parses it.
    return arg;
}

int commandHook(string arg)
{
    string cmd_path;
    object cobj;

    cmd_path = COMMAND_PREFIX + query_verb();

    cobj = load_object(cmd_path);
    if (cobj) {
        return (int)cobj->main(arg);
    } else {
        // maybe call an emote/soul daemon here
    }
    return 0;
}

// init: called by the driver to give the object a chance to add some
// actions (see the MudOS "applies" documentation for a better description).

void init()
{
    // using "" as the second argument to add_action() causes the driver
    // to call commandHook() for those user inputs not matched by other
    // add_action defined commands (thus 'commandHook' becomes the default
    // action for those verbs without an explicitly associated action).
    if (this_object() == this_player()) {
        add_action("commandHook", "", 1);
    }
}

// create: called by the driver after an object is compiled.

void create()
{
}

// receive_message: called by the message() efun.

void receive_message(string newclass, string msg)
{
    // the meaning of 'class' is at the mudlib's discretion
    receive(msg);
}

// setup: used to configure attributes that aren't known by this_object()
// at create() time such as living_name (and so can't be done in create()).

void setup()
{
    set_heart_beat(1);
    set_living_name(query_name());
    enable_commands();
    add_action("commandHook", "", 1);
}

// tell_room:  broadcasts a message to everyone in the same environment.
void tell_room(object ob, string msg)
{
    foreach (ob in all_inventory(ob) - ({ this_object() }))
        tell_object(ob, msg);
}

// net_dead: called by the gamedriver when an interactive player loses
// hir network connection to the mud.

void net_dead()
{
    ACCOUNT_D->set_offline(query_name());
    set_heart_beat(0);
    tell_room(environment(), query_name() + " is link-dead.\n");
}

// reconnect: called by the login.c object when a netdead player reconnects.

void reconnect()
{
    ACCOUNT_D->set_online(query_name());
    set_heart_beat(1);
    receive("\nReconnected.\n");
    tell_room(environment(), query_name() + " has reconnected.\n");
}
