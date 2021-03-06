/*
 * Copyright (c) 2017 Balabit
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include <syslog-ng.h>
#include <logmsg/logmsg.h>
#include <template_lib.h>
#include <apphook.h>
#include <plugin.h>

#include "cfg.h"

MsgFormatOptions parse_options;

void
stardate_assert(const gchar *msg_str, const int precision, const gchar *expected)
{

  LogMessage *logmsg = log_msg_new(msg_str, strlen(msg_str), NULL, &parse_options);

  char *template_command;
  if (precision == -1)
    asprintf(&template_command, "$(stardate $UNIXTIME)");
  else
    asprintf(&template_command, "$(stardate --digits %d $UNIXTIME)", precision);
  assert_template_format_msg(template_command, expected, logmsg);
  free(template_command);

  log_msg_unref(logmsg);
}

void
test_stardate(void)
{
  stardate_assert("2012-07-15T00:00:00", 1, "2012.5"); // 2012.01.01 + 365/2 day
  stardate_assert("2013-07-01T00:00:00", 2, "2013.49");

  stardate_assert("2014-01-01T00:00:00", 3, "2014.000");
  stardate_assert("2015-12-31T23:59:59", 3, "2015.999"); // No rounding up!
  stardate_assert("2016-12-31T23:59:59", 7, "2016.9999999"); // No rounding up!

  stardate_assert("2017-01-01T00:00:00", 0, "2017");
  stardate_assert("2018-12-01T00:00:00", 0, "2018"); // No rounding up!
}

int
main(int argc G_GNUC_UNUSED, char *argv[] G_GNUC_UNUSED)
{
  app_startup();
  init_template_tests();
  cfg_load_module(configuration, "stardate");

  test_stardate();

  deinit_template_tests();
  app_shutdown();
  return 0;
}
