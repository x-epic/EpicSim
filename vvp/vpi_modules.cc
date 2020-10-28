/* Copyright (c) 2020 XEPIC Corporation Limited */
/*
 * Copyright (c) 2001-2019 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <cstdio>
#include <cstring>

#include "config.h"
#include "ivl_alloc.h"
#include "ivl_dlfcn.h"
#include "vpi_priv.h"
#include "vvp_cleanup.h"

static ivl_dll_t *dll_list = 0;
static unsigned dll_list_cnt = 0;

#if defined(__MINGW32__) || defined(__CYGWIN32__)
typedef PLI_UINT32 (*vpip_set_callback_t)(vpip_routines_s *, PLI_UINT32);
#endif
typedef void (*vlog_startup_routines_t)(void);

#define VPIP_MODULE_PATH_MAX 64

static const char *vpip_module_path[VPIP_MODULE_PATH_MAX] = {0};

static unsigned vpip_module_path_cnt = 0;

static bool disable_default_paths = false;

void vpip_clear_module_paths() {
  vpip_module_path_cnt = 0;
  vpip_module_path[0] = 0;
  disable_default_paths = true;
}

void vpip_add_module_path(const char *path) {
  if (vpip_module_path_cnt >= VPIP_MODULE_PATH_MAX) {
    fprintf(stderr, "Too many module paths specified\n");
    exit(1);
  }
  vpip_module_path[vpip_module_path_cnt++] = path;
}

void vpip_add_env_and_default_module_paths() {
  if (disable_default_paths) return;

  if (char *var = ::getenv("EPICSIM_VPI_MODULE_PATH")) {
    char *ptr = var;
    char *end = var + strlen(var);
    int len = 0;
    while (ptr <= end) {
      if (*ptr == 0 || *ptr == ':' || *ptr == ';') {
        if (len > 0) {
          vpip_add_module_path(strndup(var, len));
        }
        len = 0;
        var = ptr + 1;
      } else {
        len++;
      }
      ptr++;
    }
  }

#ifdef MODULE_DIR1
  vpip_add_module_path(MODULE_DIR1);
#endif
#ifdef MODULE_DIR2
  vpip_add_module_path(MODULE_DIR2);
#endif
}

void load_module_delete(void) {
  for (unsigned idx = 0; idx < dll_list_cnt; idx += 1) {
    ivl_dlclose(dll_list[idx]);
  }
  free(dll_list);
  dll_list = 0;
  dll_list_cnt = 0;
}

void vpip_load_module(const char *name) {
  struct stat sb;
  int rc;
  bool export_flag = false;
  char buf[4096];

  const char sep = '/';

  ivl_dll_t dll = 0;
  buf[0] = 0; /* terminate the string */
  if (strchr(name, sep)) {
    /* If the name has at least one directory character in
       it, then assume it is a complete name, maybe including any
       possible .vpi suffix. */
    export_flag = false;
    rc = stat(name, &sb);

    if (rc != 0) { /* did we find a file? */
                   /* no, try with a .vpi suffix too */
      export_flag = false;
      sprintf(buf, "%s.vpi", name);
      rc = stat(buf, &sb);

      /* Try also with the .vpl suffix. */
      if (rc != 0) {
        export_flag = true;
        sprintf(buf, "%s.vpl", name);
        rc = stat(buf, &sb);
      }

      if (rc != 0) {
        fprintf(stderr,
                "%s: Unable to find module file `%s' "
                "or `%s.vpi'.\n",
                name, name, buf);
        return;
      }
    } else {
      strcpy(buf, name); /* yes copy the name into the buffer */
    }

  } else {
    rc = -1;
    for (unsigned idx = 0; (rc != 0) && (idx < vpip_module_path_cnt);
         idx += 1) {
      export_flag = false;
      sprintf(buf, "%s%c%s.vpi", vpip_module_path[idx], sep, name);
      rc = stat(buf, &sb);

      if (rc != 0) {
        export_flag = true;
        sprintf(buf, "%s%c%s.vpl", vpip_module_path[idx], sep, name);
        rc = stat(buf, &sb);
      }
    }

    if (rc != 0) {
      fprintf(stderr,
              "%s: Unable to find a "
              "`%s.vpi' module on the search path.\n",
              name, name);
      return;
    }
  }

  /* must have found some file that could possibly be a vpi module
   * try to open it as a shared object.
   */
  dll = ivl_dlopen(buf, export_flag);
  if (dll == 0) {
    /* hmm, this failed, let the user know what has really gone wrong */
    fprintf(stderr,
            "%s:`%s' failed to open using dlopen() because:\n"
            "    %s.\n",
            name, buf, dlerror());

    return;
  }

  void *table = ivl_dlsym(dll, LU "vlog_startup_routines" TU);
  if (table == 0) {
    fprintf(stderr, "%s: no vlog_startup_routines\n", name);
    ivl_dlclose(dll);
    return;
  }

  /* Add the dll to the list so it can be closed when we are done. */
  dll_list_cnt += 1;
  dll_list = (ivl_dll_t *)realloc(dll_list, dll_list_cnt * sizeof(ivl_dll_t));
  dll_list[dll_list_cnt - 1] = dll;

  vpi_mode_flag = VPI_MODE_REGISTER;
  vlog_startup_routines_t *routines = (vlog_startup_routines_t *)table;
  for (unsigned tmp = 0; routines[tmp]; tmp += 1) (routines[tmp])();
  vpi_mode_flag = VPI_MODE_NONE;
}
