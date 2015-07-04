/* 
 * File:   maildrops.h
 * Author: Wouter Van Rossem
 *
 */

#ifndef _MAILDROPS_H
#define	_MAILDROPS_H

#include <dvthread/thread.h>
#include <map>
#include <string>
#include <utility>
#include <string>
#include <sstream>

#include "maildrop.h"
#include "player.h"

/** The Maildrops class manages the different maildrops
 */
class Maildrops
{
public:
  /** Constructor for Maildrops
   * @param folder_path String representing the folder where
   *                    the maildrops are located
   */
  Maildrops (std::string folder_path);

  /** Destructor for Maildrops
   * Deletes all the maildrops in the map
   */
  virtual ~Maildrops ();

  /* Type of pair from name of the maildrop to the corresponding maildrop */
  typedef std::pair<std::string, Maildrop*> Pair;

  /* Type of map from name of the maildrop to the corresponding maildrop */
  typedef std::map<std::string, Maildrop*> Map;

  /** Find the maildrop of the given player
   * @param player Player who's maildrop we need to find
   * @return The maildrop of the player
   */
  Maildrop* find_maildrop (const Player* player) const;

  /** Add a maildrop for the player
   * The name of the player will be used to find the path to the maildrop
   * @param player Player who's maildrop we need to create
   * @return A bool indicating if the operation succeeded
   */
  bool new_maildrop (const Player* player);

  /** Remove the maildrop of the player
   * @param player Player who's maildrop we need to delete
   */
  void remove_maildrop (const Player* player);

private:
  /* Private copy constructor */
  Maildrops (const Maildrops& orig);

  /** Collection of maildrops
   * Key = name of the maildrop
   * Value = the maildrop 
   */
  Map _maildrops;

  /* String indicating the path to the maildrops */
  std::string _folder_path;
};

#endif	/* _MAILDROPS_H */
