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

By default, the KKJ server will only utilize 1 joke everytime a client connects to it. However, if include *KKJ.txt* as an argument, you may use the jokes formatted in the file.

Add formatted knock knock jokes to *KKJ.txt* with 

```
SETUP
PUNCHLINE
```

*Here's an example:*

Server: "Knock, knock"

Client: "Who's there?"

Server: `SETUP`

Client: "`SETUP` who?"

Server: `PUNCHLINE`

Client: *insert response here*
