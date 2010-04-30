#ifndef __NEXUSTOKEN_H
#define __NEXUSTOKEN_H

//
// NexusToken class
//
class NexusToken
{
	istream&   in;
	int        newlineType;

#if defined( __MWERKS__ )
	long       filepos;
#else
	streampos  filepos;
#endif
	
	long       fileline;
	long       filecol;

	nxsstring     token;
	nxsstring     comment;

	char       saved;
	bool       atEOF;
	bool       atEOL;

   char       special;

	int        labileFlags;

protected:

	void AppendToComment( char ch );
	void AppendToToken( char ch );
	char GetNextChar();
	void GetComment();
	void GetCurlyBracketedToken();
	void GetDoubleQuotedToken();
	void GetQuoted();
	void GetParentheticalToken();
	bool IsPunctuation( char ch );
	bool IsWhitespace( char ch );

public:
	enum {
		saveCommandComments   = 0x0001,
		parentheticalToken    = 0x0002,
		curlyBracketedToken   = 0x0004,
		doubleQuotedToken     = 0x0008,
		singleCharacterToken  = 0x0010,
		newlineIsToken        = 0x0020,
		tildeIsPunctuation    = 0x0040,
      useSpecialPunctuation = 0x0080,
      hyphenNotPunctuation  = 0x0100
	};

   nxsstring errormsg;

	NexusToken( istream& i );
	virtual ~NexusToken();

	bool       AtEOF();
	bool       AtEOL();
	bool       Abbreviation( nxsstring s );
	bool       Begins( nxsstring s, bool respect_case = false );
	void       BlanksToUnderscores();
	bool       Equals( nxsstring s, bool respect_case = false );
	long       GetFileColumn();
	streampos  GetFilePosition();
	long       GetFileLine();
	void       GetNextToken();
	nxsstring  GetToken( bool respect_case = true );
	int        GetTokenLength();
	bool       IsPlusMinusToken();
	bool       IsPunctuationToken();
	bool       IsWhitespaceToken();
	void       ReplaceToken( const nxsstring s );
	void       ResetToken();
   void       SetSpecialPunctuationCharacter( char c );
	void       SetLabileFlagBit( int bit );
   bool       StoppedOn( char ch );
	void       StripWhitespace();
	void       ToUpper();
	void       Write( ostream& out );
	void       Writeln( ostream& out );

	// virtual function that should be overridden in derived classes
	virtual void OutputComment( nxsstring& msg );
};

#endif
