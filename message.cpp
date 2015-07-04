#include "message.h"

Message::Message (unsigned int number, const std::string& filepath) :
_number (number), _deleted (false), _file_path (filepath) { }

Message::~Message ()
{
  if ( deleted() )
    {
      if ( remove(_file_path.c_str()) != 0 )
        throw std::runtime_error("unable to delete message: " + _file_path);
    }
}

unsigned long Message::size () const
{
  unsigned long begin, end;
  std::ifstream in_file(_file_path.c_str());

  if ( in_file.is_open() )
    {
      // Get position of the get pointer
      begin = in_file.tellg();
      // Set the postion of the  get pointer to the end of the file
      in_file.seekg(0, std::ios::end);
      // Get the position of the get pointer
      end = in_file.tellg();
      in_file.close();

      unsigned long bits(end - begin);

      if ( bits == 0 )
        return bits;
      else
        // We get the size size of the file by substracting end from begin
        return ceil(bits / 8);
    }
  else
    throw std::runtime_error("unable to open message: " + _file_path);
}

std::string Message::message_string () const
{
  // Output string stream where we will put the text of the message
  std::ostringstream oss;
  std::ifstream in_file(_file_path.c_str());
  std::string line;

  if ( in_file.is_open() )
    {
      // Put each line of the file in the output stream
      while ( !in_file.eof() )
        {
          getline(in_file, line);
          oss << line << std::endl;
        }
      in_file.close();
      // Return the actual string of the oss
      return oss.str(); 
    }
  else
    throw std::runtime_error("unable to open message: " + _file_path);
}

std::string Message::message_string (int n) const
{
  // Output string stream where we will put the text of the message
  std::ostringstream oss;
  std::ifstream in_file(_file_path.c_str());
  std::string line;

  if ( in_file.is_open() )
    {
      // Counter to keep track of how many lines we already got
      int counter(0);
      /* Put each line of the file in the output stream
       * until counter = n or we are at the end of the file */
      while ( (!(counter == n)) && (!in_file.eof()) )
        {
          getline(in_file, line);
          oss << line << std::endl;
          counter++;
        }
      in_file.close();
      // Return the actual string of the oss
      return oss.str(); // Return the actual string of the oss
    }
  else
    throw std::runtime_error("unable to open message: " + _file_path);
}

std::string Message::uidl () const
{
  std::ostringstream oss;
  std::size_t size(_file_path.size());
  // We start at position size-1
  int i(--size);

  /* Seek the file path backwards and put every char in oss, until we find '/'
   *  eg /home/wouter/123 will return "123" */
  while ( _file_path.at(i) != '/' )
    {
      oss << _file_path.at(i);
      i--;
    }
  std::string uidl_string(oss.str());
  /* Since we put the characters on the oss backwards
   * we have to reverse the string now */
  std::reverse(uidl_string.begin(), uidl_string.end());
  return uidl_string;
}

std::ostream & operator<< (std::ostream& os, const Message& msg)
{
  return os << msg.number() << " " << msg.size();
}
