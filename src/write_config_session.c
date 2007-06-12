/* Copyright (C) 2006 Thorsten Kukuk
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
#include <string.h>
#include <sys/types.h>

#include "pam-config.h"

int
write_config_session (const char *file, config_file_t *conf)
{
  FILE *fp;

  if (debug)
    printf ("*** write_config_session (%s, ...)\n", file);

  /* XXX rename () */

  fp = fopen(file, "w");
  if (fp == NULL)
    {
      fprintf (stderr, _("Cannot create %s: %m\n"),
	       file);
      return -1;
    }

  fprintf (fp, "#%%PAM-1.0\n#\n");
  fprintf (fp, "# This file is autogenerated by pam-config. All changes\n");
  fprintf (fp, "# will be overwritten.\n#\n");
  fprintf (fp, "# Session-related modules common to all services\n#\n");
  fprintf (fp,
	   "# This file is included from other service-specific PAM config files,\n"
	   "# and should contain a list of modules that define tasks to be performed\n"
	   "# at the start and end of sessions of *any* kind (both interactive and\n"
	   "# non-interactive\n#\n");

  if (conf->use_mkhomedir)
    fprintf (fp, "session\trequired\tpam_mkhomedir.so\n");

  if (conf->use_limits)
    fprintf (fp, "session\trequired\tpam_limits.so\n");

  fprintf (fp, "session\trequired\tpam_unix2.so\t");
  if (conf->unix2_nullok)
    fprintf (fp, "nullok ");
  if (conf->unix2_debug)
    fprintf (fp, "debug ");
  if (conf->unix2_trace)
    fprintf (fp, "trace ");
  fprintf (fp, "\n");

  if (conf->use_apparmor)
    fprintf (fp, "session\trequired\tpam_apparmor.so\n");

  if (conf->use_krb5)
    {
      fprintf (fp, "session\toptional\tpam_krb5.so\t");
      if (conf->krb5_debug)
	fprintf (fp, "debug ");
      if (conf->krb5_minuid)
	fprintf (fp, "minimum_uid=%u ", conf->krb5_minuid);
      fprintf (fp, "\n");
    }

  if (conf->use_ldap)
    {
      fprintf (fp, "session\toptional\tpam_ldap.so\t");
      if (conf->ldap_debug)
	fprintf (fp, "debug ");
      fprintf (fp, "\n");
    }

  if (conf->use_winbind)
    {
      fprintf (fp, "session\trequired\tpam_winbind.so\t");
      if (conf->winbind_debug)
	fprintf (fp, "debug ");
      fprintf (fp, "\n");
    }

  if (conf->use_lum)
    fprintf (fp, "session\toptional\tpam_nam.so\n");

  if (conf->use_env)
    fprintf (fp, "session\toptional\tpam_env.so\n");

  if (conf->use_xauth)
    fprintf (fp, "session\toptional\tpam_xauth.so\n");

  if (conf->use_umask)
    fprintf (fp, "session\toptional\tpam_umask.so\n");

  fclose (fp);

  return 0;
}
