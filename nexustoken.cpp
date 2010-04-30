#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"

/**
 * @class      NexusToken
 * @file       nexustoken.h
 * @file       nexustoken.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   atEOF [bool:private] true if last character read resulted in eof() returning true for input stream
 * @variable   atEOL [bool:private] true if newline encountered while newlineIsToken labile flag set
 * @variable   comment [nxsstring:private] temporary buffer used to store output comments while they are being built
 * @variable   filecol [long:private] current column in current line (refers to column immediately following token just read)
 * @variable   fileline [long:private] current file line
 * @variable   filepos [long:private] current file position (for Metrowerks compiler, type is streampos rather than long)
 * @variable   in [istream&:private] reference to input stream from which tokens will be read
 * @variable   labileFlags [int:private] storage for labile flags (see labile enum)
 * @variable   saved [char:private] either '\0' or is last character read from input stream
 * @variable   special [char:private] ad hoc punctuation character; default value is '\0'
 * @variable   token [nxsstring:private] the character buffer used to store the current token
 * @see        NexusReader
 * @see        XNexus
 *
 * This class is used to read tokens from a Nexus data file.  If the token
 * object is not attached to an input stream, calls to GetNextToken will have no
 * effect. If the token object is not attached to an output stream, output
 * comments will be discarded (i.e., not output anywhere) and calls to Write
 * or Writeln will be ineffective.  If input and output streams have been
 * attached to the token object, however, tokens are read one at a time from
 * the input stream, and comments are correctly read and either written to
 * the output stream (if an output comment) or ignored (if not an output
 * comment). Sequences of characters surrounded by single quotes are read in
 * as single tokens.  A pair of adjacent single quotes are stored as a single
 * quote, and underscore characters are stored as blanks.
 */

/**
 * @enumeration
 * @enumitem  saveCommandComment [0x0001] if set, command comments of the form [&X] are not ignored but are instead saved as regular tokens (without the square brackets, however)
 * @enumitem  parentheticalToken [0x0002] if set, and if next character encountered is a left parenthesis, token will include everything up to the matching right parenthesis
 * @enumitem  curlyBracketedToken [0x0004] if set, and if next character encountered is a left curly bracket, token will include everything up to the matching right curly bracket
 * @enumitem  doubleQuotedToken [0x0008] if set, grabs entire phrase surrounded by double quotes
 * @enumitem  singleCharacterToken [0x0010] if set, next non-whitespace character returned as token
 * @enumitem  newlineIsToken [0x0020] if set, newline character treated as a token and atEOL set if newline encountered
 * @enumitem  tildeIsPunctuation [0x0040] if set, tilde character treated as punctuation and returned as a separate token
 * @enumitem  useSpecialPunctuation [0x0080] if set, "special" character treated as punctuation and returned as a separate token
 *
 * For use with the variable labileFlags.
 */

/**
 * @constructor
 * @param i [istream&] the istream object to which the token is to be associated
 *
 * Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> atEOF         <td>= <td> false
 * <tr><td> atEOL         <td>= <td> false
 * <tr><td> comment       <td>= <td> ""
 * <tr><td> filecol       <td>= <td> 1L
 * <tr><td> fileline      <td>= <td> 1L
 * <tr><td> filepos       <td>= <td> 0L
 * <tr><td> in            <td>= <td> i
 * <tr><td> labileFlags   <td>= <td> 0
 * <tr><td> saved         <td>= <td> '\0'
 * <tr><td> special       <td>= <td> '\0'
 * <tr><td> token         <td>= <td> ""
 * </table>
 */
NexusToken::NexusToken( istream& i ) : in(i)
{
	atEOF       = false;
	atEOL       = false;
	comment     = "";
	filecol     = 1L;
	fileline    = 1L;
	filepos     = 0L;
	labileFlags = 0;
	saved       = '\0';
	special     = '\0';
	token       = "";
}

/**
 * @destructor
 *
 * Nothing needs to be done, since all objects take care of that delete themselves.
 */
NexusToken::~NexusToken()
{
}

/**
 * @method AppendToComment [void:protected]
 * @param ch [char] character to be appended to comment
 *
 * Adds ch to end of comment nxsstring.  
 */
void NexusToken::AppendToComment( char ch )
{
   comment += ch;
}

/**
 * @method AppendToToken [void:protected]
 * @param ch [char] character to be appended to token
 *
 * Adds ch to end of current token.
 */
void NexusToken::AppendToToken( char ch )
{
	// first three lines proved necessary to keep
   // Borland's implementation of STL from crashing
   // under some circumstances
	char s[2];
   s[0] = ch;
   s[1] = '\0';

   token += s;
}

/**
 * @method GetNextChar [char:protected]
 * @throws XNexus
 *
 * Reads next character from in and does all of the following before
 * returning it to the calling function:
 * <ul>
 * <li> if character read is either a carriage return or line feed,
 *   the variable line is incremented by one and the variable col is
 *   reset to zero
 * <li> if character read is a carriage return, and a peek at the next
 *   character to be read reveals that it is a line feed, then the
 *   next (line feed) character is read
 * <li> if either a carriage return or line feed is read, the character
 *   returned to the calling function is '\n'
 * <li> if character read is neither a carriage return nor a line feed,
 *   col is incremented by one and the character is returned as is to the
 *   calling function
 * <li> in all cases, the variable filepos is updated using a call to
 *   the tellg function of istream.
 * </ul>
 */
char NexusToken::GetNextChar()
{
	int ch;
	ch = in.get();
	int failed = in.bad();
	if( failed ) {
		errormsg = "Unknown error reading data file (check to make sure file exists)";
		throw XNexus( errormsg );
	}

	if( ch == 13 || ch == 10 )
	{
		fileline++;
		filecol = 1L;

		if( ch == 13 && (int)in.peek() == 10 )
			ch = in.get();

		atEOL = 1;
	}
	else if( ch == EOF )
		atEOF = 1;
	else {
		filecol++;
		atEOL = 0;
	}

	filepos = in.tellg();

   if( atEOF )
      return '\0';
   else if( atEOL )
      return '\n';
   else
   	return (char)ch;
}

/**
 * @method GetComment [void:protected]
 *
 * Reads rest of comment (starting '[' already input) and acts accordingly.
 * If comment is an output comment, and if an output stream has been attached,
 * writes the output comment to the output stream.  Otherwise, output comments
 * are simply ignored like regular comments. If the labileFlag bit saveCommandComments
 * is in effect, the comment (without the square brackets) will be stored in token.
 */
void NexusToken::GetComment()
{
	// Set comment level to 1 initially.  Every ']' encountered reduces
	// level by one, so that we know we can stop when level becomes 0.
	int level = 1;

	// get first character
	char ch = GetNextChar();
	if( atEOF ) {
		errormsg = "Unexpected end of file inside comment";
		throw XNexus( errormsg, GetFilePosition(), GetFileLine(), GetFileColumn() );
	}

	// see if first character is the output comment symbol ('!')
	// or command comment symbol (&)
	int printing = 0;
	int command = 0;
	if( ch == '!' )
		printing = 1;
	else if( ch == '&' && labileFlags & saveCommandComments ) {
		command = 1;
		AppendToToken(ch);
	}

	// now read the rest of the comment
	for(;;)
	{
		ch = GetNextChar();
		if( atEOF ) break;

		if( ch == ']' )
			level--;
		else if( ch == '[' )
			level++;

		if( level == 0 )
			break;

		if( printing )
			AppendToComment(ch);
		else if( command )
			AppendToToken(ch);
	}

	if( printing ) {
		// allow output comment to be printed or displayed in most appropriate
		// manner for target operating system
		OutputComment( comment );

		// now that we are done with it, free the memory used to store the comment
		comment = "";
	}
}

/**
 * @method GetCurlyBracketedToken [void:protected]
 *
 * Reads rest of a token surrounded with curly brackets (the starting '{'
 * has already been input) up to and including the matching '}' character.
 * All nested curly-bracketed phrases will be included.
 */
void NexusToken::GetCurlyBracketedToken()
{
	// Set level to 1 initially.  Every '}' encountered reduces
	// level by one, so that we know we can stop when level becomes 0.
	int level = 1;

	char ch;
	for(;;)
	{
		ch = GetNextChar();
		if( atEOF )	break;

		if( ch == '}' )
			level--;
		else if( ch == '{' )
			level++;

		AppendToToken(ch);

		if( level == 0 )
			break;
	}
}

/**
 * @method GetDoubleQuotedToken [void:protected]
 *
 * Gets remainder of a double-quoted Nexus word (the first double quote character
 * was read in already by GetNextToken). This function reads characters until
 * the next double quote is encountered.  Tandem double quotes within
 * a double-quoted Nexus word are not allowed and will be treated as the end
 * of the first word and the beginning of the next double-quoted Nexus word.
 * Tandem single quotes inside a double-quoted Nexus word are saved as two
 * separate single quote characters; to embed a single quote inside a
 * double-quoted Nexus word, simply use the single quote by itself (not
 * paired with another tandem single quote).
 */
void NexusToken::GetDoubleQuotedToken()
{
	char ch;

	for(;;)
	{
		ch = GetNextChar();
		if( atEOF )	break;

		if( ch == '\"' ) {
			break;
		}
		else if( ch == '_' )	{
			ch = ' ';
			AppendToToken(ch);
		}
		else
			AppendToToken(ch);

	}
}

/**
 * @method GetQuoted [void:protected]
 *
 * Gets remainder of a quoted Nexus word (the first single quote character
 * was read in already by GetNextToken). This function reads characters until
 * the next single quote is encountered.  An exception occurs if two single
 * quotes occur one after the other, in which case the function continues
 * to gather characters until an isolated single quote is found.  The tandem
 * quotes are stored as a single quote character in the token nxsstring.
 */
void NexusToken::GetQuoted()
{
	char ch;

	for(;;)
	{
		ch = GetNextChar();
		if( atEOF )	break;

		if( ch == '\'' && saved == '\'' ) {
			// paired single quotes, save as one single quote
			AppendToToken(ch);
			saved = '\0';
		}
		else if( ch == '\'' && saved == '\0' )
		{
			// save the single quote to see if it is followed by another
			saved = '\'';
		}
		else if( saved == '\'' )
		{
			// previously read character was single quote but this is
			// something else, save current character so that it will
			// be the first character in the next token read
			saved = ch;
			break;
		}
		else if( ch == '_' )
		{
			ch = ' ';
			AppendToToken(ch);
		}
		else
			AppendToToken(ch);

	}
}

/**
 * @method GetParentheticalToken [void:protected]
 *
 * Reads rest of parenthetical token (starting '(' already input) up to and
 * including the matching ')' character.  All nested parenthetical phrases
 * will be included.
 */
void NexusToken::GetParentheticalToken()
{
	// Set level to 1 initially.  Every ')' encountered reduces
	// level by one, so that we know we can stop when level becomes 0.
	int level = 1;

   char ch;
	for(;;)
	{
		ch = GetNextChar();
		if( atEOF )	break;

		if( ch == ')' )
			level--;
		else if( ch == '(' )
			level++;

      AppendToToken(ch);

		if( level == 0 )
			break;
	}
	
	// rdmp
	// This method is used to read Newick tree decsriptions, but these may exten beyon the
	// last ')' (e.g., information on branch length for the root node). This next hack keeps
	// reading until we hit the semicolon
	while (in.peek() != ';')
	{
		ch = in.get();
		AppendToToken (ch);
	}
	
}

/**
 * @method IsPunctuation [bool:protected]
 * @param ch [char] the character in question
 *
 * Returns true if character supplied is considered a punctuation character.
 * The following twenty characters are considered punctuation characters unless
 * otherwise indicated below: '(', ')', '[', ']', '{', '}', '/', '\\', ',', ';',
 * ':', '=', '*', '\'', '"', '`', '+', '-', '<' and '>'.  
 * <p>Exceptions:
 * <ul>
 * <li> The tilde character ('~') is also considered punctuation if the 
 *      tildeIsPunctuation labile flag is set
 * <li> The special punctuation character (specified using the 
 *      SetSpecialPunctuationCharacter) is also considered punctuation if the 
 *      useSpecialPunctuation labile flag is set
 * <li> The hyphen (i.e., minus sign) character ('-') is <i>not</i> considered 
 *      punctuation if the hyphenNotPunctuation labile flag is set
 * </ul>
 * Use the SetLabileFlagBit method to set one or more labile flags.
 */
bool NexusToken::IsPunctuation( char ch )
{
	char punctuation[21];
	punctuation[0]  = '(';
	punctuation[1]  = ')';
	punctuation[2]  = '[';
	punctuation[3]  = ']';
	punctuation[4]  = '{';
	punctuation[5]  = '}';
	punctuation[6]  = '/';
	punctuation[7]  = '\\';
	punctuation[8]  = ',';
	punctuation[9]  = ';';
	punctuation[10] = ':';
	punctuation[11] = '=';
	punctuation[12] = '*';
	punctuation[13] = '\'';
	punctuation[14] = '"';
	punctuation[15] = '`';
	punctuation[16] = '+';
	punctuation[17] = '-';
	punctuation[18] = '<';
	punctuation[19] = '>';
	punctuation[20] = '\0';

   bool is_punctuation = false;
   if( strchr( punctuation, ch ) != NULL )
      is_punctuation = true;
   if( labileFlags & tildeIsPunctuation  && ch == '~' )
      is_punctuation = true;
   if( labileFlags & useSpecialPunctuation  && ch == special )
      is_punctuation = true;
   if( labileFlags & hyphenNotPunctuation  && ch == '-' )
      is_punctuation = false;
   return is_punctuation;
}

/**
 * @method IsWhitespace [bool:protected]
 * @param ch [char] the character in question
 *
 * Returns true if character supplied is considered a whitespace character.
 * Note: treats '\n' as darkspace if labile flag newlineIsToken is in effect.
 */
bool NexusToken::IsWhitespace( char ch )
{
	char whitespace[4];
	whitespace[0]  = ' ';
	whitespace[1]  = '\t';
	whitespace[2]  = '\n';
	whitespace[3]  = '\0';

	bool ws = false;

	// if ch is found in the whitespace array, it's whitespace
	//
	if( strchr( whitespace, ch ) != NULL )
		ws = true;

	// unless of course ch is the newline character and we're currently
	// treating newlines as darkspace!
	//
	if( labileFlags & newlineIsToken && ch == '\n' )
		ws = false;

	return ws;
}

/**
 * @method Abbreviation [bool:public]
 * @param s [nxsstring] the comparison string
 *
 * Returns true if token begins with the capitalized portion of s
 * and, if token is longer than s, the remaining characters match
 * those in the lower-case portion of s.  The comparison is case
 * insensitive.  This function should be used instead of the 
 * Begins function if you wish to allow for abbreviations of commands
 * and also want to ensure that user does not type in a word that
 * does not correspond to any command.
 */
bool NexusToken::Abbreviation( nxsstring s )
{
	int k;
	int slen = s.size();
	int tlen = token.size();
	char tokenChar, otherChar;

	// The variable mlen refers to the "mandatory" portion
	// that is the upper-case portion of s
	//
	int mlen;
	for( mlen = 0; mlen < slen; mlen++ ) {
		if( !isupper(s[mlen]) )	break;
	}
	
	// User must have typed at least mlen characters in
	// for there to even be a chance at a match
	//
	if( tlen < mlen )
		return false;
	
	// If user typed in more characters than are contained in s,
	// then there must be a mismatch
	//
	if( tlen > slen )
		return false;
	
	// Check the mandatory portion for mismatches
	//
	for( k = 0; k < mlen; k++ )
	{
  		tokenChar = (char)toupper( token[k] );
  		otherChar = s[k];
		if( tokenChar != otherChar )
			return false;
	}
	
	// Check the auxiliary portion for mismatches (if necessary)
	//
	for( k = mlen; k < tlen; k++ )
	{
  		tokenChar = (char)toupper( token[k] );
  		otherChar = (char)toupper( s[k] );
		if( tokenChar != otherChar )
			return false;
	}

	return true;
}

/**
 * @method AtEOF [bool:public]
 *
 * Returns true if and only if last call to GetNextToken
 * encountered the end-of-file character (or for some reason
 * the input stream is now out of commission).
 */
bool NexusToken::AtEOF()
{
	return atEOF;
}

/**
 * @method AtEOL [bool:public]
 *
 * Returns true if and only if last call to GetNextToken
 * encountered the newline character while the newlineIsToken
 * labile flag was in effect
 */
bool NexusToken::AtEOL()
{
	return atEOL;
}

/**
 * @method BlanksToUnderscores [void:public]
 *
 * Converts all blanks in token to underscore characters. Normally,
 * underscores found in the tokens read from a NEXUS file are converted
 * to blanks automatically as they are read; this function reverts
 * the blanks back to underscores.
 */
void NexusToken::BlanksToUnderscores()
{
	char tmp[256];
	int len = token.length();
	assert( len < 256 );
	strcpy( tmp, token.c_str() );
	for( int i = 0; i < len; i++ ) {
		if( tmp[i] == ' ' )
			tmp[i] = '_';	
	}
	token = tmp;
}
	
/**
 * @method Begins [bool:public]
 * @param s [nxsstring] the comparison string
 * @param respect_case [bool] determines whether comparison is case sensitive (false by default)
 *
 * Returns true if token nxsstring begins with the nxsstring s.
 * The comparison is case insensitive by default.  This function should
 * be used instead of the Equals function if you wish to
 * allow for abbreviations of commands.
 */
bool NexusToken::Begins( nxsstring s, bool respect_case /* = false */  )
{
	int k;
	char tokenChar, otherChar;

	unsigned int slen = s.size();
	if( slen > token.size() )
		return false;

	for( k = 0; k < slen; k++ )
	{
      if( respect_case ) {
         tokenChar = token[k];
         otherChar = s[k];
      }
      else {
   		tokenChar = (char)toupper( token[k] );
   		otherChar = (char)toupper( s[k] );
      }
		if( tokenChar != otherChar )
			return false;
	}

	return true;
}

/**
 * @method Equals [bool:public]
 * @param s [nxsstring] the comparison string
 * @param respect_case [bool] determines whether or not comparison is case sensitive (default is false)
 *
 * Returns true if token nxsstring exactly equals s.  The comparison
 * is case insensitive by default.  If abbreviations are to be allowed,
 * either Begins or Abbreviation should be used instead of Equals.
 */
bool NexusToken::Equals( nxsstring s, bool respect_case /* = false */ )
{
	unsigned int k;
	char tokenChar, otherChar;

	unsigned int slen = s.size();
	if( slen != token.size() )
		return false;

	for( k = 0; k < token.size(); k++ )
	{
      if( respect_case ) {
         tokenChar = token[k];
         otherChar = s[k];
      }
      else {
   		tokenChar = (char)toupper( token[k] );
   		otherChar = (char)toupper( s[k] );
      }
		if( tokenChar != otherChar )
			return false;
	}

	return true;
}

/**
 * @method GetFileColumn [void:public]
 *
 * Returns value stored in filecol, which keeps track of the current
 * column in the data file (i.e., number of characters since the last
 * new line was encountered).
 */
long  NexusToken::GetFileColumn()
{
	return filecol;
}

/**
 * @method GetFilePosition [streampos:public]
 *
 * Returns value stored in filepos, which keeps track of the current
 * position in the data file (i.e., number of characters since the
 * beginning of the file).  Note: for Metrowerks compiler, you must use the
 * offset() method of the streampos class to use the value returned.
 */
streampos  NexusToken::GetFilePosition()
{
	return filepos;
}

/**
 * @method GetFileLine [long:public]
 *
 * Returns value stored in fileline, which keeps track of the current
 * line in the data file (i.e., number of new lines encountered thus far).
 */
long  NexusToken::GetFileLine()
{
	return fileline;
}

/**
 * @method GetNextToken [void:public]
 * @throws XNexus
 *
 * Reads characters from in until a complete token has been read and
 * stored in token.
 * <p>GetNextToken performs a number of useful operations in the process
 * of retrieving tokens:
 * <ul>
 * <li> any underscore characters encountered are stored as blank spaces
 * <li> if the first character of the next token is an isolated single
 * quote, then the entire quoted nxsstring is saved as the next token
 * <li> paired single quotes are automatically converted to single quotes
 * before being stored
 * <li> comments are handled automatically (normal comments are treated as
 * whitespace and output comments are passed to the function OutputComment
 * which does nothing in the NexusToken class but can be overridden in a
 * derived class to handle these in an appropriate fashion)
 * <li> leading whitespace (including comments) is automatically skipped
 * <li> if the end of the file is reached on reading this token, the
 * atEOF flag is set and may be queried using the AtEOF member function
 * <li> punctuation characters are always returned as individual tokens
 * (see the Maddison, Swofford, and Maddison paper for the definition of
 * punctuation characters)
 * </ul>
 * <p>The behavior of GetNextToken may be altered by using labile flags.
 * For example, the labile flag saveCommandComments can be set using
 * the member function SetLabileFlagBit.  This will cause comments
 * of the form [&X] to be saved as tokens (without the square brackets),
 * but only for the aquisition of the next token.  Labile flags are cleared
 * after each application.
 */
void NexusToken::GetNextToken()
{
	ResetToken();

	char ch = ' ';
	if( saved == '\0' || IsWhitespace(saved) )
	{
		// skip leading whitespace
		while( IsWhitespace(ch) && !atEOF )
			ch = GetNextChar();
		saved = ch;
	}

	for(;;)
	{
		// break now if singleCharacterToken mode on and token length > 0
		if( labileFlags & singleCharacterToken && token.size() > 0 )
			break;

		// get next character either from saved or from input stream
		if( saved != '\0' ) {
			ch = saved;
			saved = '\0';
		}
		else
			ch = GetNextChar();

		// break now if we've hit EOF
		if( atEOF )	break;

		if( ch == '\n' && labileFlags & newlineIsToken ) {
         if( token.size() > 0 ) {
            // newline came after token, save newline until
            // next time when it will be reported as a separate
            // token
            atEOL = 0;
            saved = ch;
         }
         else {
            atEOL = 1;
            AppendToToken(ch);
         }
			break;
		}

		else if( IsWhitespace(ch) )
		{
			// break only if we've begun adding to token
			// (remember, if we hit a comment before a token,
			// there might be further white space between the comment and
			// the next token)
			if( token.size() > 0 )
				break;
		}

		else if( ch == '_' )
		{
			ch = ' ';
			AppendToToken(ch);
		}

		else if( ch == '[' )
		{
			// get rest of comment and deal with it, but notice
			// that we only break if the comment ends a token,
			// not if it starts one (comment counts as whitespace)
			//
			// in the case of command comments (if saveCommandComment) GetComment will
			// add to the token nxsstring, causing us to break because
			// token.size() will be greater than 0
			GetComment();
			if( token.size() > 0 )
				break;
		}

		else if( ch == '(' && labileFlags & parentheticalToken )
		{
			AppendToToken(ch);

			// get rest of parenthetical token
			GetParentheticalToken();
			break;
		}

		else if( ch == '{' && labileFlags & curlyBracketedToken )
		{
			AppendToToken(ch);

			// get rest of curly-bracketed token
			GetCurlyBracketedToken();
			break;
		}

		else if( ch == '\"' && labileFlags & doubleQuotedToken )
		{
			// get rest of double-quoted token
			GetDoubleQuotedToken();
			break;
		}

		else if( ch == '\'' )
		{
			if( token.size() > 0 ) {
            // we've encountered a single quote after a token has
            // already begun to be read; should be another tandem
            // single quote character immediately following
   			ch = GetNextChar();
            if( ch == '\'' )
      			AppendToToken(ch);
            else {
               errormsg = "Expecting second single quote character";
               throw XNexus( errormsg, GetFilePosition(), GetFileLine(), GetFileColumn() );
            }
         }
         else {
   			// get rest of quoted Nexus word and break, since
	   		// we will have eaten one token after calling GetQuoted
		   	GetQuoted();
         }
			break;
		}

		else if( IsPunctuation(ch) )
		{
			if( token.size() > 0 )
			{
				// if we've already begun reading the token, encountering
				// a punctuation character means we should stop, saving
				// the punctuation character for the next token
				saved = ch;
				break;
			}

			else
			{
				// if we haven't already begun reading the token, encountering
				// a punctuation character means we should stop and return
				// the punctuation character as this token (i.e., the token
				// is just the single punctuation character
				AppendToToken(ch);
				break;
			}
		}

		else
		{
			AppendToToken(ch);
		}

	}

	labileFlags = 0;
}

/**
 * @method GetToken [nxsstring:public]
 * @param respect_case [bool] determines whether token is converted to upper case before being returned
 *
 * Returns the token nxsstring. Specifying false for respect_case parameter causes all
 * characters in token to be converted to upper case before token nxsstring is returned.  Specifying
 * true (default) results in GetToken returning exactly what it read from the file.
 */
nxsstring NexusToken::GetToken( bool respect_case /* = true */ )
{
   if( !respect_case )
      ToUpper();
	return token;
}

/**
 * @method GetTokenLength [int:public]
 *
 * Returns token.size().
 */
int NexusToken::GetTokenLength()
{
	return token.size();
}

/**
 * @method IsPlusMinusToken [bool:public]
 *
 * Returns true if current token is a single character and this character
 * is either '+' or '-'.
 */
bool NexusToken::IsPlusMinusToken()
{
	if( token.size() == 1 && ( token[0] == '+' || token[0] == '-' )  )
		return true;
	else
		return false;
}

/**
 * @method IsPunctuationToken [bool:public]
 *
 * Returns true if current token is a single character and this character
 * is a punctuation character (as defined in IsPunctuation function).
 */
bool NexusToken::IsPunctuationToken()
{
	if( token.size() == 1 && IsPunctuation( token[0] ) )
		return true;
	else
		return false;
}

/**
 * @method IsWhitespaceToken [bool:public]
 *
 * Returns true if current token is a single character and this character
 * is a whitespace character (as defined in IsWhitespace function).
 */
bool NexusToken::IsWhitespaceToken()
{
	if( token.size() == 1 && IsWhitespace( token[0] ) )
		return true;
	else
		return false;
}

/**
 * @method ReplaceToken [void:public]
 * @param s [const nxsstring] nxsstring to replace current token nxsstring
 *
 * Replaces current token nxsstring with s.
 */
void NexusToken::ReplaceToken( const nxsstring s )
{
   token = s;
}

/**
 * @method ResetToken [void:public]
 *
 * Sets token to the empty nxsstring ("").
 */
void NexusToken::ResetToken()
{
   token = "";
}

/**
 * @method SetSpecialPunctuationCharacter [void:public]
 * @param c [char] the character to which special is set
 *
 * Sets the special punctuation character to c.  If the labile bit
 * useSpecialPunctuation is set, this character will be added to the
 * standard list of punctuation symbols, and will be returned as a
 * separate token like the other punctuation characters.
 */
void NexusToken::SetSpecialPunctuationCharacter( char c )
{
   special = c;
}

/**
 * @method SetLabileFlagBit [void:public]
 * @param bit [int] the bit (see enum) to set in labileFlags
 *
 * Sets the bit specified in the variable labileFlags.  The available
 * bits are specified in the enumeration starting with saveCommandComments.
 * All bits in labileFlags are cleared after each token is read.
 */
void NexusToken::SetLabileFlagBit( int bit )
{
   labileFlags |= bit;
}

/**
 * @method StoppedOn [void:public]
 * @param ch [char] the character to compare with saved character
 *
 * Checks character stored in the variable saved to see if it
 * matches supplied character ch.  Good for checking such things
 * as whether token stopped reading characters because it encountered
 * a newline (and labileFlags bit newlineIsToken was set):
 * <p>StoppedOn('\n');
 * <p>or whether token stopped reading characters because of a
 * punctuation character such as a comma:
 * <p>StoppedOn(',');
 */
bool NexusToken::StoppedOn( char ch )
{
   if( saved == ch )
      return true;
   else
      return false;
}

/**
 * @method StripWhitespace [void:public]
 *
 * Strips whitespace from currently-stored token.  Removes leading,
 * trailing, and embedded whitespace characters.
 */
void NexusToken::StripWhitespace()
{
	nxsstring s = "";
	for(unsigned int j = 0; j < token.size(); j++ ) {
		if( IsWhitespace( token[j] ) ) continue;
		s += token[j];
	}
	token = s;
}

/**
 * @method ToUpper [void:public]
 *
 * Converts all alphabetical characters in token to upper case.
 */
void NexusToken::ToUpper()
{
   for( unsigned int i = 0; i < token.size(); i++ )
      token[i] = (char)toupper( token[i] );
}

/**
 * @method Write [void:public]
 * @param out [ostream&] the output stream to which to write token nxsstring
 *
 * Simply outputs the current nxsstring stored in token to the output
 * stream out.  Does not send a newline to the output stream afterwards.
 */
void NexusToken::Write( ostream& out )
{
	out << token;
}

/**
 * @method Writeln [void:public]
 * @param out [ostream&] the output stream to which to write token nxsstring
 *
 * Simply outputs the current nxsstring stored in token to the output
 * stream out.  Sends a newline to the output stream afterwards.
 */
void NexusToken::Writeln( ostream& out )
{
	out << token << endl;
}

/**
 * @method OutputComment [virtual void:public]
 * @param msg [nxsstring&] the output comment to be displayed
 *
 * This function is called whenever an output comment (i.e., a comment
 * beginning with an exclamation point) is found in the data file.
 * This version of OutputComment does nothing; override this virtual
 * function to display the output comment in the most appropriate way
 * for the platform you are supporting.
 */
void NexusToken::OutputComment( nxsstring& /* msg */ )
{
}


