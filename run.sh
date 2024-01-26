#!/bin/bash

menu(){  
    echo "    1. Play against engine"
    echo "    2. Play against friend"
    echo "    3. Engine vs Engine"
    echo "    4. Run Perft" 

    read -p "    >> " choice
}

ask_fen(){
    echo "    Load FEN "
    read -p "    >> " fen
}

echo "====================================="
echo "                 YLVA                "
echo -e "=====================================\n"   
ask_fen
menu


case $choice in
    1) ./chess -f "$fen" -m "pve" ;;
    2) ./chess -f "$fen" -m "pvp" ;;
    3) ./chess -f "$fen" -m "eve" ;; 
    4) ./chess -f "$fen" -m "perft" ;;
    *) exit ;; 
esac



