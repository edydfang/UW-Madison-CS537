#!/bin/bash
for f in images/*.img
do
  echo $f
  ./fscheck $f
done
