import time #for sleep
import pyglet #for music player
import threading #for threading
import os #for finding song files
import serial #for using COM ports
import zope.event #for event handling
import random #for randomize the songs array


s = serial.Serial(port='COM4',stopbits=serial.STOPBITS_ONE)
input = ''
isPlaying = False
isInterrupt = False
dirPath = "C:/Users/Lidor/Desktop/old projects/musicPlayerScript"


songPlayer = pyglet.media.Player()
vocalPlayer = pyglet.media.Player()
zotPlayer = pyglet.media.Player()
easterPlayer = pyglet.media.Player()
nothingPlayer = pyglet.media.Player()
clapPlayer = pyglet.media.Player()


i = 0
playedSongs = 0
def fillSongsQueue():
    print('fillSongsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    songs = os.listdir(dirPath + "/a - shortsongs/")
    random.shuffle(songs)
    global songPlayer
    global i
    i = 0
    global playedSongs
    playedSongs = 0
    for song in songs:
	    path = dirPath + "/a - shortsongs/" + song
	    song = pyglet.media.load(path)
	    songPlayer.queue(song)
	    i = i + 1


j = 0
playedVocalSongs = 0
def fillVocalsQueue():
    print('fillVocalsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    songs = os.listdir(dirPath + "/b - vocalsongs/")
    random.shuffle(songs)
    global vocalPlayer	
    global j
    j = 0
    global playedVocalSongs
    playedVocalSongs = 0
    for song in songs:
	    path = dirPath + "/b - vocalsongs/" + song
	    song = pyglet.media.load(path)
	    vocalPlayer.queue(song)
	    j = j + 1


m = 0
playedZotSongs = 0
def fillZotsQueue():
    print('fillZotsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    songs = os.listdir(dirPath + "/c - zotsongs/")
    random.shuffle(songs)
    global zotPlayer
    global m
    m = 0
    global playedZotSongs
    playedZotSongs = 0
    for song in songs:
	    path = dirPath + "/c - zotsongs/" + song
	    song = pyglet.media.load(path)
	    zotPlayer.queue(song)
	    m = m + 1
        
        
k = 0
playedEasterSongs = 0
def fillEastersQueue():
    print('fillEastersQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    songs = os.listdir(dirPath + "/d - eastersongs/")
    random.shuffle(songs)
    global easterPlayer
    global k
    k = 0
    global playedEasterSongs
    playedEasterSongs = 0
    for song in songs:
	    path = dirPath + "/d - eastersongs/" + song
	    song = pyglet.media.load(path)
	    easterPlayer.queue(song)
	    k = k + 1


n = 0
playedNothingSongs = 0
def fillNothingsQueue():
    print('fillNothingsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    songs = os.listdir(dirPath + "/e - nothingsongs/")
    random.shuffle(songs)
    global nothingPlayer
    global n
    n = 0
    global playedNothingSongs
    playedNothingSongs = 0
    for song in songs:
	    path = dirPath + "/e - nothingsongs/" + song
	    song = pyglet.media.load(path)
	    nothingPlayer.queue(song)
	    n = n + 1


l = 0
playedClapSongs = 0
def fillClapsQueue():
    print('fillClapsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    songs = os.listdir(dirPath + "/f - clapsongs/")
    random.shuffle(songs)
    global clapPlayer
    global l
    l = 0
    global playedClapSongs
    playedClapSongs = 0
    for song in songs:
	    path = dirPath + "/f - clapsongs/" + song
	    song = pyglet.media.load(path)
	    clapPlayer.queue(song)
	    l = l + 1
		
        
def checkQueues():
    print('checkQueues() - thread in here is ' + str(threading.currentThread().getName()))
    global i
    global playedSongs
    global j
    global playedVocalSongs
    global m
    global playedZotSongs
    global k
    global playedEasterSongs
    global n
    global playedNothingSongs
    global l
    global playedClapSongs
    if playedSongs == i:
        fillSongsQueue()
    if playedVocalSongs == j:
        fillVocalsQueue()
    if playedZotSongs == m:
        fillZotsQueue()
    if playedEasterSongs == k:
        fillEastersQueue()
    if playedNothingSongs == n:
        fillNothingsQueue()
    if playedClapSongs == l:
        fillClapsQueue()


def initiate():
    print('initiate() - thread in here is ' + str(threading.currentThread().getName()))
    global s
    global input
    global isPlaying
    global i
    global playedSongs
    global j
    global playedVocalSongs
    global m
    global playedZotSongs
    global k
    global playedEasterSongs
    global n
    global playedNothingSongs
    global l
    global playedClapSongs
    global songPlayer
    global vocalPlayer
    global zotPlayer
    global easterPlayer
    global nothingPlayer
    global clapPlayer
    print('waiting for serial input at initiate')
    while input == '':
	    pass
    print('got serial input: ' + str(input)+' at initiate')
    isPlaying = True
    if input == b'P':
        songPlayer.play()
        print('songPlayer.play()')
        playedSongs = playedSongs + 1
        print('i = ' + str(i) + ', playedSongs = ' + str(playedSongs)) 
        #clapPlayer.play()
        #print('clapPlayer.play()')
        #playedClapSongs = playedClapSongs + 1
        #print('l = ' + str(l) + ', playedClapSongs = ' + str(playedClapSongs))
   # elif input == b'V':
   #     vocalPlayer.play()
   #     print('vocalPlayer.play()')
   #     playedVocalSongs = playedVocalSongs + 1
   #     print('j = ' + str(j) + ', playedVocalSongs = ' + str(playedVocalSongs))
   # elif input == b'Z':
   #     zotPlayer.play()
   #     print('zotPlayer.play()')
   #     playedZotSongs = playedZotSongs + 1
   #     print('m = ' + str(m) + ', playedZotSongs = ' + str(playedZotSongs))
   # elif input == b'E':
   #     easterPlayer.play()
   #     print('easterPlayer.play()')
   #     playedEasterSongs = playedEasterSongs + 1
   #     print('k = ' + str(k) + ', playedEasterSongs = ' + str(playedEasterSongs))
    elif input == b'S':
        isPlaying = False
        songPlayer.pause()
        #vocalPlayer.pause()
        #zotPlayer.pause()
        #easterPlayer.pause()
        #clapPlayer.pause()
        #s.write(b'F')
        #isPlaying = False
        input = ''
        nothingPlayer.play()
        print('nothingPlayer.play()')
        playedNothingSongs = playedNothingSongs + 1
        print('n = ' + str(n) + ', playedNothingSongs = ' + str(playedNothingSongs))
        

def interrupt(): #This function runs from the listener thread ONLY and has to end without any endless loops (so it can return to listen to inputs)
    print('interrupt() - thread in here is ' + str(threading.currentThread().getName()))
    global s
    global input
    global isPlaying
    global isInterrupt
    global i
    global playedSongs
    global j
    global playedVocalSongs
    global m
    global playedZotSongs
    global k
    global playedEasterSongs
    global n
    global playedNothingSongs
    global l 
    global playedClapSongs
    global songPlayer
    global vocalPlayer
    global zotPlayer
    global easterPlayer
    global nothingPlayer
    global clapPlayer
    print('got serial input: ' + str(input)+' at interrupt')
    isPlaying = True
    isInterrupt = False
    if input == b'P':
        songPlayer.pause
        songPlayer.next_source()
        checkQueues()
        songPlayer.play()
        playedSongs = playedSongs + 1
        print('songPlayer.play()')
        print('i = ' + str(i) + ', playedSongs = ' + str(playedSongs))
        #clapPlayer.pause()
        #clapPlayer.next_source()
        #checkQueues()
        #clapPlayer.play()
        #print('clapPlayer.play()')
        #playedClapSongs = playedClapSongs + 1
        #print('l = ' + str(l) + ', playedClapSongs = ' + str(playedClapSongs))
    elif input == b'V':
        vocalPlayer.pause()
        vocalPlayer.next_source()
        checkQueues()
        vocalPlayer.play()
        playedVocalSongs = playedVocalSongs + 1
        print('vocalPlayer.play()')
        print('j = ' + str(j) + ', playedVocalSongs = ' + str(playedVocalSongs))
    elif input == b'Z':
        zotPlayer.pause()
        zotPlayer.next_source()
        checkQueues()
        zotPlayer.play()
        playedZotSongs = playedZotSongs + 1
        print('zotPlayer.play()')
        print('m = ' + str(m) + ', playedZotSongs = ' + str(playedZotSongs))
    elif input == b'E':
        easterPlayer.pause()
        easterPlayer.next_source()
        checkQueues()
        easterPlayer.play()
        playedEasterSongs = playedEasterSongs + 1
        print('easterPlayer.play()')
        print('k = ' + str(k) + ', playedEasterSongs = ' + str(playedEasterSongs))
    elif input == b'S':
        isPlaying = False
        songPlayer.pause()
        vocalPlayer.pause()
        zotPlayer.pause()
        easterPlayer.pause()
        clapPlayer.pause()
        input = ''
        nothingPlayer.play()
        playedNothingSongs = playedNothingSongs + 1
        print('nothingPlayer.play()')
        print('n = ' + str(n) + ', playedNothingSongs = ' + str(playedNothingSongs))


def endOfPlayer(player):
    print('endOfPlayer() - thread in here is ' + str(threading.currentThread().getName()))
    global s
    global input
    global isPlaying
    global i
    global playedSongs
    global j
    global playedVocalSongs
    global m
    global playedZotSongs
    global k
    global playedEasterSongs
    global n
    global playedNothingSongs
    global l
    global playedClapSongs
    global songPlayer
    global vocalPlayer
    global zotPlayer
    global easterPlayer
    global nothingPlayer
    global clapPlayer
    print('we are here because of ' + str(player))
    s.write(b'F')
    input = ''
    isPlaying = False
    checkQueues()
    print('waiting for serial input at endsongevent')
    #for some stupid reason, pyglet's player cannot run while an infinite loop is activated, so I pause every players manualy before the next line happens (at songevent endofsong)
	#need to fix that somehow :-(
    while input == '':
        pass
    print('got serial input: ' + str(input)+' at endsongevent')
    isPlaying = True
    if input == b'P':
        songPlayer.play()
        playedSongs = playedSongs + 1
        print('songPlayer.play()')
        print('i = ' + str(i) + ', playedSongs = ' + str(playedSongs)) 
        #clapPlayer.play()
        #print('clapPlayer.play()')
        #playedClapSongs = playedClapSongs + 1
        #print('l = ' + str(l) + ', playedClapSongs = ' + str(playedClapSongs))
    elif input == b'V':
        vocalPlayer.play()
        playedVocalSongs = playedVocalSongs + 1
        print('vocalPlayer.play()')
        print('j = ' + str(j) + ', playedVocalSongs = ' + str(playedVocalSongs))
    elif input == b'Z':
        zotPlayer.play()
        playedZotSongs = playedZotSongs + 1
        print('zotPlayer.play()')
        print('m = ' + str(m) + ', playedZotSongs = ' + str(playedZotSongs))
    elif input == b'E':
        easterPlayer.play()
        playedEasterSongs = playedEasterSongs + 1
        print('easterPlayer.play()')
        print('k = ' + str(k) + ', playedEasterSongs = ' + str(playedEasterSongs))
    elif input == b'S':
        isPlaying = False
        songPlayer.pause()
        vocalPlayer.pause()
        zotPlayer.pause()
        easterPlayer.pause()
        #clapPlayer.pause()
        input = ''
        nothingPlayer.play()
        playedNothingSongs = playedNothingSongs + 1
        print('nothingPlayer.play()')
        print('n = ' + str(n) + ', playedNothingSongs = ' + str(playedNothingSongs))
		
		
@songPlayer.event
def on_eos(): #The songPlayer has reached the end of the current songPlayer song
    print('@songPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global songPlayer
    songPlayer.pause()

    #this is because of the comment at interrupt() about the infinite loop and pyglet
    global playedVocalSongs
    global playedZotSongs
    global playedEasterSongs
    global playedNothingSongs
    global playedClapSongs
    global vocalPlayer
    global zotPlayer
    global easterPlayer
    global nothingPlayer
    global clapPlayer

    vocalPlayer.pause()
    zotPlayer.pause()
    clapPlayer.pause()
    easterPlayer.pause()
    nothingPlayer.pause()
    vocalPlayer.next_source()
    zotPlayer.next_source()
    clapPlayer.next_source()
    easterPlayer.next_source()
    nothingPlayer.next_source()
	
    playedVocalSongs = playedVocalSongs + 1
    playedZotSongs = playedZotSongs + 1
    playedClapSongs = playedClapSongs + 1
    playedEasterSongs = playedEasterSongs + 1
    playedNothingSongs = playedNothingSongs + 1
    
    checkQueues()
    
	#until here
	
    endOfPlayer('song')


@vocalPlayer.event
def on_eos(): #The vocalPlayer has reached the end of the current vocalPlayer song
    print('@vocalPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global vocalPlayer
    vocalPlayer.pause()
    #endOfPlayer('vocal')
		

@zotPlayer.event
def on_eos(): #The zotPlayer has reached the end of the current zotPlayer song
    print('@zotPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global zotPlayer
    zotPlayer.pause()
    #endOfPlayer('zot')
   
   
@easterPlayer.event
def on_eos(): #The easterPlayer has reached the end of the current easterPlayer song
    print('@easterPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global easterPlayer
    easterPlayer.pause()
    #endOfPlayer('easter')
	
	
@nothingPlayer.event
def on_eos(): #The nothingPlayer has reached the end of the current nothingPlayer song
    print('@nothingPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global nothingPlayer
    nothingPlayer.pause()
    endOfPlayer('nothing')

@clapPlayer.event
def on_eos(): #The clapPlayer has reached the end of the current clapPlayer song
    print('@clapPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global clapPlayer
    clapPlayer.pause()
    #endOfPlayer('clap')
		
        
def go():
    print('go() - thread in here is ' + str(threading.currentThread().getName()))
    fillSongsQueue()
    fillVocalsQueue()
    fillZotsQueue()
    fillEastersQueue()
    fillNothingsQueue()
    fillClapsQueue()
    initiate()
    pyglet.app.run()