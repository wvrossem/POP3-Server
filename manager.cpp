#include <string>

#include <dvutil/strings.h>
#include <dvutil/enum2str.h>

#include "command.h"
#include "manager.h"

Manager::Manager (const std::string& name, const Dv::Props& config, Dv::Debugable* debug) :
thread_ (name, *this, config ("timeout"), 0, config ("debuglevel"), debug),
done_ (false), config_ (config), _maildrops (config ("top").str ()) { }

void
Manager::kill ()
{
  // Kill all the player threads.
  for ( Player::Set::iterator p = players_.begin(); p != players_.end(); ++p )
    ( *p )->kill();
  // And wait for them to finish.
  for ( Player::Set::iterator p = players_.begin(); p != players_.end(); ++p )
    ( *p )->wait();
  // Now kill the manager thread.
  thread_.kill();
  // And wait for it to finish.
  thread_.join();
}

void
Manager::remove_player (Player* p)
{
  if ( players_.count(p) )
    {
      log() << "remove " << p->name() << std::endl;
      roots_.erase(p);
      players_by_name_.erase(p->name());
      players_.erase(p);
      p->kill();
    }
}

std::string
Manager::operator()(const Player::Message& m) throw (std::runtime_error)
{
  // Status indicators
  static const std::string ok("+OK");
  static const std::string error("-ERR");

  static const std::string anonymous("anonymous");

  // logging output to server console
  log() << "< "
          << (m.first->name().size() ? m.first->name() : anonymous)
          << ":" << m.second << std::endl;

  // dump the message to an string stream for easy parsing
  std::istringstream iss(m.second);
  std::string command_word;
  // read first word of command, it should correspond to one
  // of the Command values.
  if ( !(iss >> command_word) )
    return "syntax error";
  try
    {
      // the following will throw if there is no value in Command
      // corresponding to the first word
      Command c = str2command(Dv::String::trim(command_word));
      if ( c != ADDPLAYER )
        {
          // Don't reply to players that have been killed (but apparently
          // don't know it yet).
          if ( players_.count(m.first) == 0 )
            throw std::runtime_error("abandon request from killed player");
        }
      switch (c)
        {
          case ADDPLAYER: // ADDPLAYER  -- add the sending player
            {
              players_.insert(m.first);
              _players_states.insert(std::pair<Player*, State > (m.first, Authorization));
              return ok;
            }
            break;
          case USER: // USER name -- sending player logs in with the username
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Authorization )
                {
                  std::string user_name;

                  // Did the user enter a username?
                  if ( iss >> user_name )
                    {
                      std::map<Player*, State>::iterator it(_players_states.find(m.first));
                      // The player's name is set find his or her maildrop later
                      it->first->set_name(user_name);
                      // The player enters the transaction name
                      if ( _maildrops.new_maildrop(m.first) )
                        {
                          it->second = Transaction;
                          return ok;
                        }
                      else
                        return error + " invalid username";
                    }
                  else
                    return error + " user <username>";
                }
              else
                return error;
            }
            break;
          case PASS: // PASS string -- sending players enters password
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Authorization )
                return ok;
              else
                return error;
            }
          case QUIT: // QUIT -- sending player quits
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              /* If th player is in the transaction state we need to clean up
               * his or her maildrop */
              if ( it->second == Transaction )
                {
                  _maildrops.remove_maildrop(m.first);
                  remove_player(m.first);
                  return ok;
                }
              else
                {
                  remove_player(m.first);
                  return ok;
                }
            }
            break;
          case STAT: // STAT -- get info on sending player's maildrop
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  std::ostringstream oss;
                  oss << ok << " " << maildrop->nr_of_messages()
                          << " " << maildrop->size();
                  return oss.str();
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case RETR: // RETR msg -- send message with mesg number ot sending player
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  std::ostringstream oss;
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  int msg_nr;
                  // Did the player enter a message number?
                  if ( (iss >> msg_nr) )
                    {
                      Message * message(maildrop->retrieve_message(msg_nr));
                      if ( message )
                        {
                          oss << ok << " " << message->size()
                                  << " octets" << std::endl
                                  << message->message_string() << std::endl;
                          return oss.str();
                        }
                      else
                        return error + " invalid message number";
                    }
                  else
                    return error + " retr <msg number>";
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case DELE: // DELE msg -- dlete message with message number from sending player's maildrop
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  int msg_nr;
                  if ( iss >> msg_nr )
                    {
                      bool flag(maildrop->delete_message(msg_nr));

                      if ( flag )
                        return ok + " message deleted";
                      else
                        return error + " invalid message number";
                    }
                  else
                    return error + " dele <msg number>";
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case LIST: // LIST [msg] -- get info on all or one message from sending playe's maildrop
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  std::ostringstream oss;
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  int msg_nr;
                  // Message number is given
                  if ( iss >> msg_nr )
                    {
                      Message * message(maildrop->retrieve_message(msg_nr));

                      if ( message )
                        {
                          oss << ok << " " << (*message);
                          return oss.str();
                        }
                      else
                        return error + " invalid message number";
                    }
                    // Give list info for each message
                  else
                    {
                      std::vector<Message*> msgs(maildrop->messages());

                      oss << ok << " " << maildrop->nr_of_messages() << " messages "
                              << "(" << maildrop->size() << " octets)" << std::endl;

                      for ( unsigned int i = 0; i < msgs.size(); i++ )
                        {
                          oss << (*msgs[i]) << std::endl;
                        }
                      return oss.str();
                    }
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case NOOP: // NOOP -- just returns positive response
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                return ok;
              else
                return error + " use 'user <username>' first";
            }
            break;
          case UIDL: // UIDL [msg] -- get uidl of all or one message of the sending player's maildrop
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  std::ostringstream oss;
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  int msg_nr;
                  // Message number is given
                  if ( iss >> msg_nr )
                    {
                      Message * message(maildrop->retrieve_message(msg_nr));

                      if ( message )
                        {
                          oss << msg_nr << " " << message->uidl() << "\n";
                          return ok + " " + oss.str();
                        }
                      else
                        return error + " invalid message number";
                    }
                    // Give uidl for each message in the maildrop
                  else
                    {
                      std::vector<Message*> msgs(maildrop->messages());

                      for ( unsigned int i = 0; i < msgs.size(); i++ )
                        {
                          oss << msgs.at(i)->number() << " " << msgs.at(i)->uidl() << std::endl;
                        }
                      return oss.str();
                    }
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case TOP: // TOP msg n -- same as retr but only top n lines
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  std::ostringstream oss;
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  int msg_nr;
                  if ( iss >> msg_nr )
                    {
                      int n;
                      if ( iss >> n )
                        {
                          Message * message(maildrop->retrieve_message(msg_nr));

                          if ( message )
                            {
                              oss << ok << " " << message->size() << " " << "\n";
                              oss << message->message_string(n) << "\n";
                              return oss.str();
                            }
                          else
                            return error + " invalid message number";
                        }
                      else
                        return error + " top <msg number> <nr of lines>";
                    }
                  else
                    return error + " top <msg number> <nr of lines>";
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case RSET:
            {
              std::map<Player*, State>::iterator it(_players_states.find(m.first));

              if ( it->second == Transaction )
                {
                  std::ostringstream oss;
                  Maildrop * maildrop(_maildrops.find_maildrop(m.first));

                  if ( maildrop )
                    {
                      std::vector<Message*> msgs(maildrop->messages(true));

                      for ( unsigned int i = 0; i < msgs.size(); i++ )
                        {
                          if ( msgs.at(i)->deleted() )
                            msgs.at(i)->deleted(false);
                        }
                      oss << ok << " maildrop has " << maildrop->nr_of_messages()
                              << " messages (" << maildrop->size() << " octets)";
                      return oss.str();
                    }
                }
              else
                return error + " use 'user <username>' first";
            }
            break;
          case SHUTDOWN: // SHUTDOWN -- shutdown server, only for convenience
            {
              done_ = true;
              return ok;
            }
          default:
            return command2str(c) + ": not yet implemented";
        }
    }
  catch (std::logic_error& e)
    {
      log() << e.what() << std::endl;
      return "syntax error";
    }
  return "impossible reply";
}
