#include <dvutil/enum2str.h>
#include "command.h"

// Provides translation services for Command objects
// between the enum value and a corresponding string.
template<>
const char* Dv::Util::enum_parser<Command>::E_NAME ("Command");

template<>
Dv::Util::enum_parser<Command>::enum_entry
Dv::Util::enum_parser<Command>::enum_table[] = {
  { ADDPLAYER, "addplayer"},
  { USER, "user"},
  { PASS, "pass"},
  { STAT, "stat"},
  { QUIT, "quit"},
  { RETR, "retr"},
  { DELE, "dele"},
  { LIST, "list"},
  { NOOP, "noop"},
  { UIDL, "uidl"},
  { TOP, "top"},
  { RSET, "rset"},
  { SHUTDOWN, "shutdown"}
};

std::string
command2str (Command c)
{
  return Dv::Util::enum2str(c);
}

Command
str2command (const std::string& s) throw (std::logic_error)
{
  return Dv::Util::str2enum<Command > (s);
}
