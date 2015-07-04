/* 
 * File:   maildrop.h
 * Author: Wouter Van Rossem
 *
 */

#ifndef _MAILDROP_H
#define	_MAILDROP_H

#include <dvthread/thread.h>
#include <vector>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>

#include "message.h"

/** The Maildrop class represents a maildrop of a user.
 * All the messages are stored in a vector.
 * The path to the folder of the maildrop is stored
 */
class Maildrop
{
public:
  /** Constructor for Maildrop
   * @param folderpath String indicating where the folder is located
   * @exception std::runtime_error If the folder cannot be opened
   */
  Maildrop (std::string folderpath);

  /** Destructor for Maildrop
   * This will delete all the messages marked as deleted
   */
  virtual ~Maildrop ();

  /** Add a new message to the maildrop
   * @param message The message to add to the maildrop
   */
  void add_message (Message* message);

  /** Retrieve a message from the maildrop
   * @param msg_nr The number of the message
   * @return The actual message if found
   * @return false if the message is not found
   */
  Message* retrieve_message (int msg_nr) const;

  /** Delete a message from the maildrop
   * This does not actually delete the message but marks it as deleted
   * @param msg_nr The number of the message
   * @return A bool indicating if the operation has succeeded
   */
  bool delete_message (int msg_nr);

  /**
   * @param deleted Count messages marked as deleted?
   * @return the number of messages in the maildrop
   */
  int nr_of_messages (bool deleted = false) const;

  /** Return the size of the maildrop
   * @return The size of the maildrop in octets
   */
  unsigned long size () const;

  /** Get all the messages from the maildrop (including the ones marked
   * as deleted or not).
   * @param deleted Return messages marked as deleted
   * @return A vector containing the messages
   */
  std::vector<Message*> messages (bool deleted = false) const;

private:
  /** Private copy constructor */
  Maildrop (const Maildrop& orig);

  /* Vector containning all the messages in the maildrop */
  std::vector<Message*> _messages;

  /* String */
  std::string _folder_path;
};

#endif	/* _MAILDROP_H */

