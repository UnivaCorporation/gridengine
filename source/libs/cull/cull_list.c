/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* do not compile in monitoring code */
#ifndef NO_SGE_COMPILE_DEBUG
#define NO_SGE_COMPILE_DEBUG
#endif

#include "msg_cull.h"
#include <stdarg.h>

#include "sgermon.h"
#include "cull_sortP.h"
#include "cull_where.h"
#include "cull_listP.h"
#include "cull_multitypeP.h"
#include "cull_whatP.h"
#include "cull_lerrnoP.h"
#include "cull_hash.h"

#define CULL_BASIS_LAYER CULL_LAYER

static void lWriteList_(const lList *lp, int nesting_level, FILE *fp);
static void lWriteElem_(const lListElem *lp, FILE *fp);

/****** cull/list/lCopyElem() *************************************************
*  NAME
*     lCopyElem() -- Copies a whole list element 
*
*  SYNOPSIS
*     lListElem* lCopyElem(const lListElem *ep) 
*
*  FUNCTION
*     Copies a whole list element 
*
*  INPUTS
*     const lListElem *ep - element 
*
*  RESULT
*     lListElem* - copy of 'ep'
******************************************************************************/
lListElem *lCopyElem(const lListElem *ep) 
{
   lListElem *new;
   lDescr *p;

   DENTER(CULL_LAYER, "lCopyElem");

   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return NULL;
   }

   if (!(new = lCreateElem(ep->descr))) {
      LERROR(LECREATEELEM);
      DEXIT;
      return NULL;
   }

   /* for (i = 0; ep->descr[i].nm != NoName; i++)  */
   /*    if (lCopySwitch(ep, new, i, i) != 0)  */
      
   for (p = &(ep->descr[0]); p->nm != NoName; p++) {
      
      if (lCopySwitch(ep, new, p - &(ep->descr[0]), p - &(ep->descr[0])) != 0) {
         lFreeElem(new);
         LERROR(LECOPYSWITCH);
         DEXIT;
         return NULL;
      }
   }
   new->status = FREE_ELEM;

   DEXIT;
   return new;
}

/****** cull/list/lModifyWhat() ************************************************
*  NAME
*     lModifyWhat() -- Copy parts of an element 
*
*  SYNOPSIS
*     int lModifyWhat(lListElem *dst, const lListElem *src, 
*                     const lEnumeration *enp) 
*
*  FUNCTION
*     Copies elements from 'src' to 'dst' using the enumeration 'enp'
*     as a mask or copies all elements if 'enp' is NULL 
*
*  INPUTS
*     lListElem *dst          - destination element
*     const lListElem *src    - source element
*     const lEnumeration *enp - mask 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
******************************************************************************/
int lModifyWhat(lListElem *dst, const lListElem *src, const lEnumeration *enp) 
{
   int ret, i = 0;

   DENTER(CULL_LAYER, "lModifyWhat");

   ret = lCopyElemPartial(dst, &i, src, enp);

   DEXIT;
   return ret;
}

/****** cull/list/lCopyElemPartial() ****************************************
*  NAME
*     lCopyElemPartial() -- Copies parts of an element 
*
*  SYNOPSIS
*     int lCopyElemPartial(lListElem *dst, int *jp, 
*                          const lListElem *src, 
*                          const lEnumeration *enp) 
*
*  FUNCTION
*     Copies elements from list element 'src' to 'dst' using the
*     enumeration 'enp' as a mask or copies all elements if
*     'enp' is NULL. Copying starts at index *jp. 
*
*  INPUTS
*     lListElem *dst          - destination element 
*     int *jp                 - Where should the copy operation start 
*     const lListElem *src    - src element 
*     const lEnumeration *enp - enumeration 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*******************************************************************************/
int lCopyElemPartial(lListElem *dst, int *jp, const lListElem *src, 
                     const lEnumeration *enp) 
{
   int i;

   DENTER(CULL_LAYER, "lCopyElemPartial");

   if (!enp || !dst || !jp) {
      LERROR(LEENUMNULL);
      DEXIT;
      return -1;
   }

   switch (enp[0].pos) {
   case WHAT_ALL:               /* all fields of element src is copied */
      for (i = 0; src->descr[i].nm != NoName; i++, (*jp)++) {
         if (lCopySwitch(src, dst, i, *jp) != 0) {
            LERROR(LECOPYSWITCH);
            DEXIT;
            return -1;
         }
      }
      break;

   case WHAT_NONE:              /* no field of element src is copied */
      break;

   default:                     /* copy only the in enp enumerated elems */
      for (i = 0; enp[i].nm != NoName; i++, (*jp)++) {
         if (lCopySwitch(src, dst, enp[i].pos, *jp) != 0) {
            LERROR(LECOPYSWITCH);
            DEXIT;
            return -1;
         }
      }
   }
   DEXIT;
   return 0;
}

/****** cull/list/lCopySwitch() ***********************************************
*  NAME
*     lCopySwitch() -- Copy parts of elements indedendent from type 
*
*  SYNOPSIS
*     int lCopySwitch(const lListElem *sep, lListElem *dep, 
*                     int src_idx, int dst_idx) 
*
*  FUNCTION
*     Copies from the element 'sep' (using index 'src_idx') to
*     the element 'dep' (using index 'dst_idx') in dependence 
*     of the type. 
*
*  INPUTS
*     const lListElem *sep - source element 
*     lListElem *dep       - destination element 
*     int src_idx          - source index 
*     int dst_idx          - destination index 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*******************************************************************************/
int lCopySwitch(const lListElem *sep, lListElem *dep, 
                int src_idx, int dst_idx) 
{
   lList *tlp;

   DENTER(CULL_LAYER, "lCopySwitch");

   if (!dep || !sep) {
      DEXIT;
      return -1;
   }

   switch (dep->descr[dst_idx].mt) {
   case lUlongT:
      dep->cont[dst_idx].ul = sep->cont[src_idx].ul;
      /* lSetPosUlong(dep, dst_idx, lGetPosUlong(sep, src_idx)); */
      break;
   case lStringT:
      if (!sep->cont[src_idx].str)
         dep->cont[dst_idx].str = NULL;
      else
         dep->cont[dst_idx].str = strdup(sep->cont[src_idx].str);
      /* lSetPosString(dep, dst_idx, lGetPosString(sep, src_idx)); */
      break;
   case lHostT:
      if (!sep->cont[src_idx].host)
         dep->cont[dst_idx].host = NULL;
      else
         dep->cont[dst_idx].host = strdup(sep->cont[src_idx].host);
      /* lSetPosString(dep, dst_idx, lGetPosString(sep, src_idx)); */
      break;

   case lDoubleT:
      dep->cont[dst_idx].db = sep->cont[src_idx].db;
      /* lSetPosDouble(dep, dst_idx, lGetPosDouble(sep, src_idx)); */
      break;
   case lListT:
      /* if (!(tlp = lGetPosList(sep, src_idx))) 
       *    lSetPosList(dep, dst_idx, (lList *) NULL);
       */
      
      if (!(tlp = sep->cont[src_idx].glp)) 
         dep->cont[dst_idx].glp = NULL;
      else  
         dep->cont[dst_idx].glp = lCopyList(NULL, tlp);
      break;
   case lIntT:
      dep->cont[dst_idx].i = sep->cont[src_idx].i;            
      /* lSetPosUlong(dep, dst_idx, lGetPosUlong(sep, src_idx)); */
      break;
   case lFloatT:
      dep->cont[dst_idx].fl = sep->cont[src_idx].fl;
      /* lSetPosFloat(dep, dst_idx, lGetPosFloat(sep, src_idx)); */
      break;
   case lLongT:
      dep->cont[dst_idx].l = sep->cont[src_idx].l;
      /* lSetPosLong(dep, dst_idx, lGetPosLong(sep, src_idx)); */
      break;
   case lCharT:
      dep->cont[dst_idx].c = sep->cont[src_idx].c;
      /* lSetPosChar(dep, dst_idx, lGetPosChar(sep, src_idx)); */
      break;
   case lRefT:
      dep->cont[dst_idx].ref = sep->cont[src_idx].ref;
      /* lSetPosRef(dep, dst_idx, lGetPosChar(sep, src_idx)); */
      break;
   default:
      DEXIT;
      return -1;
   }

   DEXIT;
   return 0;
}

/****** cull/list/lGetListName() **********************************************
*  NAME
*     lGetListName() -- returns the user defined name of a list 
*
*  SYNOPSIS
*     const char* lGetListName(const lList *lp) 
*
*  FUNCTION
*     Returns the user defined name of a list. 
*
*  INPUTS
*     const lList *lp - list pointer 
*
*  RESULT
*     const char* - list name
******************************************************************************/
const char *lGetListName(const lList *lp) 
{
   DENTER(CULL_LAYER, "lGetListName");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return "No List specified";
   }

   if(!lp->listname) {
      LERROR(LENULLSTRING);
      DEXIT;
      return "No list name specified";
   }

   DEXIT;
   return lp->listname;
}

/****** cull/list/lGetListDescr() *********************************************
*  NAME
*     lGetListDescr() -- Returns the descriptor of a list 
*
*  SYNOPSIS
*     const lDescr* lGetListDescr(const lList *lp) 
*
*  FUNCTION
*     Returns the descriptor of a list 
*
*  INPUTS
*     const lList *lp - list pointer 
*
*  RESULT
*     const lDescr* - destriptor 
******************************************************************************/
const lDescr *lGetListDescr(const lList *lp) 
{
   DENTER(CULL_LAYER, "lGetListDescr");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return NULL;
   }
   DEXIT;
   return lp->descr;
}

/****** cull/list/lGetNumberOfElem() ******************************************
*  NAME
*     lGetNumberOfElem() -- Returns the number of elements in a list 
*
*  SYNOPSIS
*     int lGetNumberOfElem(const lList *lp) 
*
*  FUNCTION
*     Returns the number of elements in a list 
*
*  INPUTS
*     const lList *lp - list pointer 
*
*  RESULT
*     int - number of elements 
******************************************************************************/
int lGetNumberOfElem(const lList *lp) 
{
   DENTER(CULL_LAYER, "lGetNumberOfElem");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return 0;
   }

   DEXIT;
   return lp->nelem;
}

/****** cull/list/lGetElemIndex() *********************************************
*  NAME
*     lGetElemIndex() -- returns the index of element in list lp 
*
*  SYNOPSIS
*     int lGetElemIndex(const lListElem* ep, const lList* lp) 
*
*  FUNCTION
*     returns the index of element in list lp 
*
*  INPUTS
*     lListElem* ep - element 
*     lList* lp     - list 
*
*  RESULT
*     index number 
******************************************************************************/
int lGetElemIndex(const lListElem *ep, const lList *lp) 
{
   int i = -1;
   lListElem *ep2;

   DENTER(CULL_LAYER, "lGetElemIndex");

   if (!ep || ep->status != BOUND_ELEM) {
      DEXIT;
      return -1;
   }

   for_each(ep2, lp) {
      i++;
      if (ep2 == ep)
         break;
   }

   DEXIT;
   return i;
}

/****** cull/list/lGetNumberOfRemainingElem() *********************************
*  NAME
*     lGetNumberOfRemainingElem() -- Number of following elements 
*
*  SYNOPSIS
*     int lGetNumberOfRemainingElem(const lListElem *ep) 
*
*  FUNCTION
*     Returns the number of elements behind an element 
*
*  INPUTS
*     const lListElem *ep - element 
*
*  RESULT
*     int - number of elements 
******************************************************************************/
int lGetNumberOfRemainingElem(const lListElem *ep) 
{
   int i = 0;

   DENTER(CULL_LAYER, "lGetNumberOfElem");

   if (!ep) {
      DEXIT;
      return 0;
   }

   while ((ep = lNext(ep)))
      i++;

   DEXIT;
   return i;
}

/****** cull/list/lGetElemDescr() **********************************************
*  NAME
*     lGetElemDescr() -- returns the descriptor of a list element 
*
*  SYNOPSIS
*     const lDescr* lGetElemDescr(const lListElem* ep) 
*
*  FUNCTION
*     returns the descriptor of a list element 
*
*  INPUTS
*     lListElem* ep - CULL element 
*
*  RESULT
*     Pointer to descriptor 
*
******************************************************************************/
const lDescr *lGetElemDescr(const lListElem *ep) 
{
   DENTER(CULL_LAYER, "lGetListDescr");

   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return NULL;
   }
   DEXIT;
   return ep->descr;
}

/****** cull/list/lWriteElem() ************************************************
*  NAME
*     lWriteElem() -- Write a element to monitoring level CULL_LAYER 
*
*  SYNOPSIS
*     void lWriteElem(const lListElem *ep) 
*
*  FUNCTION
*     Write a element to monitoring level CULL_LAYER a info message 
*
*  INPUTS
*     const lListElem *ep - element 
******************************************************************************/
void lWriteElem(const lListElem *ep) 
{
   DENTER(CULL_LAYER, "lWriteElem");

   lWriteElem_(ep, NULL);

   DEXIT;
}

/****** cull/list/lWriteElemTo() **********************************************
*  NAME
*     lWriteElemTo() -- Write a element to file stream 
*
*  SYNOPSIS
*     void lWriteElemTo(const lListElem *ep, FILE *fp) 
*
*  FUNCTION
*     Write a element to file stream 
*
*  INPUTS
*     const lListElem *ep - element 
*     FILE *fp            - file stream 
*     ???/???
******************************************************************************/
void lWriteElemTo(const lListElem *ep, FILE *fp) 
{
   DENTER(CULL_LAYER, "lWriteElemTo");

   lWriteElem_(ep, fp);

   DEXIT;
}

static void lWriteElem_(const lListElem *ep, FILE *fp) 
{
   int i;
   static int nesting_level = 0;
   char space[128];
   lList *tlp;
   const char *str;

   DENTER(CULL_LAYER, "lWriteElem");

   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return;
   }

   for (i = 0; i < nesting_level * 3; i++)
      space[i] = ' ';
   space[i] = '\0';

   if (!fp)
      DPRINTF(("%s-------------------------------\n", space));
   else
      fprintf(fp, "%s-------------------------------\n", space);

   for (i = 0; ep->descr[i].mt != lEndT; i++) {
      switch (ep->descr[i].mt) {
      case lIntT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Integer) = %d\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosInt(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Integer) = %d\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosInt(ep, i));
         break;
      case lUlongT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Ulong)   = " u32"\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosUlong(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Ulong)   = " u32"\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosUlong(ep, i));
         break;
      case lStringT:
         str = lGetPosString(ep, i);
         if (!fp)
            DPRINTF(("%s%-20.20s (String)  = %s\n", space,
                     lNm2Str(ep->descr[i].nm), str ? str : "(null)"));
         else
            fprintf(fp, "%s%-20.20s (String)  = %s\n", space,
                    lNm2Str(ep->descr[i].nm), str ? str : "(null)");
         break;

      case lHostT:
         str = lGetPosHost(ep, i);
         if (!fp)
            DPRINTF(("%s%-20.20s (Host)  = %s\n", space,
                     lNm2Str(ep->descr[i].nm), str ? str : "(null)"));
         else
            fprintf(fp, "%s%-20.20s (Host)  = %s\n", space,
                    lNm2Str(ep->descr[i].nm), str ? str : "(null)");
         break;

      case lListT:
         tlp = lGetPosList(ep, i);
         if (!fp)
            DPRINTF(("%s%-20.20s (List)    = %s\n", space,
                     lNm2Str(ep->descr[i].nm), tlp ? "full {" : "empty"));
         else
            fprintf(fp, "%s%-20.20s (List)    = %s\n", space,
                    lNm2Str(ep->descr[i].nm), tlp ? "full {" : "empty");
         if (tlp) {
            nesting_level++;
            lWriteList_(tlp, nesting_level, fp);
            if (!fp)
               DPRINTF(("%s}\n", space));
            else
               fprintf(fp, "%s}\n", space);
            nesting_level--;
         }
         break;
      case lFloatT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Float)   = %f\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosFloat(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Float)   = %f\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosFloat(ep, i));
         break;
      case lDoubleT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Double)  = %f\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosDouble(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Double)  = %f\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosDouble(ep, i));
         break;
      case lLongT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Long)    = %ld\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosLong(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Long)    = %ld\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosLong(ep, i));
         break;
      case lCharT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Char)    = %c\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosChar(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Char)    = %c\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosChar(ep, i));
         break;
      case lRefT:
         if (!fp)
            DPRINTF(("%s%-20.20s (Ref)    = %p\n", space,
                     lNm2Str(ep->descr[i].nm), lGetPosRef(ep, i)));
         else
            fprintf(fp, "%s%-20.20s (Ref)    = %p\n", space,
                    lNm2Str(ep->descr[i].nm), lGetPosRef(ep, i));
         break;
      default:
         unknownType("lWriteElem");
      }
   }

   DEXIT;
   return;
}

/****** cull/list/lWriteList() ************************************************
*  NAME
*     lWriteList() -- Write a list to monitoring level CULL_LAYER 
*
*  SYNOPSIS
*     void lWriteList(const lList *lp) 
*
*  FUNCTION
*     Write a list to monitoring level CULL_LAYER as info message. 
*
*  INPUTS
*     const lList *lp - list 
******************************************************************************/
void lWriteList(const lList *lp) 
{
   DENTER(CULL_LAYER, "lWriteList");

   lWriteList_(lp, 0, NULL);

   DEXIT;
   return;
}

/****** cull/list/lWriteListTo() **********************************************
*  NAME
*     lWriteListTo() -- Write a list to a file stream 
*
*  SYNOPSIS
*     void lWriteListTo(const lList *lp, FILE *fp) 
*
*  FUNCTION
*     Write a list to a file stream 
*
*  INPUTS
*     const lList *lp - list 
*     FILE *fp        - file stream 
*******************************************************************************/
void lWriteListTo(const lList *lp, FILE *fp) 
{
   DENTER(CULL_LAYER, "lWriteListTo");

   lWriteList_(lp, 0, fp);

   DEXIT;
   return;
}

static void lWriteList_(const lList *lp, int nesting_level, FILE *fp) 
{
   lListElem *ep;
   char indent[128];
   int i;

   DENTER(CULL_LAYER, "lWriteList_");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return;
   }

   for (i = 0; i < nesting_level * 3; i++)
      indent[i] = ' ';
   indent[i] = '\0';

   if (!fp) {
      DPRINTF(("\n%sList: <%s> #Elements: %d\n",
               indent,
               lGetListName(lp),
               lGetNumberOfElem(lp)));
   }
   else {
      fprintf(fp, MSG_CULL_LISTXYNROFELEMENTSZ_SSI ,
              indent,
              lGetListName(lp),
              lGetNumberOfElem(lp));
   }

   for_each(ep, lp) {
      if (!fp)
         lWriteElem(ep);
      else
         lWriteElemTo(ep, fp);
   }
   DEXIT;
}

/****** cull/list/lCreateElem() ***********************************************
*  NAME
*     lCreateElem() -- Create an element for a specific list 
*
*  SYNOPSIS
*     lListElem* lCreateElem(const lDescr *dp) 
*
*  FUNCTION
*     Create an element for a specific list 
*
*  INPUTS
*     const lDescr *dp - descriptor 
*
*  RESULT
*     lListElem* - element pointer or NULL
******************************************************************************/
lListElem *lCreateElem(const lDescr *dp) 
{
   int n, i;
   lListElem *ep;

   DENTER(CULL_LAYER, "lCreateElem");

   if ((n = lCountDescr(dp)) <= 0) {
      LERROR(LECOUNTDESCR);
      DEXIT;
      return NULL;
   }

   if (!(ep = (lListElem *) malloc(sizeof(lListElem)))) {
      LERROR(LEMALLOC);
      DEXIT;
      return NULL;
   }

   ep->next = NULL;
   ep->prev = NULL;
 
   ep->descr = (lDescr *) malloc(sizeof(lDescr) * (n + 1));
   if (!ep->descr) {
      LERROR(LEMALLOC);
      free(ep);
      DEXIT;
      return NULL;
   }
   memcpy(ep->descr, dp, sizeof(lDescr) * (n + 1));

   /* copy hashing info in descriptor */
   for (i = 0; i <= n; i++) {
      if(dp[i].hash != NULL) {
         ep->descr[i].hash = cull_hash_copy_descr(&dp[i]);
      }
   }
                  
   ep->status = FREE_ELEM;
   if (!(ep->cont = (lMultiType *) calloc(1, sizeof(lMultiType) * n))) {
      LERROR(LEMALLOC);
      free(ep);
      DEXIT;
      return NULL;
   }

   DEXIT;
   return ep;
}

/****** cull/list/lCreateList() ***********************************************
*  NAME
*     lCreateList() -- Create an empty list 
*
*  SYNOPSIS
*     lList* lCreateList(const char *listname, const lDescr *descr) 
*
*  FUNCTION
*     Create an empty list with a given descriptor and a user defined
*     listname. 
*
*  INPUTS
*     const char *listname - list name 
*     const lDescr *descr  - descriptor 
*
*  RESULT
*     lList* - list pointer or NULL 
*******************************************************************************/
lList *lCreateList(const char *listname, const lDescr *descr) 
{
   lList *lp;
   int i, n;

   DENTER(CULL_LAYER, "lCreateList");

   if (!listname) {
      listname = "No list name specified";
   }

   if (!descr || descr[0].mt == lEndT) {
      LERROR(LEDESCRNULL);
      DEXIT;
      return NULL;
   }

   if (!(lp = (lList *) malloc(sizeof(lList)))) {
      LERROR(LEMALLOC);
      DEXIT;
      return NULL;
   }
   if (!(lp->listname = strdup(listname))) {
      LERROR(LESTRDUP);
      DEXIT;
      return NULL;
   }

   lp->nelem = 0;
   if ((n = lCountDescr(descr)) <= 0) {
      LERROR(LECOUNTDESCR);
      DEXIT;
      return NULL;
   }

   lp->first = NULL;
   lp->last = NULL;
   if (!(lp->descr = (lDescr *) malloc(sizeof(lDescr) * (n + 1)))) {
      LERROR(LEMALLOC);
      DEXIT;
      return NULL;
   }
   /* copy descriptor array */
   for (i = 0; i <= n; i++) {
      lp->descr[i].mt = descr[i].mt;
      lp->descr[i].nm = descr[i].nm;

      /* copy hashing information and create hashtable if necessary */
      if(descr[i].hash != NULL) {
         lp->descr[i].hash = cull_hash_create(&descr[i]);
      } else {
         lp->descr[i].hash = NULL;
      }
   }

   DEXIT;
   return lp;
}

/****** cull/list/lCreateElemList() *******************************************
*  NAME
*     lCreateElemList() -- Create a list with n elements 
*
*  SYNOPSIS
*     lList* lCreateElemList(const char *listname, const lDescr *descr, 
*                            int nr_elem) 
*
*  FUNCTION
*     Create a list with a given descriptor and insert 'nr_elem' 
*     only initialized elements 
*
*  INPUTS
*     const char *listname - list name
*     const lDescr *descr  - descriptor 
*     int nr_elem          - number of elements 
*
*  RESULT
*     lList* - list or NULL
*******************************************************************************/
lList *lCreateElemList(const char *listname, const lDescr *descr, int nr_elem) 
{
   lList *lp = NULL;
   lListElem *ep = NULL;
   int i;

   DENTER(CULL_LAYER, "lCreateElemList");

   if (!(lp = lCreateList(listname, descr))) {
      LERROR(LECREATELIST);
      DEXIT;
      return NULL;
   }

   for (i = 0; i < nr_elem; i++) {
      if (!(ep = lCreateElem(descr))) {
         LERROR(LECREATEELEM);
         lFreeList(lp);
         DEXIT;
         return NULL;
      }
      lAppendElem(lp, ep);
   }

   DEXIT;
   return lp;
}

/****** cull/list/lFreeElem() *************************************************
*  NAME
*     lFreeElem() -- Free a element including strings and sublists 
*
*  SYNOPSIS
*     lListElem* lFreeElem(lListElem *ep) 
*
*  FUNCTION
*     Free a element including strings and sublists 
*
*  INPUTS
*     lListElem *ep - element 
*
*  RESULT
*     lListElem* - NULL 
******************************************************************************/
lListElem *lFreeElem(lListElem *ep) 
{
   int i = 0;

   DENTER(CULL_LAYER, "lFreeElem");

   if (!ep) {
      DEXIT;
      return NULL;
   }

   if (!(ep->descr)) {
      LERROR(LEDESCRNULL);
      DPRINTF(("NULL descriptor not allowed !!!\n"));
      abort();
   }

   for (i = 0; ep->descr[i].mt != lEndT; i++) {
      /* remove element from hash tables */
      if(ep->descr[i].hash != NULL) {
         cull_hash_remove(ep, i);
      }
      
      switch (ep->descr[i].mt) {

      case lIntT:
      case lUlongT:
      case lFloatT:
      case lDoubleT:
      case lLongT:
      case lCharT:
      case lRefT:
         break;

      case lStringT:
         if (ep->cont[i].str)
            free(ep->cont[i].str);
         break;

      case lHostT:
         if (ep->cont[i].host)
            free(ep->cont[i].host);
         break;

      case lListT:
         if (ep->cont[i].glp)
            lFreeList(ep->cont[i].glp);
         break;

      default:
         unknownType("lFreeElem");
         break;
      }
   }

   /* lFreeElem is not responsible for descriptor array */
   if (ep->status == FREE_ELEM) {
      cull_hash_free_descr(ep->descr); 
      free(ep->descr);
   }   

   if (ep->cont)
      free(ep->cont);

   free(ep);

   DEXIT;
   return NULL;
}

/****** cull/list/lFreeList() *************************************************
*  NAME
*     lFreeList() -- Frees a list including all elements  
*
*  SYNOPSIS
*     lList* lFreeList(lList *lp) 
*
*  FUNCTION
*     Frees a list including all elements 
*
*  INPUTS
*     lList *lp - list 
*
*  RESULT
*     lList* - NULL 
******************************************************************************/
lList *lFreeList(lList *lp) 
{
   DENTER(CULL_LAYER, "lFreeList");

   if (!lp) {
      DEXIT;
      return NULL;
   }

   /* 
    * remove all hash tables, 
    * it is more efficient than removing it at the end 
    */
   if (lp->descr)
      cull_hash_free_descr(lp->descr);

   while (lp->first)
      lRemoveElem(lp, lp->first);

   if (lp->descr) {
      free(lp->descr);
   }   

   if (lp->listname)
      free(lp->listname);
   free(lp);

   DEXIT;
   return NULL;
}

/****** cull/list/lAddList() **************************************************
*  NAME
*     lAddList() -- Concatenate two lists 
*
*  SYNOPSIS
*     int lAddList(lList *lp0, lList *lp1) 
*
*  FUNCTION
*     Concatenate two lists of equal type throwing away the second list 
*
*  INPUTS
*     lList *lp0 - first list 
*     lList *lp1 - second list 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
******************************************************************************/
int lAddList(lList *lp0, lList *lp1) 
{
   lListElem *ep;
   const lDescr *dp0, *dp1;

   DENTER(CULL_LAYER, "lAddList");

   if (!lp1 || !lp0) {
      LERROR(LELISTNULL);
      DEXIT;
      return -1;
   }

   /* Check if the two lists are equal */
   dp0 = lGetListDescr(lp0);
   dp1 = lGetListDescr(lp1);
   if (lCompListDescr(dp0, dp1)) {
      LERROR(LEDIFFDESCR);
      DEXIT;
      return -1;
   }

   while (lp1->first) {
      if (!(ep = lDechainElem(lp1, lp1->first))) {
         LERROR(LEDECHAINELEM);
         DEXIT;
         return -1;
      }
      if (lAppendElem(lp0, ep) == -1) {
         LERROR(LEAPPENDELEM);
         DEXIT;
         return -1;
      }
   }

   lFreeList(lp1);

   DEXIT;
   return 0;
}

/****** cull/list/lCompListDescr() ********************************************
*  NAME
*     lCompListDescr() -- Compare two descriptors 
*
*  SYNOPSIS
*     int lCompListDescr(const lDescr *dp0, const lDescr *dp1) 
*
*  FUNCTION
*     Compare two descriptors 
*
*  INPUTS
*     const lDescr *dp0 - descriptor one 
*     const lDescr *dp1 - descriptor two 
*
*  RESULT
*     int - Result of compare operation
*         0 - equivalent
*        -1 - not equivalent
******************************************************************************/
int lCompListDescr(const lDescr *dp0, const lDescr *dp1) 
{
   int i, n, m;

   DENTER(CULL_LAYER, "lCompListDescr");

   if (!dp0 || !dp1) {
      LERROR(LELISTNULL);
      DEXIT;
      return -1;
   }

   /* Check if the two lists are equal */
   if ((n = lCountDescr(dp0)) <= 0) {
      LERROR(LECOUNTDESCR);
      DEXIT;
      return -1;
   }
   if ((m = lCountDescr(dp1)) <= 0) {
      LERROR(LECOUNTDESCR);
      DEXIT;
      return -1;
   }
   if (n == m) {
      for (i = 0; i < n; i++) {
         if (dp0[i].nm != dp1[i].nm || dp0[i].mt != dp1[i].mt) {
            LERROR(LEDIFFDESCR);
            DEXIT;
            return -1;
         }
      }
   }
   else {
      LERROR(LEDIFFDESCR);
      DEXIT;
      return -1;
   }

   DEXIT;
   return 0;
}

/****** cull/list/lCopyList() *************************************************
*  NAME
*     lCopyList() -- Copy a list including strings and sublists 
*
*  SYNOPSIS
*     lList* lCopyList(const char *name, const lList *src) 
*
*  FUNCTION
*     Copy a list including strings and sublists. The new list will
*     get 'name' as user defined name 
*
*  INPUTS
*     const char *name - list name 
*     const lList *src - source list 
*
*  RESULT
*     lList* - Copy of 'src' or NULL 
******************************************************************************/
lList *lCopyList(const char *name, const lList *src) 
{
   lList *dst = NULL;
   lListElem *sep;

   DENTER(CULL_LAYER, "lCopyList");

   if (!src) {
      LERROR(LELISTNULL);
      DEXIT;
      return NULL;
   }

   if (!name)
      name = src->listname;

   if (!name)
      name = "No list name specified";


   if (!(dst = lCreateList(name, src->descr))) {
      LERROR(LECREATELIST);
      DEXIT;
      return NULL;
   }

   for (sep = src->first; sep; sep = sep->next)
      if (lAppendElem(dst, lCopyElem(sep)) == -1) {
         lFreeList(dst);
         LERROR(LEAPPENDELEM);
         DEXIT;
         return NULL;
      }

   DEXIT;
   return dst;
}

/****** cull/list/lInsertElem() ***********************************************
*  NAME
*     lInsertElem() -- Insert element after another in a list 
*
*  SYNOPSIS
*     int lInsertElem(lList *lp, lListElem *ep, lListElem *new) 
*
*  FUNCTION
*     Insert a 'new' element after element 'ep' into list 'lp'.
*     If 'ep' is NULL then 'new' will be the first element in 'lp'.
*
*  INPUTS
*     lList *lp      - list 
*     lListElem *ep  - element 
*     lListElem *new - new element 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
******************************************************************************/
int lInsertElem(lList *lp, lListElem *ep, lListElem *new) 
{
   DENTER(CULL_LAYER, "lInsertElem");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return -1;
   }
   if (!new) {
      LERROR(LEELEMNULL);
      DEXIT;
      return -1;
   }
   if (ep) {
      new->prev = ep;
      new->next = ep->next;
      ep->next = new;
      if (new->next)            /* the new element has successors */
         new->next->prev = new;
      else                      /* the new element is the last element */
         lp->last = new;
   }
   else {                       /* insert as first element */
      new->prev = NULL;
      new->next = lp->first;
      if (!lp->first)           /* empty list ? */
         lp->last = new;
      else
         lp->first->prev = new;
      lp->first = new;
   }

   if (new->status == FREE_ELEM) {
      cull_hash_free_descr(new->descr);
      free(new->descr);
   }   
   new->status = BOUND_ELEM;
   new->descr = lp->descr;

   cull_hash_elem(new);
   
   lp->nelem++;

   DEXIT;
   return 0;
}

/****** cull/list/lAppendElem() ***********************************************
*  NAME
*     lAppendElem() -- Append element at the end of a list 
*
*  SYNOPSIS
*     int lAppendElem(lList *lp, lListElem *ep) 
*
*  FUNCTION
*     Append element 'ep' at the end of list 'lp' 
*
*  INPUTS
*     lList *lp     - list 
*     lListElem *ep - element 
*
*  RESULT
*     int - error state 
*         0 - OK
*        -1 - Error
******************************************************************************/
int lAppendElem(lList *lp, lListElem *ep) 
{
#ifdef __INSIGHT__
/* JG: this code is thorougly tested and really should be ok, but insure complains */
_Insight_set_option("suppress", "LEAK_ASSIGN");
#endif
   DENTER(CULL_LAYER, "lAppendElem");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return -1;
   }
   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return -1;
   }

   /* is the element ep still chained in an other list, this is not allowed ? */
   if (ep->status == BOUND_ELEM) {
      DPRINTF(("WARNING: tried to append chained element\n"));
      lWriteElem(ep);
      DEXIT;
      abort();
   }

   if (lp->last) {
      lp->last->next = ep;
      ep->prev = lp->last;
      lp->last = ep;
      ep->next = NULL;
   }
   else {
      lp->last = lp->first = ep;
      ep->prev = ep->next = NULL;
   }

   if (ep->status == FREE_ELEM) {
      cull_hash_free_descr(ep->descr);
      free(ep->descr);
   }
   ep->status = BOUND_ELEM;
   ep->descr = lp->descr;

   cull_hash_elem(ep);
   lp->nelem++;

   DEXIT;
   return 0;
#ifdef __INSIGHT__
_Insight_set_option("unsuppress", "LEAK_ASSIGN");
#endif
}

/****** cull/list/lRemoveElem() ***********************************************
*  NAME
*     lRemoveElem() -- Delete a element from a list 
*
*  SYNOPSIS
*     int lRemoveElem(lList *lp, lListElem *ep) 
*
*  FUNCTION
*     Remove element 'ep' from list 'lp'. 'ep' gets deleted.
*
*  INPUTS
*     lList *lp     - list 
*     lListElem *ep - element 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error 
*******************************************************************************/
int lRemoveElem(lList *lp, lListElem *ep) 
{
   DENTER(CULL_LAYER, "lRemoveElem");

   if (!lp || !ep) {
      DEXIT;
      return -1;
   }

   if (lp->descr != ep->descr) {
      DPRINTF(("Dechaining element from other list !!!\n"));
      abort();
   }

   if (ep->prev) {
      ep->prev->next = ep->next;
   }
   else {
      lp->first = ep->next;
   }

   if (ep->next) {
      ep->next->prev = ep->prev;
   }
   else {
      lp->last = ep->prev;
   }

   /* NULL the ep next and previous pointers */
   ep->prev = ep->next = NULL;

   lp->nelem--;

   lFreeElem(ep);

   DEXIT;
   return 0;
}

/****** cull/list/lDechainElem() **********************************************
*  NAME
*     lDechainElem() -- Remove a element from a list 
*
*  SYNOPSIS
*     lListElem* lDechainElem(lList *lp, lListElem *ep) 
*
*  FUNCTION
*     Remove element 'ep' from list 'lp'. 'ep' gets not deleted. 
*
*  INPUTS
*     lList *lp     - list 
*     lListElem *ep - element 
*
*  RESULT
*     lListElem* - dechained element or NULL
******************************************************************************/
lListElem *lDechainElem(lList *lp, lListElem *ep) 
{
   int i;

   DENTER(CULL_LAYER, "lDechainElem");

   if (!lp) {
      LERROR(LELISTNULL);
      DEXIT;
      return NULL;
   }
   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return NULL;
   }
   if (lp->descr != ep->descr) {
      DPRINTF(("Dechaining element from other list !!!\n"));
      abort();
   }

   if (ep->prev)
      ep->prev->next = ep->next;
   else
      lp->first = ep->next;

   if (ep->next)
      ep->next->prev = ep->prev;
   else
      lp->last = ep->prev;

   /* remove hash entries */
   for(i = 0; ep->descr[i].mt != lEndT; i++) {
      if(ep->descr[i].hash != NULL) {
         cull_hash_remove(ep, i);
      }
   }

   /* NULL the ep next and previous pointers */
   ep->prev = ep->next = (lListElem *) NULL;
   ep->descr = lCopyDescr(ep->descr);
   ep->status = FREE_ELEM;
   lp->nelem--;

   DEXIT;
   return ep;
}

/****** cull/list/lFirst() ****************************************************
*  NAME
*     lFirst() -- Return the first element of a list 
*
*  SYNOPSIS
*     lListElem* lFirst(const lList *slp) 
*
*  FUNCTION
*     Return the first element of a list. 
*
*  INPUTS
*     const lList *slp - list 
*
*  RESULT
*     lListElem* - first element or NULL 
******************************************************************************/
lListElem *lFirst(const lList *slp) 
{
   DENTER(CULL_LAYER, "lFirst");
   DEXIT;
   return slp ? slp->first : NULL;
}

/****** cull/list/lLast() *****************************************************
*  NAME
*     lLast() -- Returns the last element of a list 
*
*  SYNOPSIS
*     lListElem* lLast(const lList *slp) 
*
*  FUNCTION
*     Returns the last element of a list. 
*
*  INPUTS
*     const lList *slp - list 
*
*  RESULT
*     lListElem* - last element or NULL 
******************************************************************************/
lListElem *lLast(const lList *slp) 
{
   DENTER(CULL_LAYER, "lLast");
   DEXIT;
   return slp ? slp->last : NULL;
}

/****** cull/list/lNext() *****************************************************
*  NAME
*     lNext() -- Returns the next element or NULL
*
*  SYNOPSIS
*     lListElem* lNext(const lListElem *sep) 
*
*  FUNCTION
*     Returns the next element of 'sep' or NULL 
*
*  INPUTS
*     const lListElem *sep - element 
*
*  RESULT
*     lListElem* - next element or NULL 
*******************************************************************************/
lListElem *lNext(const lListElem *sep) 
{
   DENTER(CULL_LAYER, "lNext");
   DEXIT;
   return sep ? sep->next : NULL;
}

/****** cull/list/lPrev() *****************************************************
*  NAME
*     lPrev() -- Returns the previous element or NULL 
*
*  SYNOPSIS
*     lListElem* lPrev(const lListElem *sep) 
*
*  FUNCTION
*     Returns the previous element or NULL. 
*
*  INPUTS
*     const lListElem *sep - element 
*
*  RESULT
*     lListElem* - previous element 
******************************************************************************/
lListElem *lPrev(const lListElem *sep) 
{
   DENTER(CULL_LAYER, "lPrev");
   DEXIT;
   return sep ? sep->prev : NULL;
}

/****** cull/list/lFindFirst() ************************************************
*  NAME
*     lFindFirst() -- Returns first element fulfilling condition 
*
*  SYNOPSIS
*     lListElem* lFindFirst(const lList *slp, const lCondition *cp) 
*
*  FUNCTION
*     Returns the first element fulfilling the condition 'cp' or
*     NULL if nothing is found. If the condition is NULL the first
*     element is delivered. 
*
*  INPUTS
*     const lList *slp     - list 
*     const lCondition *cp - condition 
*
*  RESULT
*     lListElem* - element or NULL 
******************************************************************************/
lListElem *lFindFirst(const lList *slp, const lCondition *cp) 
{
   lListElem *ep;

   DENTER(CULL_LAYER, "lFindFirst");

   if (!slp) {
      LERROR(LELISTNULL);
      DEXIT;
      return NULL;
   }

   /* ep->next=NULL for the last element */
   for (ep = slp->first; ep && !lCompare(ep, cp); ep = ep->next);

   DEXIT;
   return ep;
}

/****** cull/list/lFindLast() *************************************************
*  NAME
*     lFindLast() -- Returns last element fulfilling condition 
*
*  SYNOPSIS
*     lListElem* lFindLast(const lList *slp, const lCondition *cp) 
*
*  FUNCTION
*     Retruns the last element fulfilling the condition 'cp' or NULL
*     if nothing is found. If the condition is NULL then the last
*     element is delivered.
*
*  INPUTS
*     const lList *slp     - list 
*     const lCondition *cp - condition 
*
*  RESULT
*     lListElem* - element or NULL
******************************************************************************/
lListElem *lFindLast(const lList *slp, const lCondition *cp) 
{
   lListElem *ep;

   DENTER(CULL_LAYER, "lFindLast");

   if (!slp) {
      LERROR(LELISTNULL);
      DEXIT;
      return NULL;
   }

   /* ep->prev=NULL for the first element */
   for (ep = slp->last; ep && !lCompare(ep, cp); ep = ep->prev);

   DEXIT;
   return ep;
}

/****** cull/list/lFindNext() *************************************************
*  NAME
*     lFindNext() -- Returns the next element fulfilling condition 
*
*  SYNOPSIS
*     lListElem* lFindNext(const lListElem *ep, const lCondition *cp) 
*
*  FUNCTION
*     Returns the next element fulfilling the condition 'cp' or NULL
*     if nothing is found. If condition is NULL than the following
*     element is delivered. 
*
*  INPUTS
*     const lListElem *ep  - element 
*     const lCondition *cp - condition 
*
*  RESULT
*     lListElem* - element or NULL 
*******************************************************************************/
lListElem *lFindNext(const lListElem *ep, const lCondition *cp) 
{
   DENTER(CULL_LAYER, "lFindNext");

   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return NULL;
   }

   do {
      ep = ep->next;
   } while (ep && (lCompare(ep, cp) == 0));

   DEXIT;
   return (lListElem *)ep;
}

/****** cull/list/lFindPrev() *************************************************
*  NAME
*     lFindPrev() -- Returns previous element fulfilling condition 
*
*  SYNOPSIS
*     lListElem* lFindPrev(const lListElem *ep, const lCondition *cp) 
*
*  FUNCTION
*     Returns the previous element fulfilling the condition 'cp' or
*     NULL if nothing is found. If condition is NULL than the following
*     element is delivered. 
*
*  INPUTS
*     const lListElem *ep  - element 
*     const lCondition *cp - condition 
*
*  RESULT
*     lListElem* - element or NULL 
******************************************************************************/
lListElem *lFindPrev(const lListElem *ep, const lCondition *cp) 
{
   DENTER(CULL_LAYER, "lFindNext");

   if (!ep) {
      LERROR(LEELEMNULL);
      DEXIT;
      return NULL;
   }

   do {
      ep = ep->prev;
   } while (ep && (lCompare(ep, cp) == 0));

   DEXIT;
   return (lListElem *) ep;
}

/****** cull/list/lSortList2() ************************************************
*  NAME
*     lSortList2() -- Sort a given list 
*
*  SYNOPSIS
*     int lSortList2(lList * lp, const char *fmt, ...) 
*
*  FUNCTION
*     Sort a given list. The sorting order is given by the format 
*     string and additional arguments. 
*
*  INPUTS
*     lList * lp      - list 
*     const char *fmt - format string (see lParseSortOrder()) 
*     ...             - additional arguments (see lParseSortOrder()) 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*
*  SEE ALSO
*     cull/list/lParseSortOrder() 
******************************************************************************/
int lSortList2(lList * lp, const char *fmt, ...)
{
   va_list ap;
   lListElem *temp;
   lListElem *ep, *cep, *tep;

   lSortOrder *sp;

   DENTER(CULL_LAYER, "lSortList2");

   va_start(ap, fmt);
   if (!lp) {
      LERROR(LELISTNULL);
      va_end(ap);
      DEXIT;
      return -1;
   }
   if (!fmt) {
      LERROR(LENOFORMATSTR);
      DEXIT;
      va_end(ap);
      return -1;
   }
   if (!(sp = lParseSortOrder(lp->descr, fmt, ap))) {
      LERROR(LEPARSESORTORD);
      va_end(ap);
      DEXIT;
      return -1;
   }

   temp = lp->first;

   /* lp->descr must not be NULL'ed, it is used in lInsertElem */
   lp->first = NULL;
   lp->last = NULL;
   lp->nelem = 0;

   while (temp) {
      tep = NULL;
      /* lInsertElem resets ep->next, ep->prev */
      ep = temp;
      temp = ep->next;
      /* 
       * insertion sort:
       * go through lp until new element < current element in dlp
       */
      for_each(cep, lp) {
         if (lSortCompare(ep, cep, sp) < 0)
            break;
         tep = cep;
      }
      /* insert in lp at the right position */
      lInsertElem(lp, tep, ep);
   }

   sp = lFreeSortOrder(sp);

   va_end(ap);

   DEXIT;
   return 0;
}

/****** cull/list/lPSortList() ************************************************
*  NAME
*     lPSortList() -- Sort a given list 
*
*  SYNOPSIS
*     int lPSortList(lList * lp, const char *fmt, ...) 
*
*  FUNCTION
*     Sort a given list. The sorting order is given by the format 
*     string and additional arguments. 
*
*  INPUTS
*     lList * lp      - list 
*     const char *fmt - format string (see lParseSortOrder()) 
*     ...             - additional arguments (see lParseSortOrder()) 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
*
*  SEE ALSO
*     cull/list/lParseSortOrder() 
******************************************************************************/
int lPSortList(lList * lp, const char *fmt,...)
{
   va_list ap;

   lSortOrder *sp;

   DENTER(CULL_LAYER, "lPSortList");

   va_start(ap, fmt);
   if (!lp || !fmt) {
      LERROR(LELISTNULL);
      va_end(ap);
      DEXIT;
      return -1;
   }
   if (!(sp = lParseSortOrder(lp->descr, fmt, ap))) {
      LERROR(LEPARSESORTORD);
      va_end(ap);
      DEXIT;
      return -1;
   }

   lSortList(lp, sp);

   va_end(ap);
   lFreeSortOrder(sp);

   DEXIT;
   return 0;
}

/****** cull/list/lSortList() *************************************************
*  NAME
*     lSortList() -- Sort list according to sort order object 
*
*  SYNOPSIS
*     int lSortList(lList *lp, const lSortOrder *sp) 
*
*  FUNCTION
*     Sort list according to sort order object. 
*
*  INPUTS
*     lList *lp            - list 
*     const lSortOrder *sp - sort order object 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
******************************************************************************/
int lSortList(lList *lp, const lSortOrder *sp) 
{
   lListElem *ep;
   lListElem **pointer;
   int i, n;

   DENTER(CULL_LAYER, "lSortList");

   if (!lp) {
      DEXIT;
      return 0;                 /* ok list is sorted */
   }

   /* 
    * step 1: build up a pointer array for use of qsort
    */

   n = lGetNumberOfElem(lp);
   if (n < 2) {
      DEXIT;
      return 0;                 /* ok list is sorted */
   }

   if (!(pointer = (lListElem **) malloc(sizeof(lListElem *) * n))) {
      DEXIT;
      return -1;                /* low memory */
   }

   for (i = 0, ep = lFirst(lp); ep; i++, ep = lNext(ep))
      pointer[i] = ep;

   /* 
    * step 2: sort the pointer array using parsed sort order 
    */
   lSetGlobalSortOrder(sp);     
   /* this is done to pass the sort order */
   /* to the lSortCompare function called */
   /* by lSortCompareUsingGlobal          */
   qsort((void *) pointer, n, sizeof(lListElem *), lSortCompareUsingGlobal);

   /* 
    * step 3: relink elements in list according pointer array
    */
   lp->first = pointer[0];
   lp->last = pointer[n - 1];

   /* handle first element separatly */
   pointer[0]->prev = NULL;
   pointer[n - 1]->next = NULL;

   if (n > 1) {
      pointer[0]->next = pointer[1];
      pointer[n - 1]->prev = pointer[n - 2];
   }

   for (i = 1; i < n - 1; i++) {
      pointer[i]->prev = pointer[i - 1];
      pointer[i]->next = pointer[i + 1];
   }

   free(pointer);

   DEXIT;
   return 0;
}

/****** cull/list/lUniqStr() **************************************************
*  NAME
*     lUniqStr() -- Uniq a string key list 
*
*  SYNOPSIS
*     int lUniqStr(lList *lp, int keyfield) 
*
*  FUNCTION
*     Uniq a string key list 
*
*  INPUTS
*     lList *lp    - list 
*     int keyfield - string field name id 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
******************************************************************************/
int lUniqStr(lList *lp, int keyfield) 
{
   lListElem *ep;
   lListElem *rep;

   DENTER(CULL_LAYER, "lUniqStr");

   /*
    * sort the list first to make our algorithm work
    */
   if (lPSortList(lp, "%I+", keyfield)) {
      DEXIT;
      return -1;
   }

   /*
    * go over all elements and remove following elements
    */
   ep = lFirst(lp);
   while (ep) {
      rep = lNext(ep);
      while (rep &&
          !strcmp(lGetString(rep, keyfield), lGetString(ep, keyfield))) {
         lRemoveElem(lp, rep);
         rep = lNext(ep);
      }
      ep = lNext(ep);
   }

   DEXIT;
   return 0;
}

/****** cull/list/lUniqHost() *************************************************
*  NAME
*     lUniqHost() -- Uniq a host key list 
*
*  SYNOPSIS
*     int lUniqHost(lList *lp, int keyfield) 
*
*  FUNCTION
*     Uniq a hostname key list. 
*
*  INPUTS
*     lList *lp    - list 
*     int keyfield - host field 
*
*  RESULT
*     int - error state
*         0 - OK
*        -1 - Error
******************************************************************************/
int lUniqHost(lList *lp, int keyfield) 
{
   lListElem *ep;
   lListElem *rep;

   DENTER(CULL_LAYER, "lUniqHost");

   /*
    * sort the list first to make our algorithm work
    */
   if (lPSortList(lp, "%I+", keyfield)) {
      DEXIT;
      return -1;
   }

   /*
    * go over all elements and remove following elements
    */
   ep = lFirst(lp);
   while (ep) {
      rep = lNext(ep);
      while (rep &&
          !strcmp(lGetHost(rep, keyfield), lGetHost(ep, keyfield))) {
         lRemoveElem(lp, rep);
         rep = lNext(ep);
      }
      ep = lNext(ep);
   }

   DEXIT;
   return 0;
}

