#include "address.hh"
#include "socket.hh"

#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <span>
#include <string>
#include <strings.h>
#include <sys/socket.h>

using namespace std;

void get_URL( const string& host, const string& path )
{
  TCPSocket client {};

  const Address addr( host, "http" );

  client.connect( addr );

  // send
  string request { "GET " + path + " HTTP/1.1\r\n" };
  cout << request;
  string header0 { "Host: " + host + "\r\n" };
  cout << header0;
  string header1 { "Connection: close\r\n" };
  cout << header1;
  string header2 { "\r\n" };
  cout << header2;
  client.write( request );
  client.write( header0 );
  client.write( header1 );
  client.write( header2 );

  string buf;
  while ( !client.eof() ) {
    buf.clear();
    client.read( buf );
    cout << buf;
  }
  client.close();
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
