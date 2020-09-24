/* Copyright (c) 2020 XEPIC Corporation Limited */
/*
 * Copyright (c) 1999-2016 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
// fsdb NEW FEATURE   //

#ifdef USE_FSDB

# include "sys_priv.h"
# include "vcd_priv.h"

/*
 * This file contains the implementations of the VCD related functions.
 */

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <time.h>
# include  "ivl_alloc.h"
# include  "ffwAPI.h"

char *truncate_bitvec(char *s);
void gen_new_vcd_id(void);

static struct t_vpi_time fsdbzero_delay = { vpiSimTime, 0, 0, 0.0 };
static char fsdbvcdid[8] = "!";

struct vcd_info {
      vpiHandle item;
      vpiHandle cb;
      struct t_vpi_time time;
      const char *ident;
      struct vcd_info *next;
      struct vcd_info *dmp_next;
      int scheduled;
};


static char *fsdbdump_path = NULL;
static ffwObject *fsdbdump_file = NULL;


//fsdb copy
static struct vcd_info *fsdb_list = NULL;
static struct vcd_info *fsdb_dmp_list = NULL;
static PLI_UINT64 fsdb_cur_time = 0;
static int fsdbdump_is_off = 0;
static long fsdbdump_limit = 0;
static int fsdbdump_is_full = 0;
static int fsdbfinish_status = 0;


/*
 * managed qsorted list of scope names/variables for duplicates bsearching
 */

struct vcd_names_list_s fsdb_vcd_tab = { 0, 0, 0, 0 };
struct vcd_names_list_s fsdb_vcd_var = { 0, 0, 0, 0 };

static int fsdbdumpvars_status = 0;//0:fresh 1:cb installed, 2:callback done
static PLI_UINT64 fsdbdumpvars_time;
__inline__ static int fsdbdump_header_pending(void)
{
	return fsdbdumpvars_status != 2;
}

void fsdbnexus_ident_delete();
const char* fsdbfind_nexus_ident(int nex);
void fsdbset_nexus_ident(int nex, const char*id);

static PLI_INT32 sys_fsdbdumpfile_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
	vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
	vpiHandle argv = vpi_iterate(vpiArgument, callh);
	char *path;

	/*$dumpfile must be called before dumpvars starts*/
	/*do not know the detail, to be done*/
	/*
	if (dumpvars_status != 0) {
	    char msg[64];
	    snprintf(msg, sizeof(msg), "VCD warning: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;
	    vpi_printf("%s %s called after $dumpvars started,\n", msg, name);
	    vpi_printf("%*s using existing file (%s).\n",
	               (int) strlen(msg), " ", dump_path);
	    vpi_free_object(argv);
	    return 0;
	}
	*/
	path = get_filename_with_suffix(callh, name, vpi_scan(argv), "fsdb");
	vpi_free_object(argv);
	if (! path) return 0;

	printf("dump path is %s\n",path);

	if (fsdbdump_path) {
		vpi_printf("VCD warning: %s:%d: ", vpi_get_str(vpiFile, callh),
		           (int)vpi_get(vpiLineNo, callh));
		vpi_printf("Overriding dump file %s with %s.\n", fsdbdump_path, path);
		free(fsdbdump_path);
		}
	fsdbdump_path = path;

	return 0;
}

/*
 * managed qsorted list of scope names/variables for duplicates bsearching
 */

//fsdbdump value change
static void fsdbshow_this_item(struct vcd_info*info)
{
      s_vpi_value value;
      PLI_INT32 type = vpi_get(vpiType, info->item);

      if (type == vpiRealVar) {
	  	vpi_printf("WARNING: FSDB Dump file rightnow do not support vpiRealVar %s:%d ",__FILE__,__LINE__);
	    //value.format = vpiRealVal;
	    //vpi_get_value(info->item, &value);
	    //fprintf(dump_file, "r%.16g %s\n", value.value.real, info->ident);
      } else if (type == vpiNamedEvent) {
		vpi_printf("WARNING: FSDB Dump file rightnow do not support vpiNamedEvent %s:%d ",__FILE__,__LINE__);
		//fprintf(dump_file, "1%s\n", info->ident);
      } else if (vpi_get(vpiSize, info->item) == 1) {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
		ffw_CreateVarValueByHandle(fsdbdump_file, (fsdbVarHandle)info->ident, (byte_T*)value.value.str);
	    //fprintf(dump_file, "%s%s\n", value.value.str, info->ident);
      } else {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
		ffw_CreateVarValueByHandle(fsdbdump_file, (fsdbVarHandle)info->ident, 
			(byte_T*)truncate_bitvec(value.value.str) );
		//fprintf(dump_file, "b%s %s\n", truncate_bitvec(value.value.str),
		//    info->ident);
      }
}

static PLI_INT32 fsdbvariable_cb_2(p_cb_data cause)
{
      struct vcd_info* info = fsdb_dmp_list;
      PLI_UINT64 now = timerec_to_time64(cause->time);

      if (now != fsdb_cur_time) {
		
	  	ffw_CreateXCoorByHnL(fsdbdump_file,cause->time->high , cause->time->low);
	    //fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", now);
	    fsdb_cur_time = now;
      }
	  
      do {
           fsdbshow_this_item(info);
           info->scheduled = 0;
      } while ((info = info->dmp_next) != 0);
	  

      fsdb_dmp_list = 0;

      return 0;
}

static PLI_INT32 fsdbvariable_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

      if (fsdbdump_is_full) return 0;
      if (fsdbdump_is_off) return 0;
      if (fsdbdump_header_pending()) return 0;
      if (info->scheduled) return 0;


	  /* can not control file size check,abandon in fsdbwrite
      if ((fsdbdump_limit > 0) && (ftell(fsdbdump_file) > fsdbdump_limit)) {
            fsdbdump_is_full = 1;
            vpi_printf("WARNING: FSDB Dump file limit (%ld bytes) "
                               "exceeded.\n", fsdbdump_limit);
	  		
            //fprintf(dump_file, "$comment Dump file limit (%ld bytes) "
            //                   "exceeded. $end\n", dump_limit);
            return 0;
      }
      */

      if (!fsdb_dmp_list) {
          cb = *cause;
	  	  cb.time = &fsdbzero_delay;
          cb.reason = cbReadOnlySynch;
          cb.cb_rtn = fsdbvariable_cb_2;
          vpi_register_cb(&cb);
      }

      info->scheduled = 1;
      info->dmp_next  = fsdb_dmp_list;
      fsdb_dmp_list    = info;

      return 0;
}


static PLI_INT32 fsdbdumpvars_cb(p_cb_data cause)
{
      if (fsdbdumpvars_status != 1) return 0;

      fsdbdumpvars_status = 2;

      fsdbdumpvars_time = timerec_to_time64(cause->time);
      fsdb_cur_time = fsdbdumpvars_time;

	  /*
      fprintf(dump_file, "$enddefinitions $end\n");

      if (!fsdbdump_is_off) {
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", dumpvars_time);
	    fprintf(dump_file, "$dumpvars\n");
	    vcd_checkpoint();
	    fprintf(dump_file, "$end\n");
      }
      */

      return 0;
}


static PLI_INT32 fsdbfinish_cb(p_cb_data cause)
{
      struct vcd_info *cur, *next;

      if (fsdbfinish_status != 0) return 0;

      fsdbfinish_status = 1;

      fsdbdumpvars_time = timerec_to_time64(cause->time);

	  /*
      if (!dump_is_off && !dump_is_full && dumpvars_time != vcd_cur_time) {
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", dumpvars_time);
      }
      */

      ffw_Close(fsdbdump_file);

      for (cur = fsdb_list ;  cur ;  cur = next) {
	    next = cur->next;
	    free((char *)cur->ident);
	    free(cur);
      }
      fsdb_list = 0;
      vcd_names_delete(&fsdb_vcd_tab);
      vcd_names_delete(&fsdb_vcd_var);
      fsdbnexus_ident_delete();
      free(fsdbdump_path);
      fsdbdump_path = 0;

      return 0;
}


static int fsdbdraw_scope(vpiHandle item, vpiHandle callh)
{
      int depth;
      const char *name;
      const char *type;
	  fsdbScopeType fsdbType;


      vpiHandle scope = vpi_handle(vpiScope, item);
      if (!scope) return 0;

      depth = 1 + fsdbdraw_scope(scope, callh);
      name = vpi_get_str(vpiName, scope);

      switch (vpi_get(vpiType, scope)) {
	  case vpiNamedBegin:  type = "begin"; 		fsdbType = FSDB_ST_VCD_BEGIN;   	break;
	  case vpiGenScope:    type = "begin"; 		fsdbType = FSDB_ST_VCD_GENERATE;  	break;
	  case vpiTask:        type = "task";  		fsdbType = FSDB_ST_VCD_TASK;  		break;
	  case vpiFunction:    type = "function";   fsdbType = FSDB_ST_VCD_FUNCTION;	break;
	  case vpiNamedFork:   type = "fork";       fsdbType = FSDB_ST_VCD_FORK;		break;
	  case vpiModule:      type = "module";     fsdbType = FSDB_ST_VCD_MODULE;		break;
	  default:
	    type = "invalid";
	    vpi_printf("FSDB Error: %s:%d: $fsdbdumpvars: Unsupported scope "
	               "type (%d)\n", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh),
	               (int)vpi_get(vpiType, item));
            assert(0);
      }

	  //add fsdb dump file here 
      //fprintf(dump_file, "$scope %s %s $end\n", type, name);      
	  ffw_CreateScope(fsdbdump_file, fsdbType, (char*)name);

      return depth;
}


static void fsdbscan_item(unsigned depth, vpiHandle item, int skip)
{
      struct t_cb_data cb;
      struct vcd_info* info;

      //const char *type;
      const char *name;
      const char *fullname;
      const char *prefix;
      const char *ident;
      int nexus_id;
      unsigned size;
      PLI_INT32 item_type;
	  fsdbScopeType fsdbSType;
	  fsdbVarType fsdbVType;

	/* Get the displayed type for the various $var and $scope types. */
	/* Not all of these are supported now, but they should be in a
	 * future development version. */
      item_type = vpi_get(vpiType, item);
      switch (item_type) {
	  case vpiNamedEvent: fsdbVType = FSDB_VT_VCD_EVENT; break;
	  case vpiIntVar:
	  case vpiIntegerVar: fsdbVType = FSDB_VT_VCD_INTEGER; break;
	  case vpiParameter:  fsdbVType = FSDB_VT_VCD_PARAMETER; break;
	    /* Icarus converts realtime to real. */
	  case vpiRealVar:    fsdbVType = FSDB_VT_VCD_REAL; break;
	  case vpiMemoryWord:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiLongIntVar:
	  case vpiReg:        fsdbVType = FSDB_VT_VCD_REG; break;
	    /* Icarus converts a time to a plain register. */
	  case vpiTimeVar:    fsdbVType = FSDB_VT_VCD_TIME; break;
	  case vpiNet:
	    switch (vpi_get(vpiNetType, item)) {
		case vpiWand:    fsdbVType = FSDB_VT_VCD_WAND; break;
		case vpiWor:     fsdbVType = FSDB_VT_VCD_WOR; break;
		case vpiTri:     fsdbVType = FSDB_VT_VCD_TRI; break;
		case vpiTri0:    fsdbVType = FSDB_VT_VCD_TRI0; break;
		case vpiTri1:    fsdbVType = FSDB_VT_VCD_TRI1; break;
		case vpiTriReg:  fsdbVType = FSDB_VT_VCD_TRIREG; break;
		case vpiTriAnd:  fsdbVType = FSDB_VT_VCD_TRIAND; break;
		case vpiTriOr:   fsdbVType = FSDB_VT_VCD_TRIOR; break;
		case vpiSupply1: fsdbVType = FSDB_VT_VCD_SUPPLY1; break;
		case vpiSupply0: fsdbVType = FSDB_VT_VCD_SUPPLY0; break;
		default:         fsdbVType = FSDB_VT_VCD_WIRE; break;
	    }
	    break;

	  case vpiNamedBegin: fsdbSType = FSDB_ST_VCD_BEGIN; break;
	  case vpiGenScope:   fsdbSType = FSDB_ST_VCD_GENERATE; break;
	  case vpiNamedFork:  fsdbSType = FSDB_ST_VCD_FORK; break;
	  case vpiFunction:   fsdbSType = FSDB_ST_VCD_FUNCTION; break;
	  case vpiModule:     fsdbSType = FSDB_ST_VCD_MODULE; break;
	  case vpiTask:       fsdbSType = FSDB_ST_VCD_TASK; break;

	  default:
	    vpi_printf("FSDB warning: $fsdbdumpvars: Unsupported argument "
	               "type (%s)\n", vpi_get_str(vpiType, item));
	    return;
      }

	/* Do some special processing/checking on array words. Dumping
	 * array words is an Icarus extension. */
      if (item_type == vpiMemoryWord) 
	  {
	      /* Turn a non-constant array word select into a constant
	       * word select. */
	    if (vpi_get(vpiConstantSelect, item) == 0) 
		{
		  vpiHandle array = vpi_handle(vpiParent, item);
		  PLI_INT32 idx = vpi_get(vpiIndex, item);
		  item = vpi_handle_by_index(array, idx);
	    }

	      /* An array word is implicitly escaped so look for an
	       * escaped identifier that this could conflict with. */
	      /* This does not work as expected since we always find at
	       * least the array word. We likely need a custom routine. */
       	if (vpi_get(vpiType, item) == vpiMemoryWord &&
                vpi_handle_by_name(vpi_get_str(vpiFullName, item), 0)) 
        {
		  	vpi_printf("FSDB warning: array word %s will conflict "
		             "with an escaped identifier.\n",
		             vpi_get_str(vpiFullName, item));
        }
      }

      fullname = vpi_get_str(vpiFullName, item);

	/* Generate the $var or $scope commands. */
      switch (item_type) {
	  case vpiParameter:
	    vpi_printf("FSDB sorry: $fsdbdumpvars: can not dump parameters.\n");
	    break;

	  case vpiNamedEvent:
	  case vpiIntegerVar:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiIntVar:
	  case vpiLongIntVar:
	  case vpiRealVar:
	  case vpiMemoryWord:
	  case vpiReg:
	  case vpiTimeVar:
	  case vpiNet:


	      /* If we are skipping all signal or this is in an automatic
	       * scope then just return. */
            if (skip || vpi_get(vpiAutomatic, item)) return;

	      /* Skip this signal if it has already been included.
	       * This can only happen for implicitly given signals. */
	    if (vcd_names_search(&fsdb_vcd_var, fullname)) return;

	      /* Declare the variable in the VCD file. */
	    name = vpi_get_str(vpiName, item);
	    prefix = is_escaped_id(name) ? "\\" : "";

	      /* Some signals can have an alias so handle that. */
	    nexus_id = vpi_get(_vpiNexusId, item);

		
	    ident = 0;
	    if (nexus_id) ident = fsdbfind_nexus_ident(nexus_id);

	    if (!ident) {
		  ident = strdup(fsdbvcdid);
		  gen_new_vcd_id();

		  if (nexus_id) fsdbset_nexus_ident(nexus_id, ident);

		  //Add a callback for the signal
		  info = malloc(sizeof(*info));

		  info->time.type = vpiSimTime;
		  info->item  = item;
		  info->ident = ident;
		  info->scheduled = 0;

		  cb.time      = &info->time;
		  cb.user_data = (char*)info;
		  cb.value     = NULL;
		  cb.obj       = item;
		  cb.reason    = cbValueChange;
		  cb.cb_rtn    = fsdbvariable_cb_1;

		  info->dmp_next = 0;
		  info->next  = fsdb_list;
		  fsdb_list    = info;

		  info->cb    = vpi_register_cb(&cb);
		
		}

	      /* Named events do not have a size, but other tools use
	       * a size of 1 and some viewers do not accept a width of
	       * zero so we will also use a width of one for events. */
	    if (item_type == vpiNamedEvent) size = 1;
	    else size = vpi_get(vpiSize, item);

		int lbitnum = 0, rbitnum = 0; 
	    if (size > 1 || vpi_get(vpiLeftRange, item) != 0) 
		{
			  lbitnum = (int)vpi_get(vpiLeftRange, item);
			  rbitnum = (int)vpi_get(vpiRightRange, item);
	    }

		ffwVarMapId vm_id = ffw_CreateVarByHandle(fsdbdump_file, fsdbVType, 
			FSDB_VD_INPUT, FSDB_DT_HANDLE_VERILOG_STANDARD, 
			lbitnum, rbitnum, (char*)ident,
                        (char*)name, FSDB_BYTES_PER_BIT_1B); //why use FSDB_BYTES_PER_BIT_1B,don't konw

		if(vm_id == NULL)
			vpi_printf("FSDB warning: failed "
			           "to create a var(%s).\n", fullname);

		/*
	    fprintf(dump_file, "$var %s %u %s %s%s",
		    type, size, ident, prefix, name);

	    // Add a range for vectored values.
	    if (size > 1 || vpi_get(vpiLeftRange, item) != 0) {
		  fprintf(dump_file, " [%i:%i]",
			  (int)vpi_get(vpiLeftRange, item),
			  (int)vpi_get(vpiRightRange, item));
	    }
	    fprintf(dump_file, " $end\n");
		*/	
	    break;

	  case vpiModule:		
	  case vpiGenScope:		
	  case vpiFunction:		
	  case vpiTask:			
	  case vpiNamedBegin:	
	  case vpiNamedFork:	

	    if (depth > 0) {
		/* list of types to iterate upon */
		  static int types[] = {
			/* Value */
			vpiNamedEvent,
			vpiNet,
			/* vpiParameter, */
			vpiReg,
			vpiVariables,
			/* Scope */
			vpiFunction,
			vpiGenScope,
			vpiModule,
			vpiNamedBegin,
			vpiNamedFork,
			vpiTask,
			-1
		  };
		  int i;
		  int nskip = (vcd_names_search(&fsdb_vcd_tab, fullname) != 0);

		    /* We have to always scan the scope because the
		     * depth could be different for this call. */
		  if (nskip) {
			vpi_printf("FSDB warning: ignoring signals in "
			           "previously scanned scope %s.\n", fullname);
		  } else {
			vcd_names_add(&fsdb_vcd_tab, fullname);
		  }

		  name = vpi_get_str(vpiName, item);

		  ffw_CreateScope(fsdbdump_file, fsdbSType, (char*)name);
		  //fprintf(fsdbdump_file, "$scope %s %s $end\n", type, name);

		  for (i=0; types[i]>0; i++) {
			vpiHandle hand;
			vpiHandle argv = vpi_iterate(types[i], item);
			while (argv && (hand = vpi_scan(argv))) {
			      fsdbscan_item(depth-1, hand, nskip);
			}
		  }

		    /* Sort any signals that we added above. */
		  ffw_CreateUpscope(fsdbdump_file);
		  //fprintf(dump_file, "$upscope $end\n");
	    }
	    break;
      }
}


//	vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
//	vpiHandle argv = vpi_iterate(vpiArgument, callh);
static int fsdbdumpvars(vpiHandle callh,vpiHandle argv)
{
	vpiHandle item;
	s_vpi_value value;
	unsigned depth = 0;

	/* Get the depth if it exists. */
	if (argv) 
	{
		value.format = vpiIntVal;
		vpi_get_value(vpi_scan(argv), &value);
		depth = value.value.integer;
	}
	if (!depth) depth = 10000;

	/* This dumps all the modules in the design if none are given. */
	if (!argv || !(item = vpi_scan(argv))) {
	argv = vpi_iterate(vpiModule, 0x0);
	assert(argv);  /* There must be at least one top level module. */
	item = vpi_scan(argv);
	}

	for ( ; item; item = vpi_scan(argv)) {
	char *scname;
	const char *fullname;
	int add_var = 0;
	int dep;
	PLI_INT32 item_type = vpi_get(vpiType, item);

	 /* If this is a signal make sure it has not already
	  * been included. */
	switch (item_type) 
	{
	   	case vpiIntegerVar:
		case vpiBitVar:
		case vpiByteVar:
		case vpiShortIntVar:
		case vpiIntVar:
		case vpiLongIntVar:
	   	case vpiMemoryWord:
	   	case vpiNamedEvent:
	   	case vpiNet:
	   	case vpiParameter:
	   	case vpiRealVar:
	   	case vpiReg:
	   	case vpiTimeVar:
	   /* Warn if the variables scope (which includes the
		* variable) or the variable itself was already
		* included. A scope does not automatically include
		* memory words so do not check the scope for them.	*/
		scname = strdup(vpi_get_str(vpiFullName,
								 vpi_handle(vpiScope, item)));
		fullname = vpi_get_str(vpiFullName, item);
		if (((item_type != vpiMemoryWord) &&
			  vcd_names_search(&fsdb_vcd_tab, scname)) ||
			 vcd_names_search(&fsdb_vcd_var, fullname)) 
		{
			   vpi_printf("VCD warning: skipping signal %s, "
						  "it was previously included.\n",
						  fullname);
			   free(scname);
			   continue;
		} 
		else 
		{
			   add_var = 1;
		}
		free(scname);
	}

	dep = fsdbdraw_scope(item, callh);

	fsdbscan_item(depth, item, 0);
	 /* The scope list must be sorted after we scan an item.  */
	vcd_names_sort(&fsdb_vcd_tab);

	while (dep--) ffw_CreateUpscope(fsdbdump_file);//fprintf(dump_file, "$upscope $end\n");

	 /* Add this signal to the variable list so we can verify it
	  * is not included twice. This must be done after it has
	  * been added */
	if (add_var) {
	 vcd_names_add(&fsdb_vcd_var, vpi_get_str(vpiFullName, item));
	 vcd_names_sort(&fsdb_vcd_var);
	}
	}

	return 0;
}


//TO DO 
__inline__ static int install_fsdbdumpvars_callback(void)
{

	struct t_cb_data cb;

	if (fsdbdumpvars_status == 1) return 0;

	if (fsdbdumpvars_status == 2) {
	vpi_printf("FSDB warning: $dumpvars ignored, previously"
	           " called at simtime %" PLI_UINT64_FMT "\n",
	           fsdbdumpvars_time);
	return 1;
	}

	cb.time = &fsdbzero_delay;
	cb.reason = cbReadOnlySynch;
	cb.cb_rtn = fsdbdumpvars_cb;
	cb.user_data = 0x0;
	cb.obj = 0x0;

	vpi_register_cb(&cb);

	cb.reason = cbEndOfSimulation;
	cb.cb_rtn = fsdbfinish_cb;

	vpi_register_cb(&cb);

	fsdbdumpvars_status = 1;
	return 0;
}

static PLI_INT32 sys_fsdbdumpvars_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
	
	vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
	vpiHandle argv = vpi_iterate(vpiArgument, callh);
	vpiHandle item;
	s_vpi_value value;
	unsigned depth = 0;

	(void)name; /* Parameter is not used. */

	//fopen fsdbdump_file
	if (fsdbdump_file == 0) 
	{
		if (fsdbdump_path == 0) fsdbdump_path = strdup("xepic.fsdb");

		fsdbdump_file = ffw_Open(fsdbdump_path, FSDB_FT_VERILOG);

		if (fsdbdump_file == 0) 
		{
			vpi_printf("fsdb Error: %s:%d: ", vpi_get_str(vpiFile, callh),
					   (int)vpi_get(vpiLineNo, callh));
			vpi_printf("Unable to open %s for output.\n", fsdbdump_path);
			vpi_control(vpiFinish, 1);
			free(fsdbdump_path);
			fsdbdump_path = 0;
			return 0;
		}
	}

	if (fsdbdump_file == 0) 
	{
	  if (argv) vpi_free_object(argv);
	  return 0;
	}

	if (install_fsdbdumpvars_callback()) 
	{
		if (argv) vpi_free_object(argv);
	    return 0;
    }

			
	//fsdb file fopen successfully,then write scope
    ffw_CreateTreeByHandleScheme(fsdbdump_file);
    ffw_SetScaleUnit(fsdbdump_file, "0.01n"); 

	ffw_BeginTree(fsdbdump_file);

	//dump scope
	fsdbdumpvars(callh,argv);

	ffw_EndTree(fsdbdump_file);

	return 0;
}


void sys_fsdb_register(void)
{
	s_vpi_systf_data tf_data;
	vpiHandle res;

	/* All the compiletf routines are located in vcd_priv.c. */
	tf_data.type      = vpiSysTask;
	tf_data.tfname    = "$fsdbdumpfile";
	tf_data.calltf    = sys_fsdbdumpfile_calltf;
	tf_data.compiletf = sys_one_string_arg_compiletf;
	tf_data.sizetf    = 0;
	tf_data.user_data = "$fsdbdumpfile";
	res = vpi_register_systf(&tf_data);
	vpip_make_systf_system_defined(res);

	tf_data.type	  = vpiSysTask;
	tf_data.tfname	  = "$fsdbdumpvars";
	tf_data.calltf	  = sys_fsdbdumpvars_calltf;
	tf_data.compiletf = sys_dumpvars_compiletf;
	tf_data.sizetf	  = 0;
	tf_data.user_data = "$fsdbdumpvars";
	res = vpi_register_systf(&tf_data);
	vpip_make_systf_system_defined(res);	  
}

#endif
