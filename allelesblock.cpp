// OPEN ISSUES:
// - variable alleles_fixed currently is not being used
// - alleles_fixed should be an array of bool, since user can specify 
//   allele labels for some loci and not others
// - HandleTransposedMatrix function still needs to be written

#include <iostream.h>
#include <stdio.h>

#include "nexusdefs.h"
#include "xnexus.h"
#include "nexustoken.h"
#include "nexus.h"
#include "setreader.h"
#include "taxablock.h"
#include "discretedatum.h"
#include "discretematrix.h"
#include "charactersblock.h"
#include "allelesblock.h"
#include "assumptionsblock.h"

/**
 * @class      AllelesBlock
 * @file       allelesblock.h
 * @file       allelesblock.cpp
 * @author     Paul O. Lewis
 * @copyright  Copyright © 1999. All Rights Reserved.
 * @variable   alleles_fixed [bool:private] if alleles_fixed is true, new alleles cannot be added
 * @variable   datapoint [datapoints:private] see enum datapoints for details
 * @variable   haploid [IntSet:protected] set of loci that are haploid
 * @variable   indivCount [int*:private] indivCount[i] is index of first individual in population i+1
 * @see        Nexus
 * @see        NexusBlock
 * @see        NexusReader
 * @see        NexusToken
 * @see        XNexus
 *
 * This class handles reading and storage for the Nexus block ALLELES.
 * It is derived from CharactersBlock, and overrides the member functions
 * Read and Reset, which are virtual functions in the base class.
 *
 * Because it is derived from the class CharactersBlock, this block
 * uses the DiscreteMatrix class for storing the data, and this
 * requires a little explanation.  The DiscreteMatrix class is designed for
 * storing an r X c array of integers.  The way it is used here, the rows
 * represent individuals and the columns loci.  In order to store data for
 * both genes at a given locus for diploid individuals, the integer stored
 * is broken up into a higher-order word and a lower-order word.  The macros
 * HIWORD and LOWORD can be used to extract these two components from the
 * integer value stored in the DiscreteMatrix structure.  Note that this
 * technique assumes integers are 4 bytes (or greater) in size, allowing
 * 2 bytes of storage for each of the two allelic states stored.  The maximum
 * integer that can be stored in 2 bytes is 0xFF, or 255, which is thus
 * the maximum number of alleles that can be defined for each locus.
 *
 * <p>The variable <b>gap</b> from the underlying CharactersBlock class 
 * has been reassigned to function as the separator character.
 *
 * <p>The array <b>indivCount</b> is used to store the row number of the 
 * first individual in the <i>next</i> population. Thus, the standard 
 * way to loop through all individuals in population i is:
 * <pre>
 * int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
 * for( j = 0; j < numIndivs; j++ )
 * {
 *   gene = GetGene( i, j, locus, 0 );
 *   if( gene < MAX_ALLELES ) {
 *     // do something with gene 0
 *   }
 *   else {
 *     // do nothing since gene 0 was missing data
 *   }
 *
 *   if( !IsHaploid(locus) )
 *   {
 *     gene = GetGene( i, j, locus, 1 );
 *     if( gene < MAX_ALLELES ) {
 *       // do something with gene 1
 *     }
 *     else {
 *       // do nothing since gene 1 was missing data
 *     }
 *   }
 * }
 * </pre>
 *
 * <P> Below is a table showing the correspondence between the elements of
 * an ALLELES block and the variables and member functions that can be used
 * to access each piece of information stored.
 *
 * <p><table border=1>
 * <tr>
 *   <th> Nexus command
 *   <th> Nexus subcommand
 *   <th> Data Members
 *   <th> Member Functions
 * <tr>
 *   <td rowspan=3> DIMENSIONS
 *   <td> NEWPOPS
 *   <td> bool <a href="CharactersBlock.html#newtaxa">newtaxa</a>
 *   <td>
 * <tr>
 *   <td> NPOP
 *   <td> int <a href="CharactersBlock.html#ntax">ntax</a> (see also <a href="CharactersBlock.html#ntaxTotal">ntaxTotal</a>)
 *   <td> int <a href="CharactersBlock.html#GetNTax">GetNTax()</a> (see also <a href="CharactersBlock.html#GetNumMatrixRows">GetNumMatrixRows()</a>)
 * <tr>
 *   <td> NLOCI
 *   <td> int <a href="CharactersBlock.html#nchar">nchar</a> (see also <a href="CharactersBlock.html#ncharTotal">ncharTotal</a>)
 *   <td> int <a href="CharactersBlock.html#GetNChar">GetNChar()</a> (see also <a href="CharactersBlock.html#GetNumMatrixCols">GetNumMatrixCols()</a>)
 * <tr>
 *   <td rowspan=9> FORMAT
 *   <td> DATAPOINT
 *   <td> datapoints <a href="#datapoint">datapoint</a>
 *   <td> int <a href="#GetDataPoint">GetDataPoint()</a>
 * <tr>
 *   <td> RESPECTCASE
 *   <td> int <a href="CharactersBlock.html#respectingCase">respectingCase</a>
 *   <td> int <a href="CharactersBlock.html#IsRespectCase">IsRespectCase()</a>
 * <tr>
 *   <td> MISSING
 *   <td> char <a href="CharactersBlock.html#missing">missing</a>
 *   <td> char <a href="CharactersBlock.html#GetMissingSymbol">GetMissingSymbol()</a>
 * <tr>
 *   <td> SEPARATOR
 *   <td> char <a href="CharactersBlock.html#gap">gap</a>
 *   <td> char <a href="#GetSeparatorSymbol">GetSeparatorSymbol()</a>
 * <tr>
 *   <td> EQUATE
 *   <td> AssocList <a href="CharactersBlock.html#equates">equates</a>
 *   <td> char* <a href="CharactersBlock.html#GetEquateKey">GetEquateKey( int k )</a>
 *        <br> char* <a href="CharactersBlock.html#GetEquateValue">GetEquateValue( int k )</a>
 *        <br> int <a href="CharactersBlock.html#GetNumEquates">GetNumEquates()</a>
 * <tr>
 *   <td> (NO)LABELS
 *   <td> int <a href="CharactersBlock.html#labels">labels</a>
 *   <td> int <a href="CharactersBlock.html#IsLabels">IsLabels()</a>
 * <tr>
 *   <td> TRANSPOSE
 *   <td> int <a href="CharactersBlock.html#transposing">transposing</a>
 *   <td> int <a href="CharactersBlock.html#IsTranspose">IsTranspose()</a>
 * <tr>
 *   <td> INTERLEAVE
 *   <td> int <a href="CharactersBlock.html#interleaving">interleaving</a>
 *   <td> int <a href="CharactersBlock.html#IsInterleave">IsInterleave()</a>
 * <tr>
 *   <td> (NO)TOKENS
 *   <td> int <a href="CharactersBlock.html#tokens">tokens</a>
 *   <td> int <a href="CharactersBlock.html#IsTokens">IsTokens()</a>
 * <tr>
 *   <td rowspan=1 colspan=2 align=left> ELIMINATE
 *   <td> int* <a href="CharactersBlock.html#eliminated">eliminated</a>
 *   <td> int <a href="CharactersBlock.html#IsEliminated">IsEliminated( int origCharIndex )</a>
 *        <br> int <a href="CharactersBlock.html#GetNumEliminated">GetNumEliminated()</a>
 * <tr>
 *   <td rowspan=1 colspan=2 align=left> MATRIX 
 *   <td> DiscreteMatrix* <a href="CharactersBlock.html#matrix">matrix</a>
 *   <td> char <a href="#GetState">GetState( int i, int j, int k = 0 )</a>
 *        <br> int <a href="#GetInternalRepresentation">GetInternalRepresentation( int i, int j, int k = 0 )</a>
 *        <br> int <a href="#GetNumStates">GetNumStates( int i, int j )</a>
 *        <br> int <a href="#GetNumMatrixRows">GetNumMatrixRows()</a>
 *        <br> int <a href="#GetNumMatrixCols">GetNumMatrixCols()</a>
 *        <br> int <a href="#IsPolymorphic">IsPolymorphic( int i, int j )</a>
 * </table>
 */

/**
 * @enumeration
 * @enumitem  standard [1] means standard datapoint
 * @enumitem  fraglen  [2] means fraglen datapoint
 *
 * For use with the variable datapoint.  Default is 1 (standard datapoint).
 */

/**
 * @constructor
 *
 * Performs the following initializations:
 * <table>
 * <tr><th align="left">Variable <th> <th align="left"> Initial Value
 * <tr><td> alleles_fixed  <td>= <td> false
 * <tr><td> datapoint      <td>= <td> AllelesBlock::standard
 * <tr><td> gap            <td>= <td> '/'
 * <tr><td> id             <td>= <td> "ALLELES"
 * <tr><td> indivCount     <td>= <td> NULL
 * <tr><td> labels         <td>= <td> false
 * <tr><td> respectingCase <td>= <td> true
 * <tr><td> tokens         <td>= <td> true
 * </table>
 */
AllelesBlock::AllelesBlock( TaxaBlock& tb, AssumptionsBlock& ab )
	: CharactersBlock( tb, ab )
{
	// Thinking of changing any of these defaults?
	// If so, do it also in the Reset function.
	//
	alleles_fixed  = false;
	datapoint      = standard;
	gap            = '/';
	id             = "ALLELES";
	labels         = false;
	respectingCase = true;
	tokens         = true;
	indivCount     = NULL;
}

/**
 * @destructor
 *
 * Frees memory allocated for the indivCount array.
 */
AllelesBlock::~AllelesBlock()
{
	if( indivCount != NULL ) 
		delete [] indivCount;
}

/**
 * @method AlleleCount [int:public]
 * @param allele [int] the allele in question
 * @param locus [int] the locus in question, in range [0..nloci)
 * @param pop [int] the population in question, in range [0..npops) (default value -1)
 *
 * Returns the number of genes that are identical to allele.
 * Assumes locus has not been excluded.  If no population is
 * supplied, sums across all active populations.  If population
 * is supplied, count will only be for that population.  Assumes
 * that if population is specified, that population is not one 
 * currently deleted by the user.
 *
 * Note: this is a relatively slow function because an allele count 
 * is performed for each call.
 */
int AllelesBlock::AlleleCount( int allele, int locus, int pop /* = -1 */ )
{
	assert( locus >= 0 && locus < nchar );
	assert( pop > -2 && pop < ntax );
	assert( !IsExcluded(locus) );

	bool do_one_pop = ( pop >= 0 );
	assert( !( do_one_pop && IsDeleted(pop) ) );

	int i, j, gene;
	int allele_count = 0;

	i = ( do_one_pop ? pop : 0 );
	for( ; i < ntax; i++ )
	{
		if( do_one_pop && i > pop )
			break;
		if( IsDeleted(i) ) continue;
		int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
		for( j = 0; j < numIndivs; j++ )
		{
			gene = GetGene( i, j, locus, 0 );
			if( gene < MAX_ALLELES ) {
				if( gene == allele )
					allele_count++;
			}
		
			if( !IsHaploid(locus) )
			{
				gene = GetGene( i, j, locus, 1 );
				if( gene < MAX_ALLELES ) {
					if( gene == allele )
						allele_count++;
				}
			}
		}
	}

	return allele_count;
}

/**
 * @method AlleleFrequency [double:public]
 * @param allele [int] the allele in question
 * @param locus [int] the locus in question, in range (0..nchar]
 * @param pop [int] the population in question, in range (0..ntax] (default is -1)
 * @throws XAllMissingData
 *
 * Returns observed frequency (expressed as a proportion)
 * of the allele specified at the specified locus in the specified
 * population.  Assumes locus has not been excluded. If population
 * not specified, allele frequency over all populations except
 * those currently deleted will be returned.  Assumes
 * that if population is specified, that population is not one 
 * currently deleted by the user.  Throws an XAllMissingData
 * exception if the total number of alleles of all types is zero.
 *
 * Note: this is a relatively slow function because an allele count 
 * is performed for each call.
 */
double AllelesBlock::AlleleFrequency( int allele, int locus, int pop /* = -1 */ )
{
	assert( locus >= 0 && locus < nchar );
	assert( pop > -2 && pop < ntax );
	assert( !IsExcluded(locus) );

	bool do_one_pop = ( pop >= 0 );
	assert( !( do_one_pop && IsDeleted(pop) ) );

	int i, j, gene;
	int total = 0;
	int allele_count = 0;

	i = ( do_one_pop ? pop : 0 );
	for( i; i < ntax; i++ )
	{
		if( do_one_pop && i > pop )
			break;
		if( IsDeleted(i) ) continue;
		int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
		for( j = 0; j < numIndivs; j++ )
		{
			gene = GetGene( i, j, locus, 0 );
			if( gene < MAX_ALLELES ) {
				total++;
				if( gene == allele )
					allele_count++;
			}
			
			if( !IsHaploid(locus) )
			{
				gene = GetGene( i, j, locus, 1 );
				if( gene < MAX_ALLELES ) {
					total++;
					if( gene == allele )
						allele_count++;
				}
			}
		}
	}

	if( total == 0 )
		throw XAllMissingData();
		
	double freq = (double)allele_count / (double)total;

	return freq;
}

/**
 * @method DebugShowMatrix [int:protected]
 * @param out [ostream&] output stream on which to print matrix
 * @param marginText [char*] nxsstring to print first on each line
 *
 * Provides a dump of the contents of the matrix variable.  Useful for testing
 * whether data is being read as expected.  The default for marginText is NULL,
 * which has the effect of placing the matrix output flush left.  If each line
 * of output should be prefaced with a tab character, specify marginText = "\t".
 */
void AllelesBlock::DebugShowMatrix( ostream& out, char* marginText /* = NULL */ )
{
	int pop, indiv, locus;
	
	for( pop = 0; pop < ntax; pop++ )
	{
		if( marginText != NULL )
		out << marginText;
		
		int origPopIndex =  GetOrigTaxonIndex(pop); // this function still needs to be written
		nxsstring currTaxonLabel = taxa.GetTaxonLabel( origPopIndex );
		out << currTaxonLabel << ":" << endl;
		
		int last = indivCount[pop];
		if( pop > 0 )
			last -= indivCount[pop-1];
		for( indiv = 0; indiv < last; indiv++ )
		{
			if( marginText != NULL )
				out << marginText;
			out << setw(5) << (indiv+1);
			
			for( locus = 0; locus < nchar; locus++ )
			{
				int gene0 = GetGene( pop, indiv, locus, 0 );
				out << "  ";
				if( gene0 == 0xff )
					out << "?";
				else
					out << gene0;
				
				if( !IsHaploid(locus) ) 
				{
					int gene1 = GetGene( pop, indiv, locus, 1 );
					out << "/";
					if( gene1 == 0xff )
						out << "?";
					else
						out << gene1;
				}
			}
			out << endl;
		}
	}
}

/**
 * @method FocalAlleleCount [void:public]
 * @param focal_allele [int] the allele to focus on (i.e., A)
 * @param locus [int] the locus in question, in range (0..nchar]
 * @param pop [int] the population in question, in range (0..ntax]
 * @param n_AA [int&] the place to store the count of AA genotypes
 * @param n_Aa [int&] the place to store the count of Aa genotypes
 * @param n_aa [int&] the place to store the count of aa genotypes
 * @throws XAllMissingData
 *
 * If the focal_allele is "A", then this function counts up the 
 * number of "AA", "Aa", and "aa" genotypes and returns these counts
 * in n_AA, n_Aa, and n_aa, respectively.  It is assumed that locus
 * is not excluded and pop is not deleted.  It is also assumed that
 * the data for locus are diploid.
 *
 * Note: this is a relatively slow function because genotypes are counted 
 * anew each time this function is called.
 */
void AllelesBlock::FocalAlleleCount( int focal_allele, int locus, int pop
	, int& n_AA, int& n_Aa, int& n_aa )
{
	assert( locus >= 0 && locus < nchar );
	assert( pop >= 0 && pop < ntax );
	assert( !IsDeleted(pop) );
	assert( !IsExcluded(locus) );
	assert( !IsHaploid(locus) );

	int j, gene0, gene1;
	n_AA = n_Aa = n_aa = 0;

	int numIndivs = ( pop > 0 ? indivCount[pop] - indivCount[pop-1] : indivCount[pop] );
	for( j = 0; j < numIndivs; j++ )
	{
		gene0 = GetGene( pop, j, locus, 0 );
		gene1 = GetGene( pop, j, locus, 1 );
		if( gene0 == MAX_ALLELES || gene1 == MAX_ALLELES )
			continue;
		if( gene0 == focal_allele && gene1 == focal_allele )
			n_AA++;
		else if( gene0 == focal_allele && gene1 != focal_allele )
			n_Aa++;
		else if( gene0 != focal_allele && gene1 == focal_allele )
			n_Aa++;
		else 
			n_aa++;
	}
}
	
/**
 * @method GenotypeCount [int:public]
 * @param allele1 [int] one of the two alleles in question
 * @param allele2 [int] the other of the alleles in question
 * @param locus [int] the locus in question, in range (0..nchar]
 * @param pop [int] the population in question, in range (0..ntax] (default is -1)
 * @throws XAllMissingData
 *
 * Returns the number of genotypes for which one gene is
 * the same as allele1 and the other is the same as allele2.
 * The order in which alleles are supplied does not matter
 * (i.e., the number of heterozygotes having genotype 1/2
 * could be queried in either of the following ways:
 * <pre>
 *   GenotypeCount( 1, 2, pop, locus )
 *   GenotypeCount( 2, 1, pop, locus )
 * </pre>
 * <p>Assumes locus has not been excluded and data is
 * diploid (for haploid data use AlleleCount instead).
 * If population is not specified, returns count over all
 * populations not currently deleted, otherwise returns count for
 * specified population only. Assumes that if population is specified, 
 * that population is not one currently deleted by the user. 
 * Throws XAllMissingData if only missing data encountered
 * in the populations considered for the specified locus.
 *
 * Note: this is a relatively slow function because a genotype count 
 * is performed for each call.
 */
int AllelesBlock::GenotypeCount( int allele1, int allele2, int locus, int pop /* = -1 */ )
{
	assert( locus >= 0 && locus < nchar );
	assert( pop > -2 && pop < ntax );
	assert( !IsExcluded(locus) );
	assert( !IsHaploid(locus) );

	bool do_one_pop = ( pop >= 0 );
	assert( !( do_one_pop && IsDeleted(pop) ) );

	int i, j, gene0, gene1;
	int genotype_count = 0;

	i = ( do_one_pop ? pop : 0 );
	for( ; i < ntax; i++ )
	{
		if( do_one_pop && i > pop )
			break;
		if( IsDeleted(i) ) continue;
		int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
		for( j = 0; j < numIndivs; j++ )
		{
			gene0 = GetGene( i, j, locus, 0 );
			gene1 = GetGene( i, j, locus, 1 );
			if( gene0 < MAX_ALLELES && gene1 < MAX_ALLELES ) 
			{
				if( gene0 == allele1 && gene1 == allele2 )
					genotype_count++;
				else if( gene0 == allele2 && gene1 == allele1 )
					genotype_count++;
			}
		}
	}

	return genotype_count;
}

/**
 * @method GetLocusLabel [int:public]
 * @param locus [int] the locus in the range [0..nloci)
 *
 * Returns the nxsstring label for the locus requested.
 */
nxsstring AllelesBlock::GetLocusLabel( int locus )
{
	assert( locus >= 0 );
	assert( locus < nchar );
	return charLabels[locus];
}

/**
 * @method GetAlleleLabel [int:public]
 * @param locus [int] the locus in the range [0..nloci)
 * @param allele [int] the allele
 *
 * Returns the nxsstring label for the allele requested.
 */
nxsstring AllelesBlock::GetAlleleLabel( int locus, int allele )
{
	nxsstring allele_label = "no-name";
   LabelListBag::const_iterator cib = charStates.find(locus);
   if( cib != charStates.end() )
   {
      int ns = (*cib).second.size();
      if( allele >= 0 && allele < ns )
      	allele_label = (*cib).second[allele];
   }
   return allele_label;
}

/**
 * @method GetIndivCount [int:public]
 * @param pop [int] the population
 *
 * Returns value of indivCount[pop], which is the row number of
 * the first individual in population pop+1.
 */
int AllelesBlock::GetIndivCount( int pop )
{
	return indivCount[pop];
}

/**
 * @method GetGene [int:public]
 * @param pop [int] the population
 * @param indiv [int] the individual (0-offset relative to first individual in population)
 * @param locus [int] the locus
 * @param gene [int] if 0, low-order word returned; high-order word returned otherwise
 *
 * Gets the low-order 2-byte word (if last parameter is 0) or
 * the high-order 2-byte word (if last parameter is 1) and
 * returns this as an int value.  If the word returned is
 * exactly 0xff (= 255), this should be interpreted as missing
 * data. The macro MAX_ALLELES (defined in the allelesblock.h header file)
 * equals 0xff and can thus be used to test the return value.
 */
int AllelesBlock::GetGene( int pop, int indiv, int locus, int gene )
{
   //typedef long LONG;
   //typedef unsigned long       DWORD;
   //typedef unsigned short      WORD;
   //#define LOWORD(l)           ((WORD)(l))
   //#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
   //int szWORD = sizeof(unsigned short);
   //int szDWORD = sizeof(unsigned long);
   //int szLONG = sizeof(long);
   //int sz_int = sizeof(int);
   //cerr << szWORD << "," << szDWORD << "," << szLONG << "," << sz_int << endl;
   int gene_val;
   int row = indiv + ( pop > 0 ? indivCount[pop-1] : 0 );
   assert( !matrix->IsGap( row, locus ) );
   if( matrix->IsMissing( row, locus ) )
      gene_val = 0xff;
   else {
      if( gene == 0 )
         gene_val = (unsigned short)( matrix->GetState( row, locus ) );
      else
         gene_val = (unsigned short)( ( (unsigned long)( matrix->GetState( row, locus ) ) >> 16 ) & 0xFFFF );
   }
   return gene_val;
}

/**
 * @method GetNumHaploid [int:public]
 *
 * Returns number of items in haploid vector.  Number returned does
 * take account of eliminated loci (i.e., eliminated haploid loci are
 * not included in the tally) but not excluded ones.
 */
int AllelesBlock::GetNumHaploid()
{
	if( haploid.empty() )
		return 0;
		
	if( GetNumEliminated() == 0 )
		return haploid.size();
		
	// Some loci have been eliminated, so must check each
	// one listed in haploid vector to make sure it was not
	// eliminated
	//
	int n = 0;
	IntSet::const_iterator i = haploid.begin();
	for( ; i != haploid.end(); ++i ) {
		int origIndex = (*i);
		if( IsEliminated(origIndex) )
			continue;
		n++;
	}
	return n;
}

/**
 * @method HandleAllele [int:protected]
 * @param token [NexusToken&] the token used to read from in
 * @param j [int] the locus index, in range [0..nchar)
 * @throws XNexus
 *
 * Called from HandleNextGenotype to read in the next state when 'tokens'
 * is in effect.
 * If a LOCUSALLELELABELS or ALLELELABELS command was included in the ALLELES
 * block, HandleTokenState looks up the token in charStates to make
 * sure it is a valid state, then returns the allele's value (0, 1, 2, ...).
 * If no list of valid allelic states was prespecified, HandleTokenState has
 * no way to check the validity of the allele and simply adds it to charStates,
 * returning the allele's value (0, 1, 2, ...).
 * Note: HandleTokenState does NOT handle adding the allele's value to the
 * matrix. Save the return value, let's call it x, and use the following
 * command to add it to matrix: matrix->AddState( i, j, x ).
 */
int AllelesBlock::HandleAllele( NexusToken& token, int j )
{
   int k = 0;

   // First check to see if token equals the missing data symbol.
   // If so, return 0xFF to indicate that missing data was specified.
   //
	if( token.GetTokenLength() == 1 && token.GetToken()[0] == missing )
		return 0xff;

   // If alleles were prespecified in ALLELELABELS or LOCUSALLELELABELS command,
   // check the current token against the list of valid allele labels
   // If alleles were not prespecified, so go ahead and add current token to the
   // list of valid alleles for locus j, obtaining the allele's integer designation
   // in the process to return to the calling function.
   //

   // Check to see if any alleles are listed for locus j in charStates
   //
   if( charStates.find(j) == charStates.end() )
   {
      if( alleles_fixed ) {
         errormsg = "No alleles were defined for character ";
         errormsg += ( 1 + GetOrigCharIndex(j) );
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
      else {
         charStates[j].push_back( token.GetToken() );
         return 0; // first allele in the list
      }
   }

   // Some alleles have already been added to the list of alleles stored in
   // charStates for locus j.  Check to see if token is one of these.
   //
   LabelListBag::const_iterator bagIter = charStates.find(j);
   int nAlleles = (*bagIter).second.size();
   LabelList::const_iterator ci_begin = (*bagIter).second.begin();
   LabelList::const_iterator ci_end = (*bagIter).second.end();
   nxsstring t = token.GetToken( respectingCase );
   LabelList::const_iterator cit;
   if( respectingCase )
      cit = find( ci_begin, ci_end, t );
   else
      cit = find_if( ci_begin, ci_end, bind2nd( stri_equal(), t ) );

   if( cit == ci_end )
   {
      if( alleles_fixed ) {
         errormsg = "Allele ";
         errormsg += t;
         errormsg += " not defined for locus ";
         errormsg += ( 1 + GetOrigCharIndex(j) );
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
      else {
         charStates[j].push_back( token.GetToken() );
         return nAlleles;
      }
   }

   // ok, the allele has been identified, so return the alleles's internal
   // representation.  That is, if the list of allele labels was
   // "fast medium slow" and "slow" was specified in the data file,
   // state saved in matrix would be 2 (it would be 1 if "medium" were
   // specified in the data file, and 0 if "fast" were specified in the
   // data file).
   k = ( cit - ci_begin );

	return k;
}

/**
 * @method HandleFormat [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when FORMAT command needs to be parsed from within the
 * DIMENSIONS block.  Deals with everything after the token FORMAT
 * up to and including the semicolon that terminates the FORMAT
 * command.
 */
void AllelesBlock::HandleFormat( NexusToken& token )
{
	bool standardDataTypeAssumed = false;
	bool ignoreCaseAssumed = false;

	for(;;)
	{
		token.GetNextToken();

		if( token.Equals("DATAPOINT") )
		{

			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword DATAPOINT but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be one of the following: STANDARD, FRAGLEN
			token.GetNextToken();
			if( token.Equals("STANDARD") )
				datapoint = standard;
			else if( token.Equals("FRAGLEN") )
				datapoint = fraglen;
			else {
				errormsg = token.GetToken();
            errormsg += " is not a valid DATAPOINT within a ";
            errormsg += id;
            errormsg += " block";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			if( standardDataTypeAssumed && datapoint != standard ) {
				errormsg = "DATAPOINT must be specified first in FORMAT command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			if( datapoint == fraglen )
				tokens = true;
		}

		else if( token.Equals("RESPECTCASE") )
		{
			if( ignoreCaseAssumed ) {
				errormsg = "RESPECTCASE must be specified before MISSING and SEPARATOR in FORMAT command";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			standardDataTypeAssumed = true;
			respectingCase = true;
		}

		else if( token.Equals("MISSING") )
		{
			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword MISSING but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the missing data symbol (single character)
			token.GetNextToken();
			if( token.GetTokenLength() != 1 ) {
				errormsg = "MISSING symbol should be a single character, but ";
            errormsg += token.GetToken();
            errormsg += " was specified";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsPunctuationToken() && !token.IsPlusMinusToken() ) {
				errormsg = "MISSING symbol specified cannot be a punctuation token (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsWhitespaceToken() ) {
				errormsg = "MISSING symbol specified cannot be a whitespace character (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			missing = token.GetToken()[0];

			ignoreCaseAssumed = true;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("NOSEPARATOR") )
		{
         gap = '\0';
      }

		else if( token.Equals("SEPARATOR") )
		{
			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword SEPARATOR but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be the separator symbol (single character)
			token.GetNextToken();
			if( token.GetTokenLength() != 1 ) {
				errormsg = "SEPARATOR symbol should be a single character, but ";
            errormsg += token.GetToken();
            errormsg += " was specified";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsPunctuationToken() && !token.IsPlusMinusToken() && !token.Equals("/") ) {
				errormsg = "SEPARATOR symbol specified cannot be a punctuation token (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}
			else if( token.IsWhitespaceToken() ) {
				errormsg = "SEPARATOR symbol specified cannot be a whitespace character (";
            errormsg += token.GetToken();
            errormsg += " was specified)";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			gap = token.GetToken()[0];

			ignoreCaseAssumed = true;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("EQUATE") )
		{
			// this should be an equals sign
			token.GetNextToken();
			if( !token.Equals("=") ) {
				errormsg = "Expecting '=' after keyword EQUATE but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

			// this should be a double-quote character
			token.GetNextToken();
			if( !token.Equals("\"") ) {
				errormsg = "Expecting '\"' after keyword EQUATE but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
			}

         // loop until second double-quote character is encountered
         for(;;)
			{
   			token.GetNextToken();
   			if( token.Equals("\"") )
               break;

            // if token is not a double-quote character, then it must be
            // an equate token (i.e., the token to be replaced in
            // the data matrix)

            // check for bad choice of equate symbol
            if( token.GetTokenLength() == 1 )
            {
               nxsstring t = token.GetToken();
               char ch = t[0];
               bool badEquateSymbol = false;

               // the character '^' cannot be an equate symbol
               if( ch == '^' )
                  badEquateSymbol = true;

               // equate symbols cannot be punctuation (except for + and -)
               if( token.IsPunctuationToken() && !token.IsPlusMinusToken() )
                  badEquateSymbol = true;

               // equate symbols cannot be same as missing or separator
               if( ch == missing || ch == gap )
                  badEquateSymbol = true;

               if( badEquateSymbol ) {
                  errormsg = "EQUATE symbol specified (";
                  errormsg += token.GetToken();
                  errormsg += ") is not valid; must not be same as missing, \nseparator, or any of the following: ()[]{}/\\,;:=*'\"`<>^";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }
            }

            nxsstring k = token.GetToken();

            // this should be an equals sign
				token.GetNextToken();
            if( !token.Equals("=") ) {
               errormsg = "Expecting '=' in EQUATE definition but found ";
               errormsg += token.GetToken();
               errormsg += " instead";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }

            // this should be the token to be substituted in for the equate symbol
            token.SetLabileFlagBit( NexusToken::parentheticalToken );
            token.SetLabileFlagBit( NexusToken::curlyBracketedToken );
            token.GetNextToken();
            nxsstring v = token.GetToken();

            // add the new equate association to the equates list
            equates[k] = v;
			}

			standardDataTypeAssumed = true;
		}

		else if( token.Equals("LABELS") )
		{
			labels = true;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("NOLABELS") )
		{
			labels = false;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("TRANSPOSE") )
		{
			transposing = true;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("INTERLEAVE") )
		{
			interleaving = true;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("TOKENS") )
		{
			tokens = true;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals("NOTOKENS") )
		{
			tokens = false;
			standardDataTypeAssumed = true;
		}

		else if( token.Equals(";") ) {
			break;
		}

      else {
         errormsg = "Unrecognized keyword (";
         errormsg += token.GetToken();
         errormsg += ") encountered in FORMAT command";
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
	}
}

/**
 * @method HandleHaploid [void:protected]
 * @param token [NexusToken&] the token used to read from in
 *
 * Called when HAPLOID command needs to be parsed from within the
 * ALLELES block.  Deals with everything after the token HAPLOID
 * up to and including the semicolon that terminates the HAPLOID
 * command.  Any character numbers or ranges of character numbers
 * specified are stored in the IntSet haploid, which is empty
 * until a HAPLOID command is encountered, if ever.  Note that
 * like all sets the character ranges are adjusted so that their
 * offset is 0.  For example, given "haploid 4-7;" in the data
 * file, the haploid list would contain the values 3, 4, 5,
 * and 6 (not 4, 5, 6, and 7).
 */
void AllelesBlock::HandleHaploid( NexusToken& token )
{
   // construct an object of type SetReader, then call its run function
   // to store the set in the haploid set
   //
	SetReader( token, ncharTotal, haploid, *this, SetReader::charset ).Run();

	//OPEN ISSUE: perhaps it is better to just create a local
	// haploid set, fill it using the above line, then translate
	// that to a bool array for use from this point on
}

/**
 * @method HandleMatrix [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called when MATRIX command needs to be parsed from within the
 * ALLELES block.  Deals with everything after the token MATRIX
 * up to and including the semicolon that terminates the MATRIX
 * command.
 */
void AllelesBlock::HandleMatrix( NexusToken& token )
{
   int i, j, k;
   
	if( transposing ) {
		errormsg = "Sorry, transposed ALLELES block matrix not supported at this time";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

	if( datapoint == fraglen ) {
		errormsg = "Sorry, fraglen datapoint in ALLELES block not supported at this time";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	if( ntaxTotal == 0 )
		ntaxTotal = taxa.GetNumTaxonLabels();

	if( ntax == 0 ) {
		errormsg = "Cannot create ALLELES block matrix: there are 0 populations specified";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	if( nchar == 0 ) {
		errormsg = "Cannot create ALLELES block matrix: there are 0 loci specified";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

   // create a matrix to begin with, we will need to expand it as we go
   // since there will almost always be more than one individual per population
	if( matrix != NULL )
		delete matrix;
	matrix = new DiscreteMatrix( ntax, nchar );

	// Allocate memory for (and initialize) the arrays activeTaxon and activeChar.
	// All loci (i.e., "characters") and all populations (i.e., "taxa") are initially active.
	//
	activeTaxon = new bool[ntax];
	for( i = 0; i < ntax; i++ )
		activeTaxon[i] = true;
		
	activeChar = new bool[nchar];
	for( j = 0; j < nchar; j++ )
		activeChar[j] = true;

	// Allocate memory for (and initialize) the array indivCount
	//
	indivCount = new int[ntax];
	for( i = 0; i < ntax; i++ )
		indivCount[i] = 0;
	
	// The value of ncharTotal is normally identical to the value of nchar specified
	// in the ALLELES block DIMENSIONS command.  If an ELIMINATE command is
	// processed, however, nchar < ncharTotal.  Note that the ELIMINATE command
	// will have already been read by now, and the ELIMINATEd locus numbers
	// will be stored in the array eliminated.
	//
	if( charPos != NULL )
		delete [] charPos;
	charPos = new int[ncharTotal];

	k = 0;
	for( j = 0; j < ncharTotal; j++ ) {
		if( IsEliminated(j) )
			charPos[j] = -1;
		else
			charPos[j] = k++;
	}

	// The value of ntaxTotal equals the number of taxa specified in the
	// TAXA block, whereas ntax equals the number of taxa specified in
	// the DIMENSIONS command of the ALLELES block.  These two numbers
	// will be identical unless some taxa have been left out of the MATRIX
	// command of the ALLELES block, in which case ntax < ntaxTotal. We
	// haven't yet read the MATRIX however, so for now we just initialize
	// the taxonPos array to -1.
	//
	if( taxonPos != NULL )
		delete [] taxonPos;
	taxonPos = new int[ntaxTotal];
	for( i = 0; i < ntaxTotal; i++ )
		taxonPos[i] = -1;

	// HandleTransposedMatrix and HandleStdMatrix both take care of
	// reading the semicolon that terminates the MATRIX command
	//
	if( transposing )
		HandleTransposedMatrix( token );
	else
		HandleStdMatrix( token );
	
	// If we've gotten this far, presumably it is safe to
	// tell the ASSUMPTIONS block that were ready to take on
	// the responsibility of being the current character-containing
	// block (to be consulted if characters are excluded or included)
	assumptionsBlock.SetCallback(this);
}

/**
 * @method HandleNextGenotype [bool:protected]
 * @param token [NexusToken&] the token used to read from in
 * @param i [int] the index of the row of the data matrix where this genotype should be stored
 * @param locus [int] the (original) locus index, in range [0..ncharTotal)
 * @param stopOnNewline [bool] if interleaving, stop on encountering newline character (true by default)
 * @throws XNexus
 *
 * Called from HandleStdMatrix or HandleTransposedMatrix function
 * to read in the next genotype.  Always returns true except in the
 * following cases, in which case it returns false:
 * <ul>
 * <li>if in interleave mode and a newline character is encountered
 *     before any characters of the next token are read (unless
 *     stopOnNewline is false)
 * <li>if a comma, colon or semicolon is encountered before any
 *     characters of the next token are read
 * </ul>
 * The row index (i) is the absolute row number into the matrix.  The row
 * number into the matrix is indivCount[i-1] + j (if i > 0), where j is the
 * position of an individual relative to the first individual in population
 * i (j = 0, 1, ...).
 */
bool AllelesBlock::HandleNextGenotype( NexusToken& token, int i, int locus
	, bool stopOnNewline /* = true */ )
{
	int x, y;

   // It is possible that locus currChar has been ELIMINATEd, in
   // which case we need to go through the motions of reading in the
   // data but we don't store it.  The variable k will be our guide
   // when it comes time to store data since k will be -1 for
   // characters that were ELIMINATEd and will be set to the correct
   // row for characters that have not been ELIMINATEd.
   //
   int k = charPos[locus];

	// This should be the genotype for population i, individual j, and locus k
   // Note that if this is the first individual (i.e., j == 0) and the matrix
   // is interleaved, we need to ignore leading newline characters since
   // the population label is normally placed on the line above the data
   // for the first individual.
	//
   token.SetSpecialPunctuationCharacter( gap );
   if( gap != '\0' )
      token.SetLabileFlagBit( NexusToken::useSpecialPunctuation );
   if( interleaving && stopOnNewline )
      token.SetLabileFlagBit( NexusToken::newlineIsToken );
   if( !tokens )
      token.SetLabileFlagBit( NexusToken::singleCharacterToken );
   token.SetLabileFlagBit( NexusToken::parentheticalToken );
   token.SetLabileFlagBit( NexusToken::curlyBracketedToken );
   token.GetNextToken();

	if( interleaving && token.AtEOL() )
		return false;

	if( token.Equals(",") || token.Equals(";") || token.Equals(":") )
		return false;
		
	// Allele should not be a punctuation character
	//
	if( token.IsPunctuationToken() ) {
		errormsg = "Punctuation character (";
		errormsg += token.GetToken();
		errormsg += ") found where allele name expected";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	// Make sure we didn't run out of file
	//
	if( token.AtEOF() ) {
		errormsg = "Unexpected end of file encountered";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	// If we didn't run out of file, I see no reason why we should
	// have a zero-length token on our hands
	assert( token.GetTokenLength() > 0 );

	// We've read in the allele now, so if this locus has been
	// ELIMINATEd, we want to skip the part that actually saves it
	//
	if( k >= 0 )
   {
      // see if any equate macros apply
      //
      nxsstring skey = nxsstring( token.GetToken( respectingCase ) );
      AssocList::iterator p = equates.find( skey );
      if( p != equates.end() ) {
         nxsstring sval = (*p).second;
         token.ReplaceToken( sval.c_str() );
      }

      // token should be in one of the following forms: "{"  "a"  "bb"
      //
      int polymorphism = token.Equals("(");
      int uncertainty  = token.Equals("{");

      if( !uncertainty && !polymorphism )
      {
         x = HandleAllele( token, k );
         if( x > MAX_ALLELES ) {
            errormsg = "Number of alleles for locus ";
            errormsg += (k+1);
            errormsg += " has exceeded limit of ";
            errormsg += MAX_ALLELES;
            throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
         matrix->SetState( i, k, x );
      }
      else
      {
         // borrow material from CharactersBlock::HandleNextState if and when
         // ambiguity and polymorphism are implemented for the ALLELES block
         //
         errormsg = "Ambiguity and polymorphism not yet supported in ALLELES block";
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
   }

   // Bail out here if data is haploid
   //
   if( IsHaploid(locus) )
   	return true;

   // Now read in the separator character, if one was supplied
   //
   if( gap != '\0' )
   {
      token.SetSpecialPunctuationCharacter( gap );
      token.SetLabileFlagBit( NexusToken::useSpecialPunctuation );
      token.SetLabileFlagBit( NexusToken::singleCharacterToken );
      if( interleaving )
         token.SetLabileFlagBit( NexusToken::newlineIsToken );
      token.GetNextToken();

      if( interleaving && token.AtEOL() ) {
   		errormsg = "Unexpected end of line encountered (within a genotype specification)";
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }

      if( token.Equals(",") ) {
         errormsg = "Unexpected comma encountered (within a genotype specification)";
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
   }

   // same song, second verse...
   //
   if( gap != '\0' ) {
      token.SetSpecialPunctuationCharacter( gap );
      token.SetLabileFlagBit( NexusToken::useSpecialPunctuation );
   }
	if( interleaving )
		token.SetLabileFlagBit( NexusToken::newlineIsToken );
	if( !tokens )
		token.SetLabileFlagBit( NexusToken::singleCharacterToken );
   token.SetLabileFlagBit( NexusToken::parentheticalToken );
   token.SetLabileFlagBit( NexusToken::curlyBracketedToken );
	token.GetNextToken();

	if( interleaving && token.AtEOL() ) {
		errormsg = "Unexpected end of line encountered (reading second half of genotype specification)";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

	if( token.Equals(",") ) {
		errormsg = "Unexpected comma encountered (reading second half of genotype specification)";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
   }

	// Allele should not be a punctuation character
	//
	if( token.IsPunctuationToken() ) {
		errormsg = "Punctuation character (";
		errormsg += token.GetToken();
		errormsg += ") found where allele name expected";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	// Make sure we didn't run out of file
	//
	if( token.AtEOF() ) {
		errormsg = "Unexpected end of file encountered";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	// If we didn't run out of file, I see no reason why we should
	// have a zero-length token on our hands
	assert( token.GetTokenLength() > 0 );

	// We've read in the allele now, so if this locus has been
	// ELIMINATEd, we want to skip the part that actually saves it
	//
	if( k >= 0 )
   {
      // see if any equate macros apply
      //
      nxsstring skey = nxsstring( token.GetToken( respectingCase ) );
      AssocList::iterator p = equates.find( skey );
      if( p != equates.end() ) {
         nxsstring sval = (*p).second;
         token.ReplaceToken( sval.c_str() );
      }

      // token should be in one of the following forms: "{"  "a"  "bb"
      //
      int polymorphism = token.Equals("(");
      int uncertainty  = token.Equals("{");

      if( !uncertainty && !polymorphism )
      {
         y = HandleAllele( token, k );
         if( y > MAX_ALLELES ) {
            errormsg = "Number of alleles for locus ";
            errormsg += (k+1);
            errormsg += " has exceeded limit of ";
            errormsg += MAX_ALLELES;
            throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
			int value = SplitInt( x, y );
         matrix->SetState( i, k, value );
      }
      else
      {
         // borrow material from CharactersBlock::HandleNextState if and when
         // ambiguity and polymorphism are implemented for the ALLELES block
         //
         errormsg = "Ambiguity and polymorphism not yet supported in ALLELES block";
         throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
      }
   }

	return true;
}

/**
 * @method HandleStdMatrix [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called from HandleMatrix function to read in a standard
 * (i.e., non-transposed) matrix.  Interleaving, if applicable,
 * is dealt with herein.
 */
void AllelesBlock::HandleStdMatrix( NexusToken& token )
{
	int i, j, currChar;
	int firstChar = 0;
	int lastChar = ncharTotal;
	int lastCharInSet = ncharTotal;
	int nextFirst;
	int page = 0;
   int lastIndiv = 0;
   int rowsInMatrix = ntax;
   int rowsToAdd = 25;
   bool comma = false;
   bool colon = false;
   bool semicolon = false;
   bool skipLocusLoop = false;

	for(;;)
	{
      j = 0;
      
		//*******************************************************
		//******** Beginning of loop through populations ********
		//*******************************************************

		for( i = 0; i < ntax; i++ )
		{
         // This should be the population label
         //
         token.GetNextToken();
         
         errormsg = "    Population ";
         errormsg += token.GetToken();
         nexus->OutputComment( errormsg );
         
#if 1
			if( page == 0 && newtaxa )
			{
				// This section:
				// - on first (or only) interleave page
				// - no previous TAXA block
				
				// check for duplicate taxon names
				//
				if( taxa.IsAlreadyDefined( token.GetToken() ) ) {
					errormsg = "Data for this population (";
					errormsg += token.GetToken();
					errormsg += ") has already been saved";
					throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
				}

				// Labels provided and not already stored in the taxa block with
				// the TAXLABELS command; taxa.Reset() and taxa.SetNTax() 
				// were already called, however, when the NTAX subcommand was
				// processed.
				//
				taxa.AddTaxonLabel( token.GetToken() );

				// order of occurrence in TAXA block same as row in matrix
				//
				taxonPos[i] = i;
			}
			else
			{
				// This section:
				// - TAXA block provided or has been created already
				// - may be on any (interleave) page					
				
				// Cannot assume taxon in same position in
				// taxa block. Set up taxonPos array so that we can look up
				// the correct row in matrix for any given taxon
				//
				int positionInTaxaBlock;
				try {
					positionInTaxaBlock = taxa.FindTaxon( token.GetToken() );
				}
				catch( out_of_range ) {
					errormsg = "Could not find population named ";
					errormsg += token.GetToken();
					errormsg += " among stored population labels";
					throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
				}

				if( page == 0 )
				{
					// make sure user has not duplicated data for a single taxon
					//
					if( taxonPos[positionInTaxaBlock] != -1 ) {
						errormsg = "Data for this population (";
						errormsg += token.GetToken();
						errormsg += ") has already been saved";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					// make sure user has kept same relative ordering of taxa in both the TAXA
					// block and the CHARACTERS block
					//
					if( positionInTaxaBlock >= i ) {
						errormsg = "Relative order of population must be the same in both the TAXA and CHARACTERS blocks";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}

					taxonPos[i] = positionInTaxaBlock;  // was --> taxonPos[positionInTaxaBlock] = i;
				}
				else
				{
					// make sure user has kept the ordering of taxa the same from
					// one interleave page to the next
					//
					if( taxonPos[positionInTaxaBlock] != i ) {
						errormsg = "Ordering of population must be identical to that in first interleave page";
						throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
					}
				}
			}
#else
         // If no TAXA block previously specified (i.e., newtaxa = true)
         // and we're on page 0, need to add population names as we
         // encounter them to the TAXA block
         //
         if( page == 0 && newtaxa )
         {
            // check for duplicate population names
            //
            if( taxa.IsAlreadyDefined( token.GetToken() ) ) {
               errormsg = "Data for this population (";
               errormsg += token.GetToken();
               errormsg += ") has already been saved";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }

            // population labels not already stored in the taxa block with
            // the TAXLABELS command; taxa.Reset() and taxa.SetNTax()
            // were already called, however, when the NTAX subcommand was
            // processed.
            //
            taxa.AddTaxonLabel( token.GetToken() );

				// order of occurrence in TAXA block same as row in matrix
				//
				taxonPos[i] = i;
         }

         // we go here if either (1) a TAXA block was previously specified
         // or (2) we have already built up a TAXA block because we are
         // beyond the first page of an interleaved file.  In either of
         // these cases, we must check to see whether the populations
         // are encountered in the same order as specified in the TAXA block.
         else
         {
            int positionInTaxaBlock;
            try {
               positionInTaxaBlock = taxa.FindTaxon( token.GetToken() );
            }
            catch( out_of_range ) {
               errormsg = "Could not find taxon named ";
               errormsg += token.GetToken();
               errormsg += " among stored taxon labels";
               throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
            }

            if( positionInTaxaBlock != i )
            {
               if( page == 0 ) {
                  // user has either duplicated data for a single taxon
                  // or the order of taxa is not the same as in the
                  // predefined TAXA block
                  errormsg = "This taxon (";
                  errormsg += token.GetToken();
                  errormsg += ") either (1) has already been saved";
                  errormsg += "\nor (2) is out of order";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }
               else {
                  // user has not kept the ordering of taxa the same from
                  // one interleave page to the next
                  if( positionInTaxaBlock != i ) {
                     errormsg = "Ordering of taxa must be identical to that in first interleave page";
                     throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  }
               }
            }
            
            taxonPos[i] = positionInTaxaBlock; // was --> taxonPos[positionInTaxaBlock] = i;
         }
#endif

         // read in the colon following the population label
         token.GetNextToken();
         if( !token.Equals(":") ) {
				errormsg = "Expecting ':' after population label but found ";
            errormsg += token.GetToken();
            errormsg += " instead";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
         
         //*******************************************************
         //******** Beginning of loop through individuals ********
         //*******************************************************
         for(;;)
         {
            // Read in individual label if necessary
            // These are not stored, so all we need to do is call
            // NexusToken::GetNextToken
            //
            if( labels ) 
            {

               token.GetNextToken();
               
               if( token.Equals(":") ) {
               	colon = true;
               	skipLocusLoop = true;
               }
               else if( token.Equals(",") ) {
               	comma = true;
               	break;
               }
               else if( token.Equals(";") ) {
               	semicolon = true;
               	break;
               }
            }

            // Increase number of rows in data matrix, if necessary
            //
            assert( lastIndiv <= rowsInMatrix );
            if( page == 0 && lastIndiv == rowsInMatrix ) {
					matrix->AddRows( rowsToAdd );
               rowsInMatrix += rowsToAdd;
            }

            //************************************************
            //******** Beginning of loop through loci ********
            //************************************************
            for( currChar = firstChar; currChar < lastChar; currChar++ )
            {
            	if( skipLocusLoop )
               	break;

               // HandleNextState will return false only if a newline OR comma
               // character is encountered before locus k is processed.
               // If a comma is encountered, it means it is time to break from
               // the loop over individuals; if a newline is encountered, we
               // either ignore it (if not interleave format) or, if reading
               // an interleaved matrix, break out of the loop over loci by
               // setting lastChar equal to currChar.
               // The base class version of HandleNextState is overridden
               // in AllelesBlock to read in and store and entire genotype
               // (e.g., "slow/fast")
               //
               bool stopOnNewline = ( currChar > firstChar );
               bool ok = HandleNextGenotype( token, j, currChar, stopOnNewline );
               
               comma = token.Equals(",");
               semicolon = token.Equals(";");
               colon = token.Equals(":");
               if( !ok && interleaving )
               {
                  if( !comma && !colon && lastCharInSet < ncharTotal && charPos[currChar] != lastCharInSet ) {
                     errormsg = "Each line within an interleave page must comprise the same number of loci";
                     throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  }

                  // currChar should be firstChar in next go around
                  //
                  nextFirst = currChar;

                  // set lastChar to currChar so that we can check to make sure the
                  // remaining lines in this interleave page end at the same place
                  //
                  lastCharInSet = currChar;

                  break;
               }
               else if( !ok && ( comma || colon || semicolon ) )
                  break;
            } // innermost loop (over loci)

				skipLocusLoop = false;

            // Handle here the case of a repeat count.
            // Note: if interleaving, colon will have already been eaten
            // and the boolean variable "color" will reflect this.  If
            // not interleaving, colon will not be seen until we begin
            // reading the data for the next individual (thus, decrement
            // lastIndiv before proceeding since it has already been
            // incremented.
            //
            int count = 1;
            if( ( colon || comma || semicolon ) && !interleaving ) {
               j--;
               if( page == 0 )
                  lastIndiv--;
            }
            if( colon )
            {
               // Get the repeat count itself
               //
               token.SetLabileFlagBit( NexusToken::newlineIsToken );
               token.GetNextToken();
               count = atoi( token.GetToken().c_str() );
               if( count < 1 ) {
                  errormsg = "Could not convert specified repeat count ";
                  if( token.GetTokenLength() > 0 ) {
                     errormsg += "(";
                     errormsg += token.GetToken();
                     errormsg += ")";
                  }
                  errormsg +=" to a positive integer";
                  throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
               }

               // Instruct matrix to duplicate last row count times.
               // Matrix itself handles the allocation of additional
               // rows (if necessary) to accommodate the duplication
               //
               int extraRows;
               if( interleaving )
                  extraRows = matrix->DuplicateRow( j, count, firstChar, lastCharInSet-1 );
               else
                  extraRows = matrix->DuplicateRow( j, count );
               if( extraRows > 0 ) {
                  if( page == 0 )
                     rowsInMatrix += extraRows;
                  else {
                     errormsg = "Repeat counts specify more individuals in later interleave pages than in first";
                     throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
                  }
               }

            }

            j += count;
            if( page == 0 )
            	lastIndiv += count;


            if( comma || semicolon )
               break;

         } // inner loop over individuals

         // Update rowsToAdd to more accurately reflect the number of
         // rows needing to be added for each new population.  For example,
         // if population samples are on the order of 200 individuals,
         // it is more efficient to add 200 new rows each time matrix is
         // resized rather than the default of 25.
         //
         if( page == 0 ) {
            rowsToAdd = (lastIndiv+1) / (i+1);
            if( rowsToAdd < 25 )
               rowsToAdd = 25;
         }

         if( page == 0 ) {
            // indivCount holds the _first_ row in matrix where the
            // data for individuals in the _next_ population (i.e.,
            // population i+1) are kept.
            //
            indivCount[i] = lastIndiv;
         }

		} // middle loop (over populations)

		// if semicolon is true, then we've just finished reading the last
		// interleave page and thus should break from the outer loop
		//
		if( semicolon )
			break;

		firstChar = nextFirst;
		lastChar = ncharTotal;
		lastCharInSet = ncharTotal;

		page++;

	} // outer loop (over interleave pages)
}

/**
 * @method HandleTransposedMatrix [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * Called from HandleMatrix function to read in a transposed
 * matrix.  Interleaving, if applicable, is dealt with herein.
 */
void AllelesBlock::HandleTransposedMatrix( NexusToken& /* token */ )
{
	// obviously, not yet written
}

/**
 * @method IsHaploid [bool:public]
 * @param i [int] the locus in question (in range [0..nchar))
 *
 * Returns true if character indexed by i was declared to be
 * haploid, false otherwise.  Returns false immediately if "haploid"
 * list is empty.
 */
bool AllelesBlock::IsHaploid( int i )
{
	if( haploid.empty() )
		return false;

	int origLocusIndex =GetOrigCharIndex(i);
	IntSet::const_iterator found = haploid.find(origLocusIndex);
   if( found == haploid.end() )
      return false;

	return true;
}

/**
 * @method IsHaploidOrig [bool:public]
 * @param origLocusIndex [int] the locus in question (in range [0..ncharTotal))
 *
 * Returns true if character number origLocusIndex was declared to be
 * haploid, false otherwise.  Returns false immediately if "haploid"
 * list is empty.
 */
bool AllelesBlock::IsHaploidOrig( int origLocusIndex )
{
	if( haploid.empty() )
		return false;

	IntSet::const_iterator found = haploid.find(origLocusIndex);
   if( found == haploid.end() )
      return false;

	return true;
}

/**
 * @method MostCommonAllele [int:public]
 * @param locus [int] the locus in question in range [0..nchar)
 * @throws XAllMissingData
 *
 * Returns internal representation of allele that is most
 * common (across all active populations) for the specified locus.
 * Assumes locus has not been excluded.  If no population is
 * supplied, considers all active populations.  If population
 * is supplied, the allele returned will be the one most common
 * for that population only.  Assumes that if population is 
 * specified, that population is not one currently deleted by
 * the user.  Throws XAllMissingData exception if only missing
 * data are encountered.
 *
 * Note: this is a relatively slow function because an allele count 
 * is performed for each call.
 */
int AllelesBlock::MostCommonAllele( int locus, int pop /* = -1 */ )
{
	assert( locus >= 0 && locus < nchar );
	assert( pop > -2 && pop < ntax );
	assert( !IsExcluded(locus) );

	// Make sure calling function doesn't expect us to find the
	// MostCommonAllele for a specific population that has been
	// deleted by the user.
	//
	bool do_one_pop = ( pop >= 0 );
	assert( !( do_one_pop && IsDeleted(pop) ) );

	int numAlleles = matrix->GetObsNumStates(locus);
	int counts[MAX_ALLELES];
	
	int i, j, gene;
	for( i = 0; i < MAX_ALLELES; i++ )
		counts[i] = 0;

	// Fill the counts array; counts[j] will hold the number of genes
	// corresponding to allele j at the focal locus over all active
	// populations (or just in population pop if pop is not -1)
	//
	i = ( do_one_pop ? pop : 0 );
	for( ; i < ntax; i++ )
	{
		if( do_one_pop && i > pop )
			break;
		if( IsDeleted(i) ) continue;
		int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
		for( j = 0; j < numIndivs; j++ )
		{
			gene = GetGene( i, j, locus, 0 );
			assert( gene == MAX_ALLELES || gene < numAlleles );
			if( gene < MAX_ALLELES )
				counts[gene]++;

			if( !IsHaploid(locus) )
			{
				gene = GetGene( i, j, locus, 1 );
				assert( gene == MAX_ALLELES || gene < numAlleles );
				if( gene < MAX_ALLELES )
					counts[gene]++;
			}
		}
	}

	int max = 0;
	int which = 0;
	bool bad = true;

	for( i = 0; i < numAlleles; i++ ) 
	{
		if( counts[i] > 0 )
			bad = false;

		if( counts[i] <= max ) continue;

		which = i;
		max = counts[i];
	}
	
	if(bad)
		throw XAllMissingData();

	return which;
}

/**
 * @method NumberOfAlleles [int:public]
 * @param locus [int] the locus in question in range [0..ntax)
 *
 * Returns the number of alleles observed for locus. If no population is
 * supplied, sums across all active populations.  If population
 * is supplied, count will only be for that population.
 * Assumes locus has not been excluded.  Assumes that if population is 
 * specified, that population is not one currently deleted by
 * the user.
 *
 * Note: this is a relatively slow function because an enumeration 
 * is performed for each call.
 */
int AllelesBlock::NumberOfAlleles( int locus, int pop /* = -1 */ )
{
	assert( locus >= 0 && locus < nchar );
	assert( pop > -2 && pop < ntax );
	assert( !IsExcluded(locus) );

	bool do_one_pop = ( pop >= 0 );
	assert( !( do_one_pop && IsDeleted(pop) ) );

	int i, j, gene;
	
	// A std::set is used here to simplify tallying the number of 
	// distinct alleles found (i.e., no matter how many times we
	// insert allele 1 into the set, it will end up being represented
	// by only one entry in the set at the end).
	//
	std::set<int> alleleSet;

	i = ( do_one_pop ? pop : 0 );
	for( ; i < ntax; i++ )
	{
		if( do_one_pop && i > pop )
			break;
		if( IsDeleted(i) ) continue;

		int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
		for( j = 0; j < numIndivs; j++ )
		{
			gene = GetGene( i, j, locus, 0 );
			if( gene < MAX_ALLELES ) 
				alleleSet.insert(gene);
		
			if( !IsHaploid(locus) )
			{
				gene = GetGene( i, j, locus, 1 );
				if( gene < MAX_ALLELES )
					alleleSet.insert(gene);
			}
		}
	}

	return alleleSet.size();

	// old way
	// LabelListBag::const_iterator i = charStates.find(locus);
	// return (*i).second.size();
}

/**
 * @method Read [void:protected]
 * @param token [NexusToken&] the token used to read from in
 * @throws XNexus
 *
 * This function provides the ability to read everything following
 * the block name (which is read by the Nexus object) to the end or
 * endblock statement. Characters are read from the input stream
 * in. Overrides the pure virtual function in the base class.
 */
void AllelesBlock::Read( NexusToken& token )
{
   isEmpty = false;

   // this should be the semicolon after the block name
   //
	token.GetNextToken();
	if( !token.Equals(";") ) {
		errormsg = "Expecting ';' after ";
      errormsg += id;
      errormsg += " block name, but found ";
      errormsg += token.GetToken();
      errormsg += " instead";
		throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
	}

	ntax = taxa.GetNumTaxonLabels();

	for(;;)
	{
		token.GetNextToken();

		if( token.Equals("DIMENSIONS") ) {
			HandleDimensions( token, "NEWPOPS", "NPOPS", "NLOCI" );
		}
		else if( token.Equals("FORMAT") ) {
			HandleFormat( token );
		}
		else if( token.Equals("ELIMINATE") ) {
			HandleEliminate( token );
		}
		else if( token.Equals("HAPLOID") ) {
			HandleHaploid( token );
		}
		else if( token.Equals("TAXLABELS") ) {
			HandleTaxlabels( token );
		}
		else if( token.Equals("LOCUSALLELELABELS") ) {
			HandleCharstatelabels( token );
		}
		else if( token.Equals("LOCUSLABELS") ) {
			HandleCharlabels( token );
		}
		else if( token.Equals("ALLELELABELS") ) {
			HandleStatelabels( token );
		}
		else if( token.Equals("MATRIX") ) {
			HandleMatrix( token );
		}
		else if( token.Equals("END") ) {
			HandleEndblock( token, "Locus" );
			break;
		}
		else if( token.Equals("ENDBLOCK") ) {
			HandleEndblock( token, "Locus" );
			break;
		}
		else
      {
			SkippingCommand( token.GetToken() );
         do {
            token.GetNextToken();
         } while( !token.AtEOF() && !token.Equals(";") );

         if( token.AtEOF() ) {
				errormsg = "Unexpected end of file encountered";
				throw XNexus( errormsg, token.GetFilePosition(), token.GetFileLine(), token.GetFileColumn() );
         }
		}
   }
}

/**
 * @method Report [void:public]
 * @param out [ostream&] the output stream to which to write the report
 *
 * This function outputs a brief report of the contents of this ALLELES block.
 * Overrides the pure virtual function in the base class.
 */
void AllelesBlock::Report( ostream& out )
{
	int i;

	out << endl;
	out << id << " block contains data for ";
   if( ntax == 1 )
	   out << " 1 population and ";
   else
	   out << ntax << " populations ";
   out << "(" << indivCount[ntax-1] << " total individuals)";
   if( nchar == 1 )
	   out << " and 1 locus";
   else
   	out << " and " << nchar << " loci";
   out << endl;
   
   out.flush();

   out << "  Datapoint: ";
   switch( datapoint ) {
      case fraglen:
         out << "fraglen" << endl;
         break;
      default:
         out << "standard" << endl;
   }

   out.flush();

	if( transposing && interleaving )
		out << "  Matrix transposed and interleaved" << endl;
	else if( transposing && !interleaving )
		out << "  Matrix transposed but not interleaved" << endl;
	else if( !transposing && interleaving )
		out << "  Matrix interleaved but not transposed" << endl;
	else
		out << "  Matrix neither transposed nor interleaved" << endl;

	if( tokens )
		out << "  Multicharacter allele names allowed in data matrix" << endl;
	else
		out << "  Allele names are expected to be single character symbols" << endl;

   if( labels )
      out << "  Labels for individuals provided" << endl;
   else
      out << "  Labels for individuals not provided" << endl;

   if( respectingCase )
      out << "  Allele labels in matrix case-sensitive" << endl;
   else
      out << "  Allele labels in matrix not case-sensitive" << endl;

   if( newtaxa )
      out << "  Population labels defined in matrix" << endl;
   else
      out << "  Population labels defined in TAXA block" << endl;

   out << "  Missing data symbol is " << missing << endl;

   out.flush();

   if( haploid.empty() )
      out << "  All loci are diploid" << endl;
   else if( haploid.size() == 1 ) {
      out << "  The following locus is haploid:" << endl;
      IntSet::const_iterator k = haploid.begin();
      out << "    " << ( (*k) + 1 ) << endl;
   }
   else {
      out << "  The following loci are haploid:" << endl;
      out << "    ";
      IntSet::const_iterator k;
      for( k = haploid.begin(); k != haploid.end(); k++ )
         out << ( (*k) + 1 ) << " ";
      out << endl;
   }

   out.flush();

   if( gap == ' ' )
      out << "  No separator character defined" << endl;
   else
      out << "  Separator character is " << gap << endl;

   out.flush();

	int numEquateMacros = equates.size();
	if( numEquateMacros > 0 ) {
		out << "  Equate macros in effect:" << endl;
      typedef AssocList::const_iterator CI;
		for( CI i = equates.begin(); i != equates.end(); ++i) {
			out << "    " << (*i).first << " = " << (*i).second << endl;
		}
	}
	else
		out << "  No equate macros have been defined" << endl;

   out.flush();

	out << "  Contents of the charLabels LabelList:" << endl;
   LabelList::const_iterator lli = charLabels.begin();
   while( lli != charLabels.end() )
   {
   	out << "\t" << (*lli) << endl;
  		++lli;
   }
   out.flush();
   
	out << "  Contents of the charStates LabelListBag:" << endl;
   LabelListBag::const_iterator cib = charStates.begin();
   while( cib != charStates.end() )
   {
   	out << "\t" << (*cib).first << ": ";
   	LabelList::const_iterator lli = (*cib).second.begin();
   	while( lli != (*cib).second.end() ) {
   		out << (*lli) << " ";
   		++lli;
   	}
   	out << endl;
      ++cib;
   }
   out.flush();
   
	if( charLabels.size() > 0 )
	{
		out << "  Locus and allele labels:" << endl;
		for( int k = 0; k < nchar; k++ ) {
			if( charLabels[k].size() == 0 )
				out << '\t' << ( 1 + GetOrigCharIndex(k) ) << '\t' << "(no label provided for this locus)" << endl;
			else
				out << '\t' << ( 1 + GetOrigCharIndex(k) ) << '\t' << charLabels[k] << endl;

			// output allele names if any are defined for this locus
         LabelListBag::const_iterator cib = charStates.find(k);
         if( cib != charStates.end() )
         {
            int ns = (*cib).second.size();
            for( int m = 0; m < ns; m++ ) {
         		out << "\t\t" << (*cib).second[m] << endl;
            }
         }
		}
	}

   out.flush();

	if( ncharTotal == nchar )
		out << "  No loci were eliminated" << endl;
	else {
		out << "  The following loci were eliminated:" << endl;
		IntSet::const_iterator list_iter;
		for( list_iter = eliminated.begin(); list_iter != eliminated.end(); list_iter++ ) {
			out << "    " << ((*list_iter)+1) << endl;
		}
	}

   out.flush();

	out << "  The following loci have been excluded:" << endl;
	int k;
	int nx = 0;
	for( k = 0; k < nchar; k++ ) {
		if( activeChar[k] ) continue;
		out << "    " << (k+1) << endl;
		nx++;
	}
	if( nx == 0 )
		out << "    (no loci excluded)" << endl;

   out.flush();

	out << "  The following populations have been deleted:" << endl;
	nx = 0;
	for( k = 0; k < ntax; k++ ) {
		if( activeTaxon[k] ) continue;
		out << "    " << (k+1) << endl;
		nx++;
	}
	if( nx == 0 )
		out << "    (no populations deleted)" << endl;

   out.flush();

   if( alleles_fixed )
   	out << "  Only alleles specified in ALLELELABELS command will be considered valid." << endl;
   else
   	out << "  All alleles encountered in matrix will be considered valid." << endl;

	out << "  Data matrix:" << endl;
	DebugShowMatrix( out, "    " );

   out.flush();

   out << endl;
   out << "Most common allele for each locus:" << endl;
   out << setw(20) << " ";
   out << setw(20) << "number of";
   out << setw(20) << "dominant";
   out << setw(20) << " ";
   out << endl;
   out << setw(20) << "locus";
   out << setw(20) << "alleles";
   out << setw(20) << "allele";
   out << setw(20) << "frequency";
   out << endl;
   out.setf( ios::fixed, ios::floatfield );
   out.setf( ios::showpoint );
	nxsstring s;
   for( i = 0; i < nchar; i++ ) {
		if( IsExcluded(i) ) continue;
		
		s = "";
		s += i;
		s += " (";
		s += charLabels[i];
		s += ")";
      out << setw(20) << s;

      int k = NumberOfAlleles(i);
      out << setw(20) << k;

      int j = MostCommonAllele(i);
		s = "";
		s += j;
      LabelListBag::const_iterator cib = charStates.find(i);
      if( cib != charStates.end() )
      {
			s += " (";
			s += (*cib).second[j];
			s += ")";
      }
      out << setw(20) << s;

      double frq = AlleleFrequency( j, i );
	   out << setw(20) << setprecision(6) << frq;
      out << endl;
   }
}

/**
 * @method Reset [void:protected]
 *
 * Sets npops and nloci to 0 in preparation for reading a new ALLELES block.
 * Overrides the pure virtual function in the base class.  Also performs
 * the initializations done in the constructor, as well as erasing the
 * vector haploid and freeing memory allocated previously for the indivCount
 * array.
 */
void AllelesBlock::Reset()
{
	CharactersBlock::Reset();
	
	// Thinking of changing any of these defaults?
	// If so, do it also in the constructor
	//
	alleles_fixed  = false;
	datapoint      = standard;
	gap            = '/';
	labels         = false;
	respectingCase = true;
	tokens         = true;
	
	haploid.erase( haploid.begin(), haploid.end() );
	
	if( indivCount != NULL ) {
		delete [] indivCount;
		indivCount = NULL;
	}
}

/**
 * @method SampleSize [int:public]
 * @param locus [int] the locus in question, in range (0..nchar]
 * @param pop [int] the population in question, in range (0..ntax] (default is -1)
 *
 * Returns the number of individuals (if data is diploid)
 * for which both genes are non-missing.  If data is haploid,
 * returns number of genes that are non-missing.
 * Assumes locus has not been excluded.  If no population is
 * supplied, sums across all active populations.  If
 * population is supplied, count will only be for that 
 * population. It is assumed that if a population is specified,
 * that population is not one currently deleted by the user.
 */
int AllelesBlock::SampleSize( int locus, int pop /* = -1 */ )
{
	assert( !IsExcluded(locus) );
	bool locus_haploid = IsHaploid(locus);

	bool do_one_pop = ( pop >= 0 );
	assert( !( do_one_pop && IsDeleted(pop) ) );

	int i, j, gene0, gene1;
	int total_genes = 0;
	int total_indivs = 0;

	i = ( do_one_pop ? pop : 0 );
	for( ; i < ntax; i++ )
	{
		if( do_one_pop && i > pop )
			break;
		if( IsDeleted(pop) ) continue;

		int numIndivs = ( i > 0 ? indivCount[i] - indivCount[i-1] : indivCount[i] );
		for( j = 0; j < numIndivs; j++ )
		{
			gene0 = GetGene( i, j, locus, 0 );
			if( gene0 < MAX_ALLELES )
				total_genes++;
			
			if( !locus_haploid )
			{
				gene1 = GetGene( i, j, locus, 1 );
				if( gene1 < MAX_ALLELES ) 
				{
					total_genes++;
					if( gene0 < MAX_ALLELES )
						total_indivs++;
				}
			}
		}
	}
	
	return ( locus_haploid ? total_genes : total_indivs );
}

/**
 * @method SplitInt [int:protected]
 * @param x [int] to be placed in the low-order word
 * @param y [int] to be placed in the high-order word
 *
 * Places x in the low-order word (least-significant 2 bytes)
 * and y in the high-order word (most-significant 2 bytes)
 * of the 4-byte int returned.  Note that because x and y
 * both have to fit in 2 bytes instead of 4, the maximum
 * value for each is 255 = 0xff.  The value 255 itself is
 * specifically reserved for use as the missing data value.
 */
int AllelesBlock::SplitInt( int x, int y )
{
   //typedef long LONG;
   //typedef unsigned long       DWORD;
   //typedef unsigned short      WORD;
   //#define MAKELONG(a, b)      ((LONG)( ((WORD)(a)) | ( ((DWORD)((WORD)(b))) << 16) ))
	assert( x >= 0 && x <= MAX_ALLELES );
	assert( y >= 0 && y <= MAX_ALLELES );
   int z_low  = (unsigned short)x;
   int z_high = ( (unsigned int)( (unsigned short)y ) ) << 16;
   int z = ( z_low | z_high );
   return z;
}


