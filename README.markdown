# Switched Reluctance Motor Control

 This simple library allows control of switch reluctance motors such as those found in hard disks. It provides varying degrees of precision assuming three phases.
 
 Library also supports rudimentary speed control which allows programmers to control motor speed without resorting to usage of `delay`. This is done through setting speed measured in revolutions per minute and calling `motor_step` or `motor_step_backward` frequently. State of pins will be updated only after specific time has passed. Speed control does take into account sequence sizes, so no additional calculation is needed.
 

 **Please note that:**
 
 - You should **not** connect motor contacts directly to your micro-controller as they use much higher current than what your controller can provide and will most likely result in a damaged hardware. Instead you should come up with indirect solution such as one with transistors or optocoupler;
 - You can have multiple different sequences using the same pins. However be careful when switching between the two while motor is running as you might cause motor to stall. Ideally you should synchronize when `position == 1`;
 - Avoid setting `position` manually (and pretty much anything else in config). It's not a typical counter. Library is stepping through sequence by shifting bits to the left or right;
 - Calling `motor_release` will stop applying any voltage to configured pins but phase will continue to cycle. Calling `motor_resume` again will resume applying voltage to pins. This is useful if you wish to spin up the motor and let it run on its own for a while.


Simple example without speed control.

```arduino

Sequence *motor;

void setup()
{
	motor = motor_initialize(2, 3, 4, SEQUENCE_SIMPLE);
}

void loop()
{
	motor_step(motor);
	delay(200);
}

```


The following example tries to keep motor spinning at 1000 revolutions per minute. Advantage of speed control is that it allows you to use cycles for something else other than waiting for `delay` to return.

```arduino

Sequence *motor;

void setup()
{
	motor = motor_initialize(2, 3, 4, SEQUENCE_SIMPLE);
	motor_set_speed(motor, 1000);
	motor_set_speed_control(motor, true);
}

void loop()
{
	motor_step(motor);
}

```
