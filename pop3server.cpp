/* 
 * File:   pop3server.cpp
 * Author: Dirk Vermeir
 * Edited by: Wouter Van Rossem
 *
 * Created on 23 juli 2009, 14:43
 */

#include <fstream>

#include <iostream>
#include <stdexcept>
#include <dvutil/debug.h>
#include <dvutil/props.h>
#include <dvthread/logstream.h>
#include <dvnet/serversocket.h>

#include "player.h"
#include "manager.h"

// In a production system, server_log would be linked
// to a file stream. Alternatively, it can be launched
// like so:
//
// nohup ./pop3 config-file >& /var/log/pop3server_log.txt &

std::ofstream logfile ("pop3.log", std::ios_base::app);
Dv::Thread::logstream pop3server_log (logfile, "pop3server");
Dv::Debug debug (&pop3server_log, 0);

int
main (int argc, char* argv[])
{
  static const char* usage = "pop3 config-file"; // Manual.

  // Put everything in a try block, so we catch all I/O and
  // network errors, among others.
  try
    {
      // First check whether the required config file argument is
      // present (there is no default).
      if ( argc != 2 )
        throw std::runtime_error(usage);

      Dv::Props config;

      { // read configuration info
        std::ifstream ifconfig(argv[1]);
        if ( !ifconfig )
          throw std::runtime_error(std::string(argv[1]) + ": cannot open");
        ifconfig >> config;
      }

      // Set up the manager object. Note that this will also start
      // a thread that will actually handle player requests.
      Manager manager("pop3manager", config, &debug);

      // The delay to use througout for I/O operations, mailbox waiting etc.
      size_t delay = config("timeout");

      // Set up a server socket listening on the port
      Dv::Net::ServerSocket server(config("port").get<int>());

      // Each time around the main loop, check whether the manager wants to stop.
      while ( !manager.done() )
        {
          // The following times out after delay millisec, thus we will check
          // manager::done regularly.
          if ( server.connection(delay) )
            {
              // The delay argument to ServerSocket::accept() makes e.g.
              // getline(socket) time out after delay millisecs, ensuring that
              // we can often check conditions in a player's main loop.
              Dv::shared_ptr<Dv::Net::Socket> socket(server.accept(delay));
              // The timeout argument to Player::make ensures that the player
              // will e.g. timeout after not receiving a reply from the
              // manager.
              Player::make(manager, socket, delay, config("debuglevel"), &debug)->start();
            }
        }

      // The manager wants to stop: kill it. This will kill all
      // remaining players and wait for them to finish, then it
      // will kill the manager thread and wait for it to finish before
      // returning.
      manager.kill();
    }
  catch (std::exception& e)
    {
      // You get here if anything goes wrong in the try block.
      pop3server_log << "pop3server error: " << e.what() << std::endl;
      return 2;
    }

  pop3server_log << "bye" << std::endl;
  logfile.close();
  return 0;
}


