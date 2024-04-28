/*
 * This file was autogenerated by Header2Scheme on Wed Jan 10 01:12:17 1996
 * as part of Ivy version 1.4.1.
 * For more information, contact Kenneth B. Russell <kbrussel@media.mit.edu>.
 */

#ifndef _SCM_SONODE_
#define _SCM_SONODE_

#include "Ivy.h"

extern long tc32_SoNode;
extern ClassNode *SCM_SoNode_classnode;
// get the class definition from our header file
#include <Inventor/nodes/SoNode.h>

#define SCM2SONODE(x)  ((SoNode *) SCM2PTR(CDR(x)))
#define SONODEPE(x)  (CELLP(x) ? (TYP32(x)==tc32_SoNode) : 0)
#define SONODEP(x)  (CELLP(x) ? (classController->searchUpForClass(SCM_SoNode_classnode,TYP32(x))) : 0)
// Conversion functions from SoNode to SCM
SCM SCM_SoNode(const SoNode *);
SCM SCM_SoNode(const SoNode &);

// Cast procedure
SCM SCM_SoNode_Cast(SCM x ...);

// Predicate procedure
SCM SCM_SoNode_Predicate(SCM arglist ...);

// Required functions for data type
static int SCM_SoNode_Print(SCM, SCM, int);
SCM SCM_SoNode_EqualP(SCM, SCM);
SCM SCM_SoNode_Dispatch(char *method_name, SCM arglist);

// initializer function for registering this class with SCM
void init_SCM_SoNode();

SCM SCM_SoNode_setoverride(SCM arglist ...);
SCM SCM_SoNode_isoverride(SCM arglist ...);
SCM SCM_SoNode_getclasstypeid(SCM arglist ...);
SCM SCM_SoNode_copy(SCM arglist ...);
SCM SCM_SoNode_affectsstate(SCM arglist ...);
SCM SCM_SoNode_getbyname(SCM arglist ...);

#endif