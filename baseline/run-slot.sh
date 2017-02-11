#!/bin/bash
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/release-hugepage.sh ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/release-hugepage.sh ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/release-hugepage.sh ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/release-hugepage.sh ) & )"'

ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_00 2>> larm/slot_00 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_10 2>> larm/slot_10 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_20 2>> larm/slot_20 ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_30 2>> larm/slot_30 ) & )"'

ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_31 2>> larm/slot_31 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_21 2>> larm/slot_21 ) & )"'
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_11 2>> larm/slot_11 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_01 2>> larm/slot_01 ) & )"'

ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_02 2>> larm/slot_02 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_12 2>> larm/slot_12 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_22 2>> larm/slot_22 ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_32 2>> larm/slot_32 ) & )"'

ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_33 2>> larm/slot_33 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_23 2>> larm/slot_23 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_13 2>> larm/slot_13 ) & )"'
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_03 2>> larm/slot_03 ) & )"'

ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_04 2>> larm/slot_04 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_14 2>> larm/slot_14 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_24 2>> larm/slot_24 ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_34 2>> larm/slot_34 ) & )"'

ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_35 2>> larm/slot_35 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_25 2>> larm/slot_25 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_15 2>> larm/slot_15 ) & )"'
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_05 2>> larm/slot_05 ) & )"'

ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_06 2>> larm/slot_06 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_16 2>> larm/slot_16 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_26 2>> larm/slot_26 ) & )"'
ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_36 2>> larm/slot_36 ) & )"'

ssh node20 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_07 2>> larm/slot_07 ) & )"'
ssh node16 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_17 2>> larm/slot_17 ) & )"'
ssh node5 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_27 2>> larm/slot_27 ) & )"'
ssh node7 'sh -c "( ( nohup /home/ndhai/larm/tests/run_stress < /dev/null > larm/slot_37 2>> larm/slot_37 ) & )"'





