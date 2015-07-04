/* 
 * File:   message.h
 * Author: Wouter Van Rossem
 *
 */

#ifndef _MESSAGE_H
#define	_MESSAGE_H

#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstddef>
#include <algorithm>
#include <math.h>

/** The Message class represents a message in a maildrop
 * Operations on the message file are executed by opening
 * a filestream with the _file_path as path
 */
class Message
{
public:
  /** Constructor for Message
   * @param number The number of this message
   * @param filepath String representing the path to the message file
   */
  Message (unsigned int number, const std::string& filepath);

  /** Destructor for message
   * If the message was marked as deleted, it will be deleted now
   * @exception std::runtime_error If the message file can't be deleted
   */
  virtual ~Message ();

  /**
   * @return Bool indicating if the message is marked as deleted
   */
  bool deleted () const
  {
    return _deleted;
  }

  /** Overloaded deleted method
   * @param mark Mark or unmark the message as deleted
   */
  void deleted (bool mark)
  {
    _deleted = mark;
  }

  /**
   * @return The number of the message
   */
  int number () const
  {
    return _number;
  }

  /**
   * @return the size of the file in bytes
   */
  unsigned long size () const;

  /**
   * @return A string which contains the text in the message file
   * @exception std::runtime_error If the message file can't be opened
   */
  std::string message_string () const;

  /** Overloaded message_string method
   * @param n The top n lines of the message only
   * @return A string containing the top n lines of the message file
   */
  std::string message_string (int n) const;

  /**
   * @return A string which is the filename = uidl in this implementation
   */
  std::string uidl () const;

  /** Overloaded << operator, sends some information of the message 
   * to the ostream
   */
  friend std::ostream& operator<< (std::ostream& os, const Message& msg);

private:
  /* The number of the message */
  unsigned int _number;

  /* A bool indicating if the message is marked a deleted */
  bool _deleted;

  /* String containing the path to the message file */
  std::string _file_path;
};

#endif	/* _MESSAGE_H */
