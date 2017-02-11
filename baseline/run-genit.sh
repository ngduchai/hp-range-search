#!/bin/bash
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 0 9999999 < /dev/null > larm/genit_00 2>> larm/genit_00 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 10000000 19999999 < /dev/null > larm/genit_10 2>> larm/genit_10 ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 20000000 29999999 < /dev/null > larm/genit_20 2>> larm/genit_20 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 30000000 39999999 < /dev/null > larm/genit_30 2>> larm/genit_30 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 40000000 49999999 < /dev/null > larm/genit_31 2>> larm/genit_31 ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 50000000 59999999 < /dev/null > larm/genit_21 2>> larm/genit_21 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 60000000 69999999 < /dev/null > larm/genit_11 2>> larm/genit_11 ) & )"'
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_genit 70000000 79999999 < /dev/null > larm/genit_01 2>> larm/genit_01 ) & )"'





