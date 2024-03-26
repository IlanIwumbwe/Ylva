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
        $executable -f "$fen" -m "pve";;
    2) 
        $executable -f "$fen" -m "pvp" ;;
    3) 
        $executable -f "$fen" -m "eve" ;; 
    4) 
        $executable -f "$fen" -m "perft" ;;
    *) 
        exit ;;  
esac



