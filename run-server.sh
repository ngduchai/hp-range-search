#!/bin/bash
ssh node21 'sh -c "( ( nohup /home/ndhai/larm/release-hugepage.sh ) & )"'
ssh node21 'sh -c "( ( nohup /home/ndhai/larm/tests/run_server 12345 < /dev/null > larm/server_00 2>> larm/server_00 ) & )"'
ssh node21 'sh -c "( ( nohup /home/ndhai/larm/tests/run_server 30289 < /dev/null > larm/server_01 2>> larm/server_01 ) & )"'
ssh node23 'sh -c "( ( nohup /home/ndhai/larm/release-hugepage.sh ) & )"'
ssh node23 'sh -c "( ( nohup /home/ndhai/larm/tests/run_server 12345 < /dev/null > larm/server_10 2>> larm/server_10 ) & )"'
ssh node23 'sh -c "( ( nohup /home/ndhai/larm/tests/run_server 30289 < /dev/null > larm/server_11 2>> larm/server_11 ) & )"'




