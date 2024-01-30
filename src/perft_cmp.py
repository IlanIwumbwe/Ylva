import chess
import chess.engine
from typing import Dict

def perft(board : chess.Board , depth : int) -> None:
    if(depth == 0):
        return 1
    
    nodes = 0

    for move in board.legal_moves:
        board.push(move)
        nodes += perft(board, depth-1)
        board.pop()

    return nodes

def perft_results(fen : str, depth : int, counts : Dict[str, int]) -> None:
    board = chess.Board(fen)
    legal_moves = list(board.legal_moves)

    engine = chess.engine.SimpleEngine.popen_uci("stockfish")   

    for move in legal_moves:
        board.push(move)
        pos = perft(board, depth-1)
        counts[move] = pos
        board.pop()

    engine.quit()

counts = {}
total = 0

perft_results("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, counts)

for move, count in counts.items():
    print(f"{move} : {count}")
    total += count

print(f"nodes : {total}")