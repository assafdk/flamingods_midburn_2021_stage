import time #for sleep
import threading #for threading
import serial #for using COM ports
import players #for using the players


def listenToInterruptions():
    while True:
        print('listenToInterruptions() - thread in here is ' + str(threading.currentThread().getName()))
        players.input = players.s.read()
        print('main print ' + str(players.input))
        if players.isPlaying:
            players.interrupt()
    

main = threading.Thread(name='main',target=players.go, args=()) #listen only to events on players program
listener = threading.Thread(name='listener',target=listenToInterruptions, args=()) #liten only to input on main program and plays only songs


main.start()
listener.start()