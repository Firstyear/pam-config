/* Copyright (C) 2006, 2007, 2019 Thorsten Kukuk
   Author: Thorsten Kukuk <kukuk@thkukuk.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pam-config.h"

#define DEF_MODE 0644

int
write_config (const char *sysconfdir, const char *file, write_type_t op, pam_module_t **module_list)
{
  const char *opc = type2string (op);
  struct stat f_stat;
  char *tmpfname;
  FILE *fp;
  int fd;
  int result = 0;
  /* defaults for uid, gid and mode */
  uid_t user_id = getuid();
  gid_t group_id = getgid();
  mode_t mode = DEF_MODE;
  char *config = NULL;
  char *pamddir = NULL;

  if (debug)
    printf ("*** write_config (%s, %s/pam.d/%s, ...)\n", opc, sysconfdir, file);

  if (asprintf (&pamddir, "%s/pam.d", sysconfdir) < 0)
    return -1;

  if (access(pamddir, F_OK) < 0)
    {
      if (mkdir (pamddir, 0755) < 0)
	{
	  fprintf (stderr, _("Cannot create %s: %m\n"), pamddir);
	  free (pamddir);
	  return -1;
	}
    }
  free (pamddir);

  if (asprintf (&config, "%s/pam.d/%s", sysconfdir, file) < 0)
    return -1;

  if (asprintf (&tmpfname, "%s.XXXXXX", config) < 0)
    return -1;

  if (stat (config, &f_stat) == 0)
    {
      user_id = f_stat.st_uid;
      group_id = f_stat.st_gid;
      mode = f_stat.st_mode;
    }

  fd = mkstemp (tmpfname);
  if (fchmod (fd, mode) < 0)
    {
      fprintf (stderr, _("Cannot set permissions for '%s': %m\n"),
               tmpfname);
      return 1;
    }
  if (fchown (fd, user_id, group_id) < 0)
    {
      fprintf (stderr,
               _("Cannot change owner/group for `%s': %m\n"),
               tmpfname);
      return 1;
    }

  fp = fdopen(fd, "w");
  if (fp == NULL)
    {
      fprintf (stderr, _("Cannot create %s: %m\n"),
	       config);
      return -1;
    }

  fprintf (fp, "#%%PAM-1.0\n#\n");
  fprintf (fp, "# This file is autogenerated by pam-config. All manual\n");
  fprintf (fp, "# changes will be overwritten!\n#\n");
  fprintf (fp,
	   "# The pam-config configuration files can be used as template\n"
	   "# for an own PAM configuration not managed by pam-config:\n"
	   "#\n"
	   "# for i in account auth password session session-nonlogin; do \\\n"
	   "#      rm -f common-$i; sed '/^#.*/d' common-$i-pc > common-$i; \\\n"
	   "# done\n"
	   "# for i in account auth password session; do \\\n"
	   "#      rm -f postlogin-$i; sed '/^#.*/d' postlogin-$i-pc > postlogin-$i; \\\n"
	   "# done\n#\n"
	   "# Afterwards common-{account, auth, password, session, session-nonlogin}\n"
	   "# and postlogin-{account, auth, password, session} can be\n"
	   "# adjusted. Never edit or delete common-*-pc or postlogin-*-pc files!\n#\n"
	   "# WARNING: changes done by pam-config afterwards are not\n"
	   "# visible to the PAM stack anymore!\n"
	   "#\n"
	   "# WARNING: self managed PAM configuration files are not supported,\n"
	   "# will not see required adjustments by pam-config and can become\n"
	   "# insecure or break system functionality through system updates!\n#\n#\n");
  switch (op) {
  case ACCOUNT:
    fprintf (fp, "# Account-related modules common to all or login services\n#\n");
    fprintf (fp,
	     "# This file is included from other service-specific PAM config files,\n"
	     "# and should contain a list of the account modules that define\n"
	     "# the central access policy for use on the system.\n#\n");

    break;
  case AUTH:
    fprintf (fp, "# Authentication-related modules common to all or login services\n#\n");
    fprintf (fp,
	     "# This file is included from other service-specific PAM config files,\n"
	     "# and should contain a list of the authentication modules that define\n"
	     "# the central authentication scheme for use on the system\n"
	     "# (e.g., /etc/shadow, LDAP, Kerberos, etc.). The default is to use the\n"
	     "# traditional Unix authentication mechanisms.\n#\n");
    break;
  case PASSWORD:
    fprintf (fp, "# Password-related modules common to all or login services\n#\n");
    fprintf (fp,
	     "# This file is included from other service-specific PAM config files,\n"
	     "# and should contain a list of modules that define  the services to be\n"
	     "# used to change user passwords.\n#\n");
    break;
  case SESSION:
    fprintf (fp, "# Session-related modules common to all or login services\n#\n");
    fprintf (fp,
	     "# This file is included from other service-specific PAM config files,\n"
	     "# and should contain a list of modules that define tasks to be performed\n"
	     "# at the start and end of sessions of *any* kind (both interactive and\n"
	     "# non-interactive\n#\n");
    break;
  }

  pam_module_t **modptr = module_list;

  while (*modptr != NULL)
    {
      result |= (*modptr)->write_config (*modptr, op, fp);
      ++modptr;
    }

  fclose (fp);

  rename (tmpfname, config);
  free (tmpfname);

  return result;
}
