# Modulo

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

## Usage:

modulo is a minimal productivity app designed for continuity! 
It allows you to offload end-of-day thoughts, motivations, and goals onto tomorrows to-do list.

Run the `modulo tomorrow` command and you will be prompted with an interactive loop to add new entries. 
Use the % character as a delimiter between entries. Type done when you're finished!

These entries will then be available tomorrow via the `modulo today` command. 
modulo defines tomorrow using a wakeup time that you can configure.

modulo assumes a default wakeup time of 9am but this can be configured
using either `modulo set wakeup` or `modulo init` (see below).

You can also run `modulo peek` to view tomorrows entries today.

If desired use `modulo remove 1` to remove entry 1 from tomorrows list.

Todays entries will be available for reflection and review until wakeup time tomorrow! 

Commands

modulo init:
    Sets username (Your name) and wakeup time parameters for the app. 

modulo tomorrow

modulo today

modulo set wakeup [8am, 1pm, 7:00]

modulo set name
