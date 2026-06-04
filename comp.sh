#!/bin/sh
im="$1"
om="$2"
sc="$3"
if [ ! -f "$im" ]; then
    echo "Input image '$im' not found."
    exit 1
fi
y=0
while [ $y -lt 2488 ]; do
    if [ $((y % (2488 / 100))) -eq 0 ]; then
        echo "Progress : $((y * 100 / 2488))%"
    fi
    bp=$((y * 2488))
    x=0
    pixel_data=$(xxd -p -s "$bp" -l 2488 "$im" | tr -d ' ' | tr -d '\n')
    while [ $x -lt 2488 ]; do
        pi=$((x * 2))
        echo -e -n "\x${pixel_data:pi:2}" >> "$om"
        x=$((x + sc))
    done
    y=$((y + sc))
done
