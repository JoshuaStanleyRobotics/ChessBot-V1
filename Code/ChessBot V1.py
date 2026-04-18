import serial
ser = serial.Serial('COM22', 115200, timeout=None)

import chess
import chess.engine
engine = chess.engine.SimpleEngine.popen_uci("C:\Program Files (x86)\Stockfish\stockfish-windows-x86-64-avx2.exe")
engine.configure({"Threads":4, "Hash":16, "Skill Level":20})

board = chess.Board()
limit = chess.engine.Limit(time=.1)

try:
    while board.is_checkmate() != True:                                                 # gameplay loop to repeat until the game is over
        move = ser.readline().decode('utf-8').rstrip()                                  # read in move from the microcontroller
        if move:                                                                        # if a move was received 
            print(f"Received: {move}")                                                  # print the recieved move in the command line
            
            try:
                move = chess.Move.from_uci(move)                                        # convert the UCI string to a move object
            except chess.InvalidMoveError:
                print(f"'{move}' is an invalid UCI format string.")                     # if the move is invalid 
                response = f"{move}\n"
                ser.write(response.encode('utf-8'))
                print(f"Sent: {move}")
                move = None

            if move is not None and move in board.legal_moves:                          # check if the move is in the list of legal moves                                
                board.push(move)                                                        # if legal, make the move on the board
                print(board)
                print('\n')

                result = engine.play(board, limit)                                      # get the best response
                best_move = result.move

                board.push(best_move)                                                   # update the response on the board

                if board.is_checkmate():
                    response = f"{best_move}#\n"
                    print(f"Checkmate")
                elif board.is_check():
                    response = f"{best_move}+\n"
                    print(f"Check")
                else:
                    response = f"{best_move}\n"
                
                ser.write(response.encode('utf-8'))                                     # write the response to the microcontroller
                print(f"Sent: {response}")
                print(board)

            else:
                print(f"The move '{move}' is not legal in the current position.")
                response = f"{move}\n"
                ser.write(response.encode('utf-8'))
                print(f"Sent: {move}")
                move = None
    print(f"Check Mate")
    


except KeyboardInterrupt:
    print("Program terminated by user.")
finally:
    ser.close()
    engine.quit()
