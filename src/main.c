/*
 * Copyright 2020 Liviu Nicolescu
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"

#include "mgos_pppos.h"

static void timer_cb(void *arg) {
  time_t t = time(0);

  struct tm local_time_tm;
  localtime_r(&t, &local_time_tm);
  char local_time[32];
  strftime(local_time, sizeof(local_time), "%F %T %Z", &local_time_tm);

  struct tm utc_time_tm;
  gmtime_r(&t, &utc_time_tm);
  char utc_time[32];
  strftime(utc_time, sizeof(utc_time), "%F %T Z", &utc_time_tm);

  const char *tz = mgos_sys_config_get_sys_tz_spec();
  LOG(LL_INFO, ("tz: %s, epoch: %ld, local: %s, utc: %s",
                (tz != NULL ? tz : ""), t, local_time, utc_time));
}

static void modem_power_on_timer_cb(void *arg) {
  int power_on = (int) arg;
  mgos_gpio_setup_output(power_on, true);

  mgos_pppos_connect(0);
}

static void modem_start(void) {
  int power_ctrl = mgos_sys_config_get_modem_power_ctrl();
  if (power_ctrl > 0) {
    mgos_gpio_setup_output(power_ctrl, true);
    LOG(LL_INFO, ("%s - set power_ctrl on", __FUNCTION__));
    int power_on = mgos_sys_config_get_modem_power_on_pin();
    mgos_gpio_setup_output(power_on, false);
    mgos_set_timer(mgos_sys_config_get_modem_power_on_interval(), 0,
                   modem_power_on_timer_cb, (void *) power_on);
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  modem_start();

  mgos_set_timer(10000 /* ms */, MGOS_TIMER_REPEAT, timer_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}
