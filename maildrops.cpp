#include "maildrops.h"

Maildrops::Maildrops (std::string folder_path) :
_folder_path (folder_path) { }

Maildrops::~Maildrops ()
{
  for ( Map::iterator it = _maildrops.begin(); it != _maildrops.end(); it++ )
    {
      delete (*it).second;
    }
}

Maildrop* Maildrops::find_maildrop (const Player* player) const
{
  Maildrop * maildrop(_maildrops.find(player->name())->second);

  if ( maildrop )
    return maildrop;
  else
    return false;
}

bool Maildrops::new_maildrop (const Player* player)
{
  using namespace std;
  DIR *dp; // pointer to the directory
  string path("/exports/home/wvrossem/pop3/maildrops/" + player->name() + "/");

  dp = opendir(path.c_str());
  // Can we open the directory? If not, wrong username
  if ( dp != NULL )
    {
      pair<Map::iterator,bool> ret(_maildrops.insert
                                    (Pair(player->name(), new Maildrop(path))));
      closedir(dp);
      /* Ret is a pair with as first element an iterator pointer pointing to
       * the newly inserted element or the element with the same key.
       * The second element is true if the insert was successful and
       * false if an element with that key already existed
       */
      return ret.second;
    }
  else
    return false;
}

void Maildrops::remove_maildrop (const Player* player)
{
  Map::iterator it(_maildrops.find(player->name()));
  delete it->second;
  _maildrops.erase(it);
}
