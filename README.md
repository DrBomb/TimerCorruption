# Timer corruption issue

## Description

Setting the "wall time" on the same function call as a timer set to MGOS_TIMER_RUN_NOW corrupts the timer and breaks all timer invokations.

We have a requiement to as much as possible, not lose the system clock, for that we set a reboot handler that writes the current wall time on reboot using an event handler.

On boot, we set a timer with the MGOS_TIMER_REPEAT and MGOS_TIMER_RUN_NOW flags, then we set the wall time with `mgos_settimeofday`, this sets the timer's `next_invocation` to a negative number, breaking the timer and any other timer, including cron jobs!

## App Config Schema

- save_time: Enable saving the wall time on reboot
- long_timer: Enabling setting the long timer that runs instantly BEFORE setting any saved wall time.
- saved_time: Saved wall time

## Reproduction steps

* Build and flash the app, run `mos console`.
* On first run a "Long Timer" log should show up, which is the timer set to run instantly. It should also show up after 60 seconds.
* Every 5 seconds a second timer should trigger logging the text "Timer" on console as well'
* With the console running paste the RPC command for a reboot. `{"method": "Sys.Reboot", "id": 123}`, it should reboot the board, if not just call `mos call Sys.Reboot && mos console` to send the command and continue monitoring the console.
* After the reboot the "Long Timer" message will not show up on boot, nor the "Timer" message that should show up every 5 seconds. No other timer will work again on this board. Not even a `Sys.Reboot` RPC command will work because the board sets a 100 ms timer before actually doing the reboot.
* To fix, a hardware reboot is needed.

## Visualization

Apply the patch file `mgos_timers.c.patch` to the `mgos_timers.c` file. With a local build the command should be `patch deps/modules/mongoose-os/src/mgos_timers.c mgos_timers.c.patch`

This patch adds a few logs when setting, clearing and when the `mgos_time_change_cb` is triggered by changing the wall time.

Here's a sample of the failed state:

```
[Jan  8 17:01:36.089] mgos_timers.c:130       Setting timer with id 1073508848
[Jan  8 17:01:36.095] mgos_timers.c:193       Updated timer 1073508848 to invocation -1.812651
[Jan  8 17:01:36.101] main.c:33               Wall time set to 5 from config
[Jan  8 17:01:36.331] mgos_sys_config.c:174   Saved to conf9.json
[Jan  8 17:01:36.334] mgos_timers.c:130       Setting timer with id 1073511848
[Jan  8 17:01:36.341] mgos_init.c:36          Init done, RAM: 287336 total, 227684 free, 227020 min free
[Jan  8 17:01:37.349] mgos_mongoose.c:66      New heap free LWM: 225896
```