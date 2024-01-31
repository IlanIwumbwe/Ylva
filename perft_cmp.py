import chess
import chess.engine
from typing import Dict
import sys
import subprocess

def perft(board : chess.Board , depth : int) -> None:
    if(depth == 0):
        return 1
    
    nodes = 0

    for move in board.legal_moves:
        board.push(move)
        nodes += perft(board, depth-1)
        board.pop()

    return nodes

def stockfish_results(fen : str, depth : int, counts : Dict[str, int]) -> None:
    board = chess.Board(fen)
    legal_moves = list(board.legal_moves)

    engine = chess.engine.SimpleEngine.popen_uci("stockfish")   

    for move in legal_moves:
        board.push(move)
        pos = perft(board, depth-1)
        counts[move] = pos
        board.pop()

    engine.quit()

if __name__ == "__main__":
    if(len(sys.argv) > 1):
        fen = sys.argv[1]
        depth = int(sys.argv[2])

        counts = {}
        total = 0

        stockfish_results(fen, depth, counts)

        for move, count in counts.items():
            print(f"{move} : {count}")
            total += count

        print(f"nodes : {total}")
        