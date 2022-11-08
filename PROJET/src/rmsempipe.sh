#!/bin/bash

DROITS=641
nb=`ipcs -s | grep $USER | grep $DROITS | wc -l`

if [ $nb -eq 0 ]
then
    echo "aucun sémaphore à détruire"
else
    pluriel=""
    if [ $nb -gt 1 ]; then pluriel="s"; fi
    echo "vous avez $nb sémaphore$pluriel non détruit$pluriel";

    for id in `ipcs -s | grep $USER | grep $DROITS | awk '{print $2;}'`
    do
        echo "  destruction sémaphore " $id
        ipcrm -s $id
    done

    nb=`ipcs -s | grep $USER | grep $DROITS | wc -l`
    pluriel=""
    if [ $nb -gt 1 ]; then pluriel="s"; fi
    echo "il reste $nb sémaphore$pluriel non détruit$pluriel";
fi


c2m="pipe_cl2ma"
m2c="pipe_ma2cl"

if [ ! -p $c2m ]
then
    echo "aucun tube à détruire"
else
    echo "tubes détruits"
    /bin/rm $c2m $m2c
fi
