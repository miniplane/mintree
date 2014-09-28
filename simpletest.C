 // simpletest.C
 // 
 // UE Algorithmen und Datenstrukturen - SS 2014 Universitaet Wien
 // Container - Projekt
 // https://cewebs.cs.univie.ac.at/ADS/ss14/
 //
 // Simples Testprogramm zur Ueberpruefung der Container-Funktionalitaet
 // Die Zeichenfolge CONTAINERTYPE ist in der ganzen Datei durch den Klassennamen 
 // der Datenstruktur zu ersetzen.
 // 
 // Der Elementdatentyp muss mit Compileroption -DETYPE=<typ> festgelegt werden,
 // also zb -DETYPE=std::string
 
 #include <iostream>
 #include <sstream>
 #include <fstream>
 #include <string>
 #include <cstring>
 #include <cstdlib>
 #include <cctype>
 #include "BpTree.h"
 
 #ifndef ETYPE
 #error ETYPE not defined - compile with option -DETYPE=type
 #endif
 #define str(x) #x
 
 const char helpstr[] = 
   "new ............................... create new Container\n"
   "delete ............................ delete Container\n"
   "add <key> [...] ................... add <key>(s) with Container::add( int )\n"
   "remove <key> [...] ................ remove <key>(s) with Container::remove( int )\n"
   "member <key> ...................... call Container::member( <key> )\n"
   "size .............................. call Container::size()\n"
   "empty ............................. call Container::empty()\n"
   "min ............................... call Container::min()\n"
   "max ............................... call Container::max()\n"
   "print ............................. print container with operator<<()\n"
   "apply [asc|desc|dontcare [<n>]] ... traverse container with PrintN functor\n"
   "trace ............................. toggle tracing on/off\n"
   "fadd <filename> ................... add values read from file <filename>\n"
   "fremove <filename> ................ remove values read from file <filename>\n"
   "radd [<n> [<seed>]] ............... add <n> random values, optionally reset generator to <seed>\n"
   "rremove [<n> [<seed>]] ............ remove <n> random values, optionally reset generator to <seed>\n"
   "quit .............................. quit program\n\n"
   "arguments surrounded by [] are optional\n";
 
 template <typename E>
 class PrintN : public Functor<E> {
   std::ostream& o;
   mutable int n;
 public:
   explicit PrintN( int n = 0, std::ostream& o = std::cout ) : o( o ), n( n ) { }
   explicit PrintN( std::ostream& o ) : o( o ), n( 0 ) { }
   bool operator()( const E& e ) const {
     o << e << ' ';
     return n <= 0 || --n;
   }
 };

 void setrandom( int seed ) { srand( seed ); }
 template <typename E> E nextrandom( ) { return E( rand( ) ); }
 
 // Template-Spezialisierungen fuer Klasse std::string
 
 template <> inline double doubleValue( const std::string& e ) { double rc = 0.; for (size_t i = e.length(); i--; ) rc /= 256., rc += e[i]; return rc; }
 template <> inline unsigned long hashValue( const std::string& e ) { unsigned long rc = 0; for (size_t i = 0; i < e.length(); ++i) rc = rc * 13 + e[i]; return rc; }
 template <> std::string nextrandom( ) {
   const char* start = helpstr + rand() % sizeof helpstr;
   while (!isalpha( *start )) if (*start) ++start; else start = helpstr;
   const char* end = start + 1;
   while (isalpha( *end )) ++end;
   char buf[10];
   sprintf( buf, "%d", rand() % 1000 );
   return std::string( start, end - start ) += buf;
 }

 // Klasse Person mit allen für die Verwendung als Container-Elementdatentyp noetigen Methoden und Funktionen
 
 class Person {
   std::string vorname;
   std::string nachname;
 public:
   Person() { }
   Person( std::string vorname, std::string nachname ) : vorname( vorname ), nachname( nachname ) { }
   bool operator==( const Person& p ) const { return vorname == p.vorname && nachname == p.nachname; }
   bool operator>( const Person& p ) const { return nachname > p.nachname || (nachname == p.nachname && vorname > p.vorname); }
 
   std::ostream& print( std::ostream& o ) const { return o << '[' << nachname << ", " << vorname << ']'; }
   std::istream& read( std::istream& i ) { return i >> vorname >> nachname; }
   friend double doubleValue<Person>( const Person& e );
   friend unsigned long hashValue<Person>( const Person& e );
   friend unsigned long ordinalValue<Person>( const Person& e );
 };
 
 inline std::ostream& operator<<( std::ostream& o, const Person& p ) { return p.print( o ); }
 inline std::istream& operator>>( std::istream& i, Person& p ) { return p.read( i ); }
 
 // Template-Spezialisierungen fuer Klasse Person
 
 template <> inline double doubleValue( const Person& e ) { return doubleValue( e.nachname ); }
 template <> inline unsigned long hashValue( const Person& e ) { return hashValue( e.nachname ); }
 template <> Person nextrandom( ) { 
   return Person( nextrandom<std::string>(), nextrandom<std::string>() );
 }
 
 bool match( const std::string& s, const char * c ) {
   return c && s.length() <= std::strlen( c ) && s.compare( 0, s.length(), c, s.length() ) == 0;
 }
 
 int main() {
 
   Container<ETYPE>* c = 0;
   bool traceIt = false;
   std::cout.setf( std::ios_base::boolalpha );
 
   while (true) {
     if (traceIt) {
       if (c) {
         std::cout << std::endl << "container: " << *c;
       } else {
         std::cout << std::endl << "no container";
       }
     }
     std::cout << std::endl << "> ";
 
     std::string cmdline;
     if (!std::getline( std::cin, cmdline )) break;
 
     std::istringstream cmdstream( cmdline );
     std::string cmd;
 
     cmdstream >> cmd;
 
     try {
       if (cmd.length() == 0) {
       } else if (match( cmd, "quit" )) {
         break;
       } else if (match( cmd, "new" )) {
         if (c) {
           std::cerr << "container exists, 'delete' it first";
         } else {
           //c = new CONTAINERTYPE<ETYPE,13>;
           c = new CONTAINERTYPE<ETYPE>;
         }
       } else if (match( cmd, "help" ) || cmd == "?") {
         std::cout << helpstr;
       } else if (match( cmd, "trace" )) {
         std::cout << "trace " << ((traceIt = !traceIt) ? "on" : "off");
       } else if (!c) {
         std::cout << "no container (use 'new')";
       } else {
         ETYPE key;
         if (match( cmd, "delete" )) {
           delete c;
           c = 0;
         } else if (match( cmd, "add" )) {
           while (cmdstream >> key) { c->add( key ); }
         } else if (match( cmd, "remove" )) {
           while (cmdstream >> key) { c->remove( key ); }
         } else if (match( cmd, "member" )) {
           cmdstream >> key;
           std::cout << "returns " << c->member( key );
         } else if (match( cmd, "size" )) {
           std::cout << "returns " << c->size( );
         } else if (match( cmd, "empty" )) {
           std::cout << "returns " << c->empty( );
         } else if (match( cmd, "min" )) {
           std::cout << "returns " << c->min( );
         } else if (match( cmd, "max" )) {
           std::cout << "returns " << c->max( );
         } else if (match( cmd, "print" )) {
           std::cout << *c;
         } else if (match( cmd, "apply" )) {
           int n = 0;
           std::string order = "dontcare";
           cmdstream >> order >> n;
           size_t rc = c->apply( PrintN<ETYPE>( n ), match( order, "ascending" ) ? ascending : match( order, "descending" ) ? descending : dontcare );
           std::cout << "\nreturns " << rc;
         } else if (match( cmd, "fadd" )) {
           std::string filename;
           cmdstream >> filename;
           std::ifstream keystream( filename.c_str() );
           while (keystream >> key) { c->add( key ); }
         } else if (match( cmd, "fremove" )) {
           std::string filename;
           cmdstream >> filename;
           std::ifstream keystream( filename.c_str() );
           while (keystream >> key) { c->remove( key ); }
         } else if (match( cmd, "radd" )) {
           int seed = -1, count = 1;
           cmdstream >> count >> seed;
           if (seed != -1) setrandom( seed );
           while (count-- > 0) c->add( nextrandom<ETYPE>() );
         } else if (match( cmd, "rremove" )) {
           int seed = -1, count = 1;
           cmdstream >> count >> seed;
           if (seed != -1) setrandom( seed );
           while (count-- > 0) c->remove( nextrandom<ETYPE>() );
         } else {
           std::cout << cmd << "? try 'help'";
         }
       }
     } catch (ContainerException& e) {
       std::cout << "ContainerException " << e.what();
     } catch (std::exception& e) {
       std::cout << "Exception " << e.what();
     } catch (...) {
       std::cout << "OOPS!";
     }
   }
   return 0;
 }

