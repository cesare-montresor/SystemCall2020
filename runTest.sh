#! /bin/bash

max_msg=100
min_msg_delay=2
max_msg_delay=7
rng_msg_delay=$(($max_msg_delay-min_msg_delay))

max_distance=10

msg_queue_key=42 


make client
clear

devices_txt=$(cat output/server.run)
devices=($devices_txt)
num_devices=${#devices[@]}
#echo num devices $num_devices

for (( msg_i=1; msg_i<=$max_msg ; msg_i++ ))
do
  #Pick PID
  rnd_idx=$(( $RANDOM % $num_devices ))
  rnd_dev=${devices[$rnd_idx]}
  
  #Pick max distance
  dist=$(( ( $RANDOM % $max_distance) + 1 ))

  #Run Client 
  echo -----------------------------------------------------
  
  echo ./client $msg_queue_key $rnd_dev $msg_i \"msg_number_${msg_i}\" $dist 
  ./client $msg_queue_key $rnd_dev $msg_i "msg_number_${msg_i}" $dist 

  #Wait
  delay=$(( ($RANDOM % $rng_msg_delay) + $min_msg_delay ))
  echo Sleep for $delay sec
  sleep $delay
  echo
done

echo $idx 