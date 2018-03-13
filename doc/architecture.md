# DANS General Architecture

Here we talk about how the *DANS* library functions as a whole and can be extended in the future. NOTE that for many classes, there is an Abstract base class. This is done both for testing and also to ensure that new, different D-Stages and modules can be used in the future.

The overall goal is that an Application or a DStage Scheduler can dispatch Jobs to any DStage which can in turn forward the Job on to another without the knowledge or consent of the originator. 

## [Jobs](../common/dstage/job.h)

Jobs are the way that work is communicated to a D-Stage. All D-Stages accept unique pointers to Jobs as work input. It is common to convert unique pointers to Jobs to shared pointers temporarily as a duplication method.

### Jobs have a:

* *Job ID* which will be shared by any duplicates and is used to refer to the task across priority levels.
* *Priority* which denotes this specific duplicate job's priority.
* *Duplication* - deprecated
* Templated *Job Data* object which can be custom to every D-Stage. Job Data is used for job specific data.

### Notes about Jobs

* Note that the *Job Data* object is an excellent place to store thread safe, shared data as shared pointers or duplicate specific data otherwise.
* It has been common to store a shared pointer to a PurgeState (an object that tracks whether a job has been marked purged) as part of every Job Data type. It might make sense to add this to all Jobs by removing it from the *Job Data* object and storing it with the *Job ID* and *Priority,* but it is unclear if all D-Stages will use it as vitally.
* It is important that unique pointers to Jobs clean up any associated resource with that Job when they go out of scope. This is the case as it is the assumption of the unique pointer semantics. An example where this takes place is when Jobs are Purged from a Multiqueue as the Multiqueue has no idea how a specific Job type is removed and therefore relies on the Job's destructor.

## [BaseDStage](../common/dstage/basedstage.h)

The BaseDStage *abstract* class is a typed class. The generic type denotes the type of Job that this DStage accepts, which we will call the **Input Type**. The reason for this abstract class to exist is so that any DStage can have a reference to any other as long as it conforms to the correct Job Input Type, regardless of whether it follows the general form that the rest of the library uses.

## [DStage](../common/dstage/dstage.h)

The DStage class is derived class which adds a second type, the **Input Type**. This is confusing at first, but is important in that either another DStage or possibly an Application will be sending Jobs to a DStage. The receiving DStage will then duplicate and enqueue the Job. The duplication process might affect the way that an Input Job is represented. For example, an Input Job might be a list of IP addresses which might be mapped after duplication to multiple Input Jobs which each have a single IP address. A DStage is also not required to have the Input Type differ from the Input Type.

DStages contain a unique pointer of a BaseDispatcher, BaseMultiqueue and a BaseScheduler. The lifetime of the three is managed by the BStage. The only additional function of the DStage at this layer is to forward Dispatch requests to the BaseDispatcher and Purge requests to the BaseMultiQueue and the BaseScheduler.

## [Dispatcher](../common/dstage/dispatcher.h)

Dispatcher inherits from BaseDispatcher. It is responsible for linking a BaseMultiqueue for a dispatchers use. It also calculates the amount of duplication that a derived Dispatcher class might use. This second functionality it a flaw as it is a minimal calculation and does not add much value. It will likely be removed in the future. Derived sub

## [Multiqueue](../common/dstage/multiqueue.h)

The MultiQueue is a completely thread safe class which will allow queuing of any number of predefined Jobs. All operations, including Purge, are constant time amortized cost. Additionally, Dequeue is a blocking call which will block indefinitely until there is a job to Dequeue without spinning.

## [Scheduler](../common/dstage/scheduler.h)

The Scheduler inherits from BaseScheduler. It is responsible for linking a BaseMultiqueue and creating a pool of threads for each Priority level to schedule Jobs. It is up to derived classes of Scheduler to override the StartScheduling() in order to give those threads direction. It is also up to the derived class to release those threads from blocking calls when their destructor is called via calling MultiQueue.Release() or some of means.

