/* 
 * File:   player.h
 * Author: Dirk Vermeir
 * Edited by: Wouter Van Rossem
 *
 */

#ifndef _PLAYER_H
#define	_PLAYER_H

#include <set>
#include <map>
#include <sstream>

#include <dvutil/shared_ptr.h>
#include <dvnet/socket.h>
#include <dvthread/thread.h>
#include <dvthread/mailbox.h>

/** The Player class represents a user connected to the server.  It is
 * also a thread. The class is very simple and reusable: its main
 * function (the one executed by the thread) simply reads commands
 * from the socket connection (with the 'client/user') and sends them
 * (via the Manager::request function) to the manager for processing,
 * after which it displays the reply. It also reads replies from the
 * manager from its regular mailbox and for an 'out of band data'
 * mailbox (such data are sent to the player without a corresponding
 * request from the player). All these replies are sent back to the
 * user via the socket.
 */
class Player : public Dv::Thread::Thread
{
public:
  /** The type of a message that the player sends out.
   * It consists of a pointer to the player and the
   * actual message (a string). The former is useful for allowing
   * the player's manager to request extra services from the player, e.g.
   * kill it.
   */
  typedef std::pair<Player*, std::string> Message;

  /** Type of mailbox where a player receives information.*/
  typedef Dv::Thread::MailBox<std::string> MailBox;

  /** Abstract class representing the player's manager. All input
   * from a player is passed on to its manager.
   */
  class Manager
  {
  public:
    virtual void request (Player::Message, MailBox* = 0) = 0;
  };

  /** Factory method to create a new Player. This function also
   * reports the creation to the manager via a 'newplayer' command.
   *
   * Note that the player will autodestruct after the thread
   * finishes its main() function.
   *
   * Note also that a player will never wait indefinitely for any
   * event: the calls to the manager time out, as do the read calls
   * to the socket connection. Thus, a player that is killed will
   * always notice this after a while.
   *
   * @param manager of this player
   * @param so socket connection to player
   * @param delay millisecs that the player will wait for the
   *   manager to reply before throwing an exception
   * @param delay millisecs that the player will wait for the
   * @param debug_level
   * @param debug
   * @return pointer to new Player object
   */
  static Player* make (Manager& manager,
                       Dv::shared_ptr<Dv::Net::Socket> so, size_t delay, size_t
                       debug_level, Dv::Debugable* debug);

  /** Type of sets of players. */
  typedef std::set<Player*> Set;
  /** Type of map from player names to players. */
  typedef std::map<std::string, Player*> Map;

  /** Get the name of this player.
   * @return name of this player
   */
  const std::string& name () const
  {
    return name_;
  }

  /** Set the name of this player.
   * @param name new name of this player
   */
  void set_name (const std::string& name)
  {
    name_ = name;
  }

  /** Send out-of-band data to this player by storing them
   * in the incoming_ mailbox.
   */
  void put (const std::string& text)
  {
    incoming_.put(text);
  }
private:

  /** Destructor */
  ~Player () { }
  Player (const Player&);
  Player & operator= (const Player&);

  /** Main function. Note that this function should not return unless
   * the player's manager has been informed.
   */
  virtual int main ();
  /** Constructor.
   * @param manager of this player
   * @param so socket connection of this player with the client
   * @param delay millisecs that the player will wait for the
   *   manager to reply before throwing an exception
   * @param debug_level only if the master debug level is larger
   *   than this level will debug output be generated
   * @param debug object (may be 0)
   */
  Player (Manager& manager, Dv::shared_ptr<Dv::Net::Socket> so, size_t delay,
          size_t debug_level, Dv::Debugable* debug);


  /** Clean up before exiting this thread, in particular send
   * a 'quit' command to the manager. */
  void quit ();

  /** Read a line from a socket that may time out.
   * While waiting for input, the function also checks any
   * messages available in @c incoming_ and displays them
   * on @c so.
   * If successful, The read line is also trimmed (i.e. white
   * space is removed from both ends).
   * @param so socket to read from
   * @param line to read
   * @return 0 if a line was successfully read
   * @return 2 if the player thread was killed (and input is ignored)
   * @return 1 if the read did not succeed, e.g. because of an I/O error
   */
  int get_line (Dv::Net::Socket& so, std::string& line);

  /** Send a message to the player's manager and wait for a reply.
   * @param message to send
   * @return reply from manager on this message
   * @sa Player::Manager::request
   */
  std::string query_manager (const std::string& message);

  /** Manager of this player. */
  Manager& manager_;

  /** Connection to user/client. */
  Dv::shared_ptr<Dv::Net::Socket> so_;
  /** Mailbox used to communicate with the manager: the player
   * forwards commands to the manager who will store replies
   * in this mailbox.
   */
  MailBox mbox_;
  /** Mailbox for incoming 'out of band' data. */
  MailBox incoming_;
  /** Name of the player. */
  std::string name_;
  /** Delay used when communicating with the manager or when doing
   * I/O operations. */
  size_t delay_;
};

#endif	/* _PLAYER_H */

