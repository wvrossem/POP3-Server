/* 
 * File:   manager.h
 * Author: Dirk Vermeir
 * Edited by: Wouter Van Rossem
 *
 * Created on July 22, 2009, 10:54 AM
 */

#ifndef _MANAGER_H
#define	_MANAGER_H

#include <set>
#include <map>

#include <dvutil/debug.h>
#include <dvutil/props.h> // for config()
#include <dvthread/thread.h>
#include <dvthread/actor.h>

#include "player.h"
#include "maildrops.h"

/** The class that manages the maildrops. It communicates with
 * the players via an Dv::Thread::Actor thread which itself
 * uses a Dv::Thread::MailBox: the thread simply
 * waits for messages to be delivered to its mailbox and
 * then processes them using the Manager::operator() function.
 */
class Manager : public std::unary_function<Player::Message, std::string>,
public Player::Manager
{
public:

  enum State
  {
    Authorization,
    Transaction,
    Update
  };

  /** The following is pure virtual in Player::Manager and
   * the only thing a Player needs to know about the manager. */
  void request (Player::Message m, Player::MailBox* mbox)
  {
    thread_.request(m, mbox);
  }

  /** Function called by the Actor thread associated with this Manager.
   * The thread will read messages from its mailbox and then
   * process them using this function.
   * @param m message put by a player in the actor's mailbox.
   * @return a string that will be put into the client's reply
   *   mailbox, if any.
   * @exception std::runtime_error if the manager refuses
   *   for some reason to react to a request
   */
  std::string operator()(const Player::Message& m) throw (std::runtime_error);

  /** This function will return true after the manager (thread)
   * has processed a 'shutdown' command.
   * The main server program should check for this
   * function in its main loop. If true, it can
   * safely call Manager::kill.
   * @return true iff the manager may be killed
   * @see Manager::kill
   */
  bool done () const
  {
    return done_;
  }

  /** This function will
   * first kill all the players and then the manager thread.
   * @warning this function cannot be called from the
   * manager thread (otherwise, this would be suicide).
   * @see Manager::done
   */
  void kill ();

  /** The configuration of this program.
   * @return the configuration parameters as a Dv::Props object
   */
  const Dv::Props& config ()
  {
    return config_;
  }

  /** Constructor.
   * @param name of this manager
   * @param config contains configuration parameters
   * @param debug object which allows connected objects (e.g.
   * threads) to write debug info
   */
  Manager (const std::string& name, const Dv::Props& config, Dv::Debugable* debug = 0);

private:
  Manager (const Manager&);
  Manager & operator= (const Manager&);

  /** Remove all references to a player from the manager's database
   * and kill its thread.
   * The function is robust: calling it twice will have no effect
   * the second time. I.e. it is idempotent.
   * @param player pointer to player object
   */
  void remove_player (Player* player);

  /** The set of active players, including nameless ones. */
  Player::Set players_;
  /** The set of active players that have 'root' status (via a
   * successful 'su' command. */
  Player::Set roots_;
  /** A map that supports finding an active named player by name. */
  Player::Map players_by_name_;

  /** Type of the actor thread associated with Manager::manager. */
  typedef Dv::Thread::Actor<Manager> Actor;
  /** This is the manager's thread. */
  Actor thread_;

  /** Has the manager thread processed a shutdown command? */
  bool done_;
  /** The server configuration */
  Dv::Props config_;

  /** Return a pseudo-stream to write log info on.
   * @param i debug level, the pseudo stream is real only
   * if the actual debug level is at least @c i
   * @return a pseudo stream to write on
   */
  Dv::ostream_ptr& log (unsigned int i = 0)
  {
    return thread_.log(i);
  }

  /** The active maildrops */
  Maildrops _maildrops;

  /** A map of players and their current state */
  std::map<Player*, State> _players_states;
};

#endif	/* _MANAGER_H */

