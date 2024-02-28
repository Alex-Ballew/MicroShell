# MicroShell

A project from my Systems II class while at Western Washington Univeristy. This project is a fully workable Linux shell, commands I made are ls, ls -l, cp,env, env [Name=Value], exit, cd [directory], pwd. All other Linux commands are available and implement the fork-exec-wait pattern to run processes, use exec provided by the C library. 

## How to run/Examples ##

<b> Compile with: </b>
<pre> make </pre>

<b> To run: </b>
<pre> ./myshell </pre>

<b> Example with ls -l command </b>

<pre>% ls -l
total: 87
-rw-r--r-- 1 ballewa2 grp.csci.students  8650 Feb 25 15:06 builtin.c
-rwxr-xr-x 1 ballewa2 grp.csci.students   326 Feb 13 13:55 argparse.h
-rw-r--r-- 1 ballewa2 grp.csci.students   354 Feb 25 15:43 Makefile
-rwxr-xr-x 1 ballewa2 grp.csci.students 33880 Feb 25 15:54 myshell
-rwxr-xr-x 1 ballewa2 grp.csci.students  1738 Feb 25 15:51 argparse.c
-rwxr-xr-x 1 ballewa2 grp.csci.students   314 Feb 13 13:55 builtin.h
-rw-r--r-- 1 ballewa2 grp.csci.students  4125 Feb 25 15:19 myshell.c
</pre>

