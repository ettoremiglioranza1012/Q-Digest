# MPI Implementation

The first attempt to parallelize the algorithm is implemented using the
Message Passing Interface (MPI). MPI is a common API used to develop
distributed-memory programs. The approach that will be attempted in the
current implementation will be SPMD-oriented. A Single Program Multiple
Data approach takes advantage of branching, as determined by the 
programmer, to write the main logic of the program inside a single source 
file. This means that to make a parallel program using this approach
the first problem that should be tackled is how to distribute the work
across the various processes (**load balancing**).

## Load balancing

The first approach is to use a load balancing strategy that equally 
divides the work across the various processes. So, given an input vector 
of size `n`, the single process reading that input and saving it into 
memory, will have to then send only the necessary portions (approximately
of size `n/p`) to all the other processes so that the workload is equally
distributed among all processes.

Each process will then be in charge of:

1. allocate memory for the vector.
2. receive the vector from the designated distributing process.
3. build a q-digest from the received data (`insert` + vector iteration)
4. compress the resulting q-digest (`compress`)

Now we face two alternatives:

- Either all processes send the resulting q-digest back to the distributing node
- or each process sends to a partner process its q-digest and the partner
process will then be in charge of merging the two q-digests and so on
until a final q-digest containing data from all processes is available 
back in the distributing process.

### Potential issues

It is still unclear how q-digests should be sent around processes.
Probably, there will be the need to serialize each q-digest and implement
a parser so that the message sent every time is a string that can be
parsed to rebuild a specific q-digest.

A possible alternative would be the creation of a *derived MPI Type*.
This would contain the necessary info to rebuild the q-digest struct.
