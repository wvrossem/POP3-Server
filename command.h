/* 
 * File:   command.h
 * Author: Dirk Vermeir
 * Edited by: Wouter Van Rossem
 *
 */

#ifndef _COMMAND_H
#define	_COMMAND_H

#include <string>

enum Command
{
  ADDPLAYER, /* Add a new nameless player */
  USER, /* Player logs in with the username */
  PASS, /* Enter a password to log in */
  STAT, /* Get information of the messages in the player's maildrop */
  QUIT, /* The player disconnects */
  RETR, /* Retrieve a message from the player's maildrop */
  DELE, /* Delete a message from the player's maildrop */
  LIST, /* List a/the message(s) in the player's maildrop */
  NOOP, /* Positive respone */
  UIDL, /* Get the uidl from a/the message(s) */
  TOP, /* Get the top n lines from a message */
  RSET, /* Unmark all messages as deleted */
  SHUTDOWN /* Shut down the server */
};

/** Return the string representation of a command. This
 * representation is defined in the file @c command.C .
 * @param command to find the string representation for
 * @return string representation of command
 */
std::string command2str (Command command);

/** Find the command corresponding to a string. The string
 * must be equal to the string representation of the command.
 * @param s string to parse
 * @return command that is represented by @c s
 * @exception std::logic_error if there is no command represented by  @c s
 */
Command str2command (const std::string& s) throw (std::logic_error);

#endif	/* _COMMAND_H */

