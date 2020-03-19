#ifndef SList_h
#define SList_h

/*
 * SList.h
 *
 * SOFTWARE RIGHTS
 *
 * We reserve no LEGAL rights to SORCERER -- SORCERER is in the public
 * domain.  An individual or company may do whatever they wish with
 * source code distributed with SORCERER or the code generated by
 * SORCERER, including the incorporation of SORCERER, or its output, into
 * commerical software.
 *
 * We encourage users to develop software with SORCERER.  However, we do
 * ask that credit is given to us for developing SORCERER.  By "credit",
 * we mean that if you incorporate our source code into one of your
 * programs (commercial product, research project, or otherwise) that you
 * acknowledge this fact somewhere in the documentation, research report,
 * etc...  If you like SORCERER and have developed a nice tool with the
 * output, please mention that you developed it using SORCERER.  In
 * addition, we ask that this header remain intact in our source code.
 * As long as these guidelines are kept, we expect to continue enhancing
 * this system and expect to make other tools available as they are
 * completed.
 *
 * PCCTS 1.33
 * Terence Parr
 * Parr Research Corporation
 * with Purdue University and AHPCRC, University of Minnesota
 * 1992-1998
 */

#include "pcctscfg.h"

#include <stdio.h>
#include <stdlib.h>

#include "PCCTSAST.h"

class SListErr{};

class PCCTS_AST;

class SListNode {
protected:
	void *_elem;			/* pointer to any kind of element */
	SListNode *_next;
public:
	SListNode()				{_elem=_next=NULL;}
	virtual ~SListNode()	{_elem=_next=NULL;}
	void *elem()			{ return _elem; }
	void setElem(void *e)	{ _elem = e; }
	void setNext(SListNode *t)	{ _next = t; }
	SListNode *next()		{ return _next; }
};

class SList {
	SListNode *head, *tail;
public:
	SList() {head=tail=NULL;}
	virtual ~SList() {head=tail=NULL;}
	virtual void *iterate(SListNode **);
	virtual void add(void *e);
	virtual void lfree();
	virtual PCCTS_AST *to_ast(SList list);
	virtual void require(int e,char *err){ if ( !e ) panic(err); }
	virtual void panic(char *err)
	{
	    throw SListErr();
	    
//Punch	     fprintf(stderr, "SList panic: %s\n", err);
//Punch	     exit(PCCTS_EXIT_FAILURE);
    }
};

#endif
