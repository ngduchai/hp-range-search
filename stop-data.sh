#!/bin/bash
ssh node7 'sh -c "( ( nohup pkill -9 run_genit ) & )"'
ssh node5 'sh -c "( ( nohup pkill -9 run_genit ) & )"'
ssh node20 'sh -c "( ( nohup pkill -9 run_genit ) & )"'
ssh node16 'sh -c "( ( nohup pkill -9 run_genit ) & )"'

ssh node21 'sh -c "( ( nohup pkill -9 run_server ) & )"'
ssh node23 'sh -c "( ( nohup pkill -9 run_server ) & )"'


ssh node16 'sh -c "( ( nohup pkill -9 run_stress ) & )"'
ssh node20 'sh -c "( ( nohup pkill -9 run_stress ) & )"'
ssh node5 'sh -c "( ( nohup pkill -9 run_stress ) & )"'
ssh node7 'sh -c "( ( nohup pkill -9 run_stress ) & )"'

rm slot_*
rm server_*
rm genit_*




