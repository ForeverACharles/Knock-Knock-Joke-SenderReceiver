# Knock Knock Joke Client & Server
Communicate client side to a Knock Knock Joke server! written in C

## Usage on Linux

**On server terminal**

Compile *server.c* with gcc `make` and run the KKJserver.o executable.
Specfiy the port number to open for the server by including the port as an argument.

**On client terminal**

Compile *client.c* with gcc and run `a.out` executable. 
Specfiy the port number to open for the client by including the port as an argument.

## Playing with KKJs

By default, the KKJ server will only use the joke everytime a client connects to it. Including *KKJ.txt* as an argument allows you to use any jokes formatted in the *KKJ.txt* file.

You can add your own knock knock jokes to *KKJ.txt* by following the format below:

```
SETUP(for joke 1)
PUNCHLINE(for joke 1)

SETUP(for joke 2)
PUNCHLINE(for joke 2)
.
.
.
SETUP(for joke n)
PUNCHLINE(for joke n)
```

**Example client-server exchange:**
> Server: "Knock, knock"  
Client: "Who's there?"  
Server: `SETUP`  
Client: "`SETUP` who?"  
Server: `PUNCHLINE`  
Client: *insert response here*

The KKJ server will send an error messsage and terminate connection if the client responds with anything that does not conform to the KKJ format above


