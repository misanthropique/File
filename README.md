# File

A class with an interface mirroring the Unix file interface for the purposes of allowing the user
to read/write/seek on a file regardless of location or protocol, and without needing to use different
APIs for each and every protocol. There is currently a TODO item to allow the use of user credentials
for access to secured protocols, but that will be included in a later release.

A few example uses:
```
File localFile( "/home/user1/stock_prices.csv", File::IOFlag::READ );
File httpFile( "http://example.com/index.html", File::IOFlag::READ );
File ftpReadFile( "ftp://bigcompany.com/media/opus_17.mp3", File::IOFlag::READ );
File ftpWriteFile( "ftp://bigcompany.com/visitors.log", File::IOFlag::WRITE );

// Let them know we gave them a visit.
char visitor[] = "Johnny O.T. Spot\n";
ftpWriteFile.append( visitor, strlen( visitor ) );
ftpWriteFile.close();

int64_t bytesRead;
char readBuffer[ 2048 ];
while ( 0 < ( bytesRead = httpFile.read( readBuffer, sizeof( readBuffer ) ) ) ) {
    write( STDOUT_FILENO, readBuffer, bytesRead );
}
httpFile.close();

... etc ...

```
