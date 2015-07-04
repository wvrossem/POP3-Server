#include <dvutil/strings.h> // for Dv::String::trim

#include "player.h"

std::string
Player::query_manager (const std::string& s)
{
  manager_.request(std::make_pair(this, s), &mbox_);
  std::string reply = mbox_.get(delay_);
  log() << name() << "> " << reply << std::endl;
  return reply;
}

Player*
Player::make (Manager& mgr, Dv::shared_ptr<Dv::Net::Socket> so, size_t delay,
              size_t debug_level, Dv::Debugable* debug)
{
  Player* player = new Player(mgr, so, delay, debug_level, debug);
  mgr.request(std::make_pair(player, "addplayer"));
  return player;
}

Player::Player (Manager& mgr, Dv::shared_ptr<Dv::Net::Socket> so, size_t delay,
                size_t debug_level, Dv::Debugable* debug) :
Dv::Thread::Thread (true, debug_level, debug), manager_ (mgr), so_ (so),
mbox_ ("player"), incoming_ ("incoming"), name_ (""), delay_ (delay) { }

void
Player::quit ()
{
  log() << name() << " quitting.. " << std::endl;
  so_->close();
  // if killed() then the manager already knows we're quitting
  if ( !killed() )
    {
      // let the manager know that we quit
      manager_.request(std::make_pair(this, "quit"));
      while ( !killed() )
        sleep(1);
    }
}

/**
 * @return 0 iff a line was read
 * @return 1 I/O error while reading
 * @return 2 we were killed
 */
int
Player::get_line (Dv::Net::Socket& so, std::string& line)
{
  (so << "> ").flush();
  while ( true )
    {
      // check if out-of-band data came in and, if so, show them
      try
        {
          if ( incoming_.size() )
            {
              while ( incoming_.size() )
                so << incoming_.get(100) << std::endl;
              (so << "> ").flush(); // show new prompt
            }
        }
      catch (std::runtime_error& e)
        {
          log() << __PRETTY_FUNCTION__ << ": exception: " << e.what()
                  << ": ignored" << std::endl;
        }
      if ( std::getline(so, line) )
        {
          Dv::String::trim(line);
          return 0;
        }
      if ( killed() )
        return 2;
      if ( !so.timedout() )
        return 1;
      so.timedout(false);
    }
}

int
Player::main ()
{
  try
    {
      std::string line;
      while ( !killed() )
        {
          log(1) << __PRETTY_FUNCTION__ << " " << __FILE__ << "." << __LINE__ << std::endl;
          switch (get_line(*so_, line))
            {
              case 0:
                if ( !killed() )
                  {
                    // send message to manager for processing and show her reply
                    try
                      {
                        std::string reply = query_manager(line);
                        *so_ << reply << std::endl;
                      }
                    catch (std::runtime_error& e)
                      {
                        log() << "query_manager exception: " << e.what() << std::endl;
                        throw;
                      }
                  }
                else
                  *so_ << "bye" << std::endl;
                break;
              case 1: // I/O error
              case 2: // killed()
                quit();
                return 0;
            }
        }
      // killed by manager
      quit();
      return 0;
    }
  catch (std::exception& e)
    {
      quit();
      return 1;
    }
}
