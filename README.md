# Modulo

Modulo is a productivity app designed to bridge the gap between today's thoughts and tomorrow's actions.  
It's like a personal messaging system for sending thoughts to the future!

In particular, Modulo syncs to your sleep schedule so you can
1. Send messages to tomorrow
2. Read messages from yesterday.


## Tomorrow
Run the `modulo tomorrow` command to start writing your thoughts for tomorrow. 
This will launch an interactive editor.

![Tomorrow Demo](./img/tomorrow-demo.gif)

## Today
The next day you can run the following sequence to review these entries:

`modulo wakeup`  
`modulo today`

The `modulo wakeup` command tells Modulo that you're ready to start a new day. 
You can optionally omit this command if you run `modulo today` later than your specified 'wakeup_latest' time 
(configured in preferences). 

## Building and Installation Guide

The following should work on unix systems (linux, macOS, BSD)

Modulo uses linked dependencies `ncurses` and `cJSON`. 
Ensure these are installed. Then from the project's root directory run:

```bash
make
make install
```

You will probably need `sudo make install` to avoid permissions issues.

On success, this will build and install the project to /usr/local/bin/.



## Motivation
Going to sleep after a day of lack-luster productivity is pretty difficult. 
There's a need to keep going and finally push through on that last thing..
But we all have limited capacity and sleep is important. 

If only there was a way to channel the motivation at the end of one day into 
the infinite potential of the next. Like a modulo operator
that could take the desire and energy at the last index of the day and map it 
into the 0th index of the next.. yeah that would be great.

Ok that sounds epic but honestly this is just a neat little logging app
that lets you plan tasks for the next day! It's written in c and gives you a 
interactive cli to add new tasks for the next day.
