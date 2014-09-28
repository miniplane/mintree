 #ifndef CONTAINER_H
 #define CONTAINER_H
 
 // Container.h
 // 
 // UE Algorithmen und Datenstrukturen - SS 2014 Universitaet Wien
 // Container - Projekt
 // https://cewebs.cs.univie.ac.at/ADS/ss14/
 
 #include <iostream>
 
 enum Order { dontcare, ascending, descending };
 template <typename E> class Functor;
 
 class ContainerException : public std::exception {
 public:
   virtual const char* what() const throw() = 0;
 };
 
 template <typename E>
 class Container {
   Container& operator=( const Container<E>& );
   Container( const Container<E>& );
 public:
   Container( ) { }
   virtual ~Container( ) { }
 
   virtual void add( const E& e ) { add( &e, 1 ); }
   virtual void add( const E e[], size_t s ) = 0;
 
   virtual void remove( const E& e ) { remove( &e, 1 ); }
   virtual void remove( const E e[], size_t s ) = 0;
 
   virtual bool member( const E& e ) const = 0;
   virtual size_t size( ) const = 0;
   virtual bool empty( ) const { return size( ) == 0; }
 
   virtual size_t apply( const Functor<E>& f, Order order = dontcare ) const = 0;
 
   virtual E min( ) const = 0;
   virtual E max( ) const = 0;
 
   virtual std::ostream& print( std::ostream& o ) const = 0;
 };
 
 template <typename E>
 inline std::ostream& operator<<( std::ostream& o, const Container<E>& c ) { return c.print( o ); }
  
 template <typename E>
 class Functor {
 public:
   virtual bool operator( )( const E& e ) const = 0;
   virtual ~Functor( ) {}
 };
 
 template <typename E> inline unsigned long hashValue( const E& e ) { return (unsigned long) e * 47114711UL; }
 template <typename E> inline double doubleValue( const E& e ) { return double( e ); }
 template <typename E> inline unsigned long ordinalValue( const E& e ) { return (unsigned long) e; }
 
 #endif //CONTAINER_H
