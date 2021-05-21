#!/usr/bin/env bash

#: ${CONF:=application}

#When a process is killed by a signal, it has exit code 128 + signum. (LINUX)
#Killed by SIGTERM (15) => 128 + 15 = 143.

app=kvbufrd
version=$(cat /usr/share/$app/VERSION)
set -e

export PGPASSFILE=/etc/kvalobs/.pgpass

: ${KV_LIBDIR:=/var/lib/kvalobs/kvbufrd}
: ${KV_RM_CACHE:=false}
dbname=${app}.sqlite
dbpath=${KV_LIBDIR}/${dbname}

app_PID=
got_exit_signal=false
running=true
trap _term SIGTERM SIGINT

_term() {
    echo "TERMINATING"
    running=false
    got_exit_signal=true    
}

#kill_pid pid [signal=SIGTERM [timeout=60]
kill_pid() {
    local pid sig timeout tuntil
    pid=$1
    sig=SIGTERM
    timeout=60
    
    shift
    [ $# -gt 0 ] && sig=$1; shift 
    [ $# -gt 0 ] && timeout=$1; shift 

    tuntil=$(($(date +'%s')+$timeout))
        
    if ! kill -$sig $pid &>/dev/null; then
      wait $pid
      return $?
    fi

    while [ $(date +'%s') -lt $tuntil ]; do
      if ! kill -0 $pid &>/dev/null; then
        wait $pid
        return $?
      fi
      sleep 1
    done

    kill -9 $pid &>/dev/null
    wait $pid &>/dev/null
    return $?
}

echo "ENTRYPOINT: NARGS: $# ARGS: '$@'"
echo "getent: $(getent passwd kvalobs)"
echo "id: $(id -u)"
echo "VERSION: $version"

if [ $# -gt 0 ]; then
   app=$1
   shift 
fi

echo "app: '$app'  $#"
echo "$version" > "/var/log/kvalobs/${app}_VERSION"
if [ "$app" != "bash" ]; then
  echo "ENTRYPOINT $app"
  echo "Starting $app."

  if [ "$KV_RM_CACHE" = true ]; then 
    echo "Removing old cache at startup '$dbpath'"
    rm -f $dbpath
  fi

  /usr/bin/${app}  2>&1 &
  app_PID=$!
  echo "${app} pid: $app_PID"
  wait -n
  ec=$?

  if [ $got_exit_signal = true ]; then
    echo "Got exit signal: $got_exit_signal"
    kill_pid $app_PID
    ec=$?
    echo "Killed $app on signal. Exit code: $ec"
    echo "VERSION: $version"
  else
    kill -0 $app_PID &>/dev/null || echo "kvbufrd: died  ec: $ec"
    echo "VERSION: $version"
  fi

  #It may be dead, but we call kill_pid for kvManagerd anyway. It will be killed
  #cleanly if it is not dead and we get the exit code if it is dead.

  kill_pid $app_PID  
  echo "app exit code: $?"
  echo "VERSION: $version"
  #return the exitcode for the process that died in the first place.
  exit $ec  
elif [ "$app" = "bash" ]; then
  echo "VERSION: $version"
  echo "ENTRYPOINT starting bash!"
  /bin/bash
else
  echo "VERSION: $version"
  echo "ENTRYPOINT sleep forever!"
  while running="true"; do 
    sleep 1; 
  done
fi
