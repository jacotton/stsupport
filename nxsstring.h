#ifndef __NXSSTRING_H
#define __NXSSTRING_H

class nxsstring : public std::string
{
   public:
      nxsstring() {}
      nxsstring( const char* s ) {
         assign(s);
      }
      nxsstring( const nxsstring& s ) {
         assign(s);
      }
      nxsstring& operator=( const char* s ) {
         assign(s);
         return *this;
      }
      nxsstring& operator=( const nxsstring& s ) {
         assign(s);
         return *this;
      }
      nxsstring& operator+=( const nxsstring& s ) {
         append(s);
         return *this;
      }
      nxsstring& operator+=( const char c ) {
      	 char s[2];
      	 s[0] = c;
      	 s[1] = '\0';
         append(s);
         return *this;
      }
      nxsstring& operator+=( const int i ) {
         char tmp[81];
         sprintf( tmp, "%d", i );
         append(tmp);
         return *this;
      }
      nxsstring& operator+=( const long l ) {
         char tmp[81];
         sprintf( tmp, "%ld", l );
         append(tmp);
         return *this;
      }
      nxsstring& operator+=( const double d );
      
		void RightJustifyLong( long x, int w, bool clear_first = false );
		void RightJustifyDbl( double x, int w, int p, bool clear_first = false );
      void ShortenTo( int n );
};

#endif



