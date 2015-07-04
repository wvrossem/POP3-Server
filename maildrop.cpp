#include "maildrop.h"

// See man 3 for information on opendir and readdir

Maildrop::Maildrop (std::string folderpath) :
_folder_path (folderpath)
{
  using namespace std;

  DIR *dp; // pointer to the directory
  struct dirent *ep;

  dp = opendir(_folder_path.c_str());
  // Can we open the directory?
  if ( dp != NULL )
    {
      // Each message will get a subsequent number
      int msg_counter(0);

      /* Add all the messages file in the folder to the maildrop
       * readdir returns pointer to dirent */
      while ( (ep = readdir(dp)) )
        {
          // needed so that messages won't be created for "." and ".."
          if ( !(ep->d_name[0] == '.') )
            {
              _messages.push_back(new Message(msg_counter,
                                              /* ep->d_name retuns the name of
                                               * current file */
                                              _folder_path + ep->d_name));
              msg_counter++;
            }
        }
      closedir(dp);
    }
  else
    throw runtime_error("unable to open maildrop folder");
}

Maildrop::~Maildrop ()
{
  for ( int i = 0; i < nr_of_messages(true); i++ )
    {
      delete _messages.at(i); /* If the message was marked as deleted,
                               * it will be deleted from the filesystem now */
    }
}

void Maildrop::add_message (Message* message)
{
  _messages.push_back(message);
}

Message* Maildrop::retrieve_message (int msg_nr) const
{
  if ( msg_nr < nr_of_messages(true) )
    {
      Message* msg(_messages.at(msg_nr));

      if ( msg->deleted() )
        return false;
      else
        return msg;
    }
  else
    return false;
}

bool Maildrop::delete_message (int msg_nr)
{
  if ( msg_nr <= nr_of_messages(true) )
    {
      Message* msg(_messages.at(msg_nr));

      if ( msg->deleted() )
        return false;
      else
        {
          msg->deleted(true);
          return true;
        }
    }
  else
    return false;
}

int Maildrop::nr_of_messages (bool deleted) const
{
  /* If deleted = true we just give the size of the messages vector
   * Otherwise we calculate size */
  if ( !deleted )
    {
      int nr_of_msgs(0);

      for ( unsigned int i = 0; i < _messages.size(); i++ )
        {
          if ( !_messages.at(i)->deleted() )
            nr_of_msgs++;
        }
      return nr_of_msgs;
    }
  else
    return _messages.size();
}

unsigned long Maildrop::size () const
{
  unsigned long size;

  for ( int i = 0; i < nr_of_messages(false); i++ )
    {
      if ( !_messages.at(i)->deleted() )
        size += _messages.at(i)->size();
    }
  return size;
}

std::vector<Message*> Maildrop::messages (bool deleted) const
{
  std::vector<Message*> msgs;

  for ( int i = 0; i < nr_of_messages(true); i++ )
    {
      if ( !_messages.at(i)->deleted() || deleted )
        msgs.push_back(_messages.at(i));
    }
  return msgs;
}
