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

#include <fnmatch.h>
#include <string.h>

#include "rmon/sgermon.h"

#include "uti/sge_string.h"
#include "uti/sge_log.h"
#include "uti/sge_dstring.h"
#include "uti/sge_signal.h"

#include "cull/cull_list.h"

#include "sge.h"
#include "sge_answer.h"
#include "sge_object.h"
#include "sge_cqueue.h"
#include "sge_event.h"
#include "sge_qinstance.h"
#include "sge_qinstance_state.h"
#include "sge_qref.h"
#include "sge_subordinate.h"

#include "msg_sgeobjlib.h"

/* -----------------------------------------------

   test suspend on subordinate (nice neuron)

    A1            C1
      \          /
       \        /
        \      /
  A2--------->B-------> C2
        /      \
       /        \
      /          \
    A3            C3

   a queue C subordinated by B must be suspended if
   the used slots of queue B meet the thresold for C

*/
/*
int used,      umber of slots actually used in queue B    
int total,     total number of slots in queue B              
lListElem *so  SO_Type referencing to a queue C              
*/
bool
tst_sos(int used, int total, lListElem *so)
{
   u_long32 threshold;

   DENTER(TOP_LAYER, "tst_sos");

   /*
      then check if B's usage meets the threshold
      for suspension of the subordinated queue C
   */
   if (!(threshold=lGetUlong(so, SO_threshold))) {
      /* queue must be full for suspend of queue C */
      DPRINTF(("TSTSOS: %sfull -> %ssuspended\n", (used>=total)?"":"not ",
               (used>=total)?"":"not "));
      DEXIT;
      return (used>=total) ? true : false;
   }

   /* used slots greater or equal threshold */
   DPRINTF(("TSTSOS: "sge_u32" slots used (limit "sge_u32") -> %ssuspended\n",
            used, threshold, ( (u_long32)(used) >= threshold)?"":"not "));
   DEXIT;
   return ((u_long32)used) >= threshold ? true : false;
}

const char *
so_list_append_to_dstring(const lList *this_list, dstring *string)
{
   const char *ret = NULL;

   DENTER(BASIS_LAYER, "so_list_append_to_dstring");
   if (string != NULL) {
      lListElem *elem = NULL;
      bool printed = false;

      for_each(elem, this_list) {
         if (printed) {
            sge_dstring_append (string, " ");
         }
         
         sge_dstring_append(string, lGetString(elem, SO_name));
         if (lGetUlong(elem, SO_threshold)) {
            sge_dstring_sprintf_append(string, "="sge_u32"%s",
                                       lGetUlong(elem, SO_threshold),
                                       lNext(elem) ? "," : "");
         }
         printed = true;
      }
      if (!printed) {
         sge_dstring_append(string, "NONE");
      }
      ret = sge_dstring_get_string(string);
   }
   DRETURN(ret);
}

/*
   add new so elements
   if element already exists then possibly overwrite threshold value
   (lower values are prefered)
*/
bool
so_list_add(lList **this_list, lList **answer_list, const char *so_name,
            u_long32 threshold)
{
   DENTER(TOP_LAYER, "so_list_add");

   if (this_list != NULL && so_name != NULL) {
      lListElem *elem = lGetElemStr(*this_list, SO_name, so_name);
   
      if (elem != NULL) {
         u_long32 current_threshold = lGetUlong(elem, SO_threshold);

         if (threshold != 0 && threshold < current_threshold) {
            DPRINTF (("Replacing entry with higher threshold: %d => %d\n",
                      current_threshold, threshold));
            lSetUlong(elem, SO_threshold, threshold);
         }
      } else {
         DPRINTF (("Adding new entry with threshold: %d\n", threshold));
         elem = lAddElemStr(this_list, SO_name, so_name, SO_Type);
         lSetUlong(elem, SO_threshold, threshold);
      }
   }
   
   DRETURN(true);
}

/****** sgeobj/subordinate/so_list_resolve() ***********************************
*  NAME
*     so_list_resolve() -- Resolve a generic list of subordinates into their
*                          full names.
*
*  SYNOPSIS
*     bool so_list_resolve(const lList *so_list, lList **answer_list,
*                          lList **resolved_so_list, const char *cq_name,
*                          const char *hostname)
*
*  FUNCTION
*     Goes through every entry in the so_list, retrieves the corresponding
*     cqueue, gets the qinstance for hostname from the cqueue, and adds the
*     qinstance's full name to the resolved_so_list.  If qi_name is given, the
*     subordinate list will be checked to make sure that it doesn't contain the
*     queue to which the list is subordinate.
*
*  INPUTS
*     const lList *so_list     - the list of subordinates to resolve
*     lList **answer_list      - answer list for errors
*     lList **resolved_so_list - the destination list for resolved subordinates
*     const char *cq_name      - the queue name of the qinstance to which the
*                                subordinate list is subordinate
*     const char *hostname     - the hostname for the queue to which the
*                                subordinate list is subordinate
*
*  RESULT
*     bool - error state
*        true  - success
*        false - error
*******************************************************************************/
bool
so_list_resolve(const lList *so_list, lList **answer_list,
                lList **resolved_so_list, const char *cq_name,
                const char *hostname)
{
   bool ret = true;

   DENTER(TOP_LAYER, "so_list_resolve");
   if ((so_list != NULL) && (hostname != NULL)) {
      lListElem *so;
      lList *cqueue_list = *(object_type_get_master_list(SGE_TYPE_CQUEUE));

      if (cq_name != NULL) {
         DPRINTF(("Finding subordinates for %s on %s\n", cq_name, hostname));
      } else {
         DPRINTF(("Finding subordinates on host %s\n", hostname));
      }
      
      /* Get the list of resolved qinstances for each subsordinate. */
      for_each (so, so_list) {
         const char *qinstance_name = NULL;
         const char *cq_name_str = lGetString (so, SO_name);

         lListElem *cqueue = cqueue_list_locate(cqueue_list, cq_name_str);

         if (cqueue != NULL) {
            lListElem *qinstance = cqueue_locate_qinstance(cqueue, hostname);

            /* If this cqueue doesn't have a qinstance on this host,
             * just skip it. */
            if (qinstance != NULL) {
               qinstance_name = lGetString(qinstance, QU_full_name);

               so_list_add(resolved_so_list, answer_list, qinstance_name, lGetUlong(so, SO_threshold));
               continue;
            }
         }
         if (cq_name && strcmp(cq_name, cq_name_str) == 0){
            dstring buffer = DSTRING_INIT;
            
            qinstance_name = sge_dstring_sprintf(&buffer, "%s@%s", cq_name, hostname);
            so_list_add(resolved_so_list, answer_list, qinstance_name, lGetUlong(so, SO_threshold));
            sge_dstring_free(&buffer);
         }
      }
   }

   DRETURN(ret);
}
