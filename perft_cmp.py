import chess
import chess.engine
from typing import Dict
import sys
import subprocess

STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

def perft(board : chess.Board , depth : int) -> None:
    if(depth == 0):
        return 1
    
    nodes = 0

    for move in board.legal_moves:
        board.push(move)
        nodes += perft(board, depth-1)
        board.pop()

    return nodes

def my_results(fen:str, depth:int):
    try:
        engine = subprocess.Popen(
            ["./chess", f'-f "{fen}"', '-m "benchmark"', f'-d "{str(depth)}"'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )   

        output, errors = engine.communicate()

        if(errors):
            print("Errors: ", errors)
        
        if(output):
            return sorted(list(engine.stdout))
        
    except Exception as e:
        print("Error: ", e)
        return []

def stockfish_results(fen : str, depth : int, counts : Dict[str, int]) -> None:
    board = chess.Board(fen)
    legal_moves = list(board.legal_moves)

    engine = chess.engine.SimpleEngine.popen_uci("stockfish")   

    for move in legal_moves:
        board.push(move)
        pos = perft(board, depth-1)
        counts[str(move)] = pos
        board.pop()

    engine.quit()

if __name__ == "__main__":
    if(len(sys.argv) > 1):
        fen = sys.argv[1]
        depth = int(sys.argv[2])

        counts : Dict[str,int] = {}
        total = 0
        p = 2

        stockfish_results(fen, depth, counts)
        
        res = my_results(fen, depth)

        print(res)

        for move, count in sorted(counts.items(), key=lambda pair : pair[0]):
            print(move, count)

            p+=1
            total += count
        


