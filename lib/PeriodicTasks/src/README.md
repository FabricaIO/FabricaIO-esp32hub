# Periodic Tasks
> [!IMPORTANT]
> The below are important notes regarding the proper setting of the periodic task period

The periodic task period *must* be at least last as long as all the tasks take to run. For example, if all the tasks take a combined 30 seconds to run, the period must be at least 30 seconds (30000 ms) long.

It's best to set the task period to no longer than a few minutes. If there are tasks that should run less frequently, set the device's individual `taskPeriod` to the longer interval but leave the global task period interval at the lower value. The global task period value may be hard coded in the future.