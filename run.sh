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

menu
ask_fen

case $choice in
    1) 
        ./chess -f "$fen" -m "pve"
        ;;
    2) ./chess -f "$fen" -m "pvp" ;;
    3)  
        ./chess -f "$fen" -m "eve"
        ;; 
    4) ./chess -f "$fen" -m "perft" ;;
    5)  
        read -p "    depth: " d

        if [ -z "$fen" ]; then
            python3 perft_cmp.py "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" $d  
        else 
            python3 perft_cmp.py "$fen" $d 
        fi   
        ;;

    *) exit ;;  
esac



