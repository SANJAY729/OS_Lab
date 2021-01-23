#!/bin/bash
gcd (){
	if [ $1 -gt $2 ] 
	then
		a=$1
		b=$2
	else
		a=$2
		b=$1
	fi
	r=`expr $(($a%$b))`
	if [ $r -eq 0 ]
	then
		return $b
	else
		gcd $b $r 
		return $?
	fi
}
string=$1
string="${string},"
if [[ $string =~ ^(-?[0-9]+,)+$ ]]
then
	i=0
	j=-1
	g=1
else
	echo "Invalid_input_format"
	exit	
fi
for (( k=0; i<${#string}; i++ )); do
  j=`expr $(($j+1))`
  if [ ${string:$i:1} = - ]
  	then 
  		j=-1
  		continue
  	fi
  if [ ${string:$i:1} = , ] 
  then 
  	num=${string:$(($i-$j)):$j}
  	var=`expr $(($num))`
  	if [ $k -eq 0 ] 
  	then
  		g=$var
  	else
  		gcd $var $g
  		g=$?
  	fi
  	k=`expr $(($k+1))`
  	j=-1
  fi
done
if [ $k -ge 10 ]
then 
	echo "Invalid_input _(more_than_9_numbers)"
	exit
fi
echo "GCD=$g"