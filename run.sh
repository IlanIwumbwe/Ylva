#!/bin/bash

executable="./chess"

if [ -x "$(command -v $executable)" ]; then
    echo "Executable '$executable' exists"
else
    echo "Executable '$executable' will be created by running make"
    make 
fi

menu(){  
    echo "    1. Play against engine"
    echo "    2. Play against friend"
    echo "    3. Engine vs Engine"
    echo "    4. Run Perft" 
    echo "    5. UCI test (Under Development)" 

    read -p "    >> " choice
}

ask_fen(){
    echo "    Load FEN (starting fen by defualt)"
    read -p "    >> " fen
}

echo "====================================="
echo "                 YLVA                "
echo -e "=====================================\n"   

menu

case $choice in
    1) 
        $executable -f "$fen" -m "pve";;
    2)  ask_fen
        $executable -f "$fen" -m "pvp" ;;
    3) 
        ask_fen 
        $executable -f "$fen" -m "eve" ;;
    4) 
        ask_fen
        $executable -f "$fen" -m "perft" ;;
    5) 
        $executable -f "$fen" -m "uci" ;;
    *) 
        exit ;;  
esac



