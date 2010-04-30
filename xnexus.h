#ifndef __XNEXUS_H
#define __XNEXUS_H

//
// XNexus exception class
//
class XNexus
{
public:
	nxsstring msg;
	streampos pos;
	long line;
	long col;

	XNexus( nxsstring s, streampos fp = 0, long fl = 0L, long fc = 0L );
};

#endif
