import time #for sleep
import pyglet #for music player
import threading #for threading
import os #for finding song files
import serial #for using COM ports
import random #for randomize the songs array


s = serial.Serial(port='COM6',stopbits=serial.STOPBITS_ONE)
input = ''
isPlaying = False
dirPath = "C:/Users/Lidor/Desktop/old projects/musicPlayerScript"

songPlayer = pyglet.media.Player()
vocalPlayer = pyglet.media.Player()
zotPlayer = pyglet.media.Player()
easterPlayer = pyglet.media.Player()
clapPlayer = pyglet.media.Player()
nothingPlayer = pyglet.media.Player()
songPlayer.volume = 0.4 #This is 0.4 on purpose (so you would hear easter eggs above it)
vocalPlayer.volume = 1.0
zotPlayer.volume = 1.0
easterPlayer.volume = 1.0
clapPlayer.volume = 1.0
nothingPlayer.volume = 0.0 #This is 0.0 on purpose


i = 0
playedSongs = 0
def fillSongsQueue():
    print('fillSongsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    global songPlayer
    global i
    global playedSongs
    songs = os.listdir(dirPath + "/a - shortsongs/")
    random.shuffle(songs)
    i = 0
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
    global vocalPlayer	
    global j
    global playedVocalSongs
    songs = os.listdir(dirPath + "/b - vocalsongs/")
    random.shuffle(songs)
    j = 0
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
    global zotPlayer
    global m
    global playedZotSongs
    songs = os.listdir(dirPath + "/c - zotsongs/")
    random.shuffle(songs)
    m = 0
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
    global easterPlayer
    global k
    global playedEasterSongs
    songs = os.listdir(dirPath + "/d - eastersongs/")
    random.shuffle(songs)
    k = 0
    playedEasterSongs = 0
    for song in songs:
	    path = dirPath + "/d - eastersongs/" + song
	    song = pyglet.media.load(path)
	    easterPlayer.queue(song)
	    k = k + 1


l = 0
playedClapSongs = 0
def fillClapsQueue():
    print('fillClapsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    global clapPlayer
    global l
    global playedClapSongs
    songs = os.listdir(dirPath + "/f - clapsongs/")
    random.shuffle(songs)
    l = 0
    playedClapSongs = 0
    for song in songs:
	    path = dirPath + "/f - clapsongs/" + song
	    song = pyglet.media.load(path)
	    clapPlayer.queue(song)
	    l = l + 1


n = 0
playedNothingSongs = 0
def fillNothingsQueue():
    print('fillNothingsQueue() - thread in here is ' + str(threading.currentThread().getName()))
    global dirPath
    global nothingPlayer
    global n
    global playedNothingSongs
    songs = os.listdir(dirPath + "/e - nothingsongs/")
    random.shuffle(songs)
    n = 0
    playedNothingSongs = 0
    for song in songs:
	    path = dirPath + "/e - nothingsongs/" + song
	    song = pyglet.media.load(path)
	    nothingPlayer.queue(song)
	    n = n + 1
		
        
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
    global songPlayer
    global vocalPlayer
    global zotPlayer
    global easterPlayer
    global clapPlayer
    global nothingPlayer
    if playedSongs >= i:
        fillSongsQueue()
    if playedVocalSongs >= j:
        fillVocalsQueue()
    if playedZotSongs >= m:
        fillZotsQueue()
    if playedEasterSongs >= k:
        fillEastersQueue()
    if playedClapSongs >= l:
        fillClapsQueue()
    if playedNothingSongs >= n:
        fillNothingsQueue()


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
    global clapPlayer
    global nothingPlayer
    input = ''
    print('waiting for serial input at initiate')
    while input == '':
	    pass
    print('got serial input: ' + str(input)+' at initiate')
    checkQueues()
    if input == b'P':
        isPlaying = True
        songPlayer.play()
        playedSongs = playedSongs + 1
        print('songPlayer.play()')
        print('i = ' + str(i) + ', playedSongs = ' + str(playedSongs)) 
    else:
        songPlayer.pause()
        isPlaying = False
        input = ''
        nothingPlayer.play()
        playedNothingSongs = playedNothingSongs + 1
        print('nothingPlayer.play()')
        print('n = ' + str(n) + ', playedNothingSongs = ' + str(playedNothingSongs))
        

def interrupt(): #This function runs from the listener thread ONLY and has to end without any endless loops (so it can return to listen to inputs)
    print('interrupt() - thread in here is ' + str(threading.currentThread().getName()))
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
    global clapPlayer
    global nothingPlayer
    print('got serial input: ' + str(input)+' at interrupt')
    if input == b'P':
        songPlayer.pause()
        songPlayer.next_source()
        playedSongs = playedSongs + 1
        checkQueues()
        songPlayer.play()
        isPlaying = True
        print('songPlayer.play()')
        print('i = ' + str(i) + ', playedSongs = ' + str(playedSongs))
    elif input == b'V':
        vocalPlayer.pause()
        vocalPlayer.next_source()
        playedVocalSongs = playedVocalSongs + 1
        checkQueues()
        vocalPlayer.play()
        print('vocalPlayer.play()')
        print('j = ' + str(j) + ', playedVocalSongs = ' + str(playedVocalSongs))
    elif input == b'Z': 
        vocalPlayer.pause()
        vocalPlayer.next_source()
        playedVocalSongs = playedVocalSongs + 1
        zotPlayer.pause()
        zotPlayer.next_source()
        playedZotSongs = playedZotSongs + 1
        checkQueues()
        zotPlayer.play()
        print('zotPlayer.play()')
        print('m = ' + str(m) + ', playedZotSongs = ' + str(playedZotSongs))
    elif input == b'E':
        easterPlayer.pause()
        easterPlayer.next_source()
        playedEasterSongs = playedEasterSongs + 1
        checkQueues()
        easterPlayer.play()
        print('easterPlayer.play()')
        print('k = ' + str(k) + ', playedEasterSongs = ' + str(playedEasterSongs))
    elif input == b'S':
        isPlaying = False
        songPlayer.pause()
        songPlayer.next_source()
        playedSongs = playedSongs + 1
        vocalPlayer.pause()
        vocalPlayer.next_source()
        playedVocalSongs = playedVocalSongs + 1
        zotPlayer.pause()
        zotPlayer.next_source()
        playedZotSongs = playedZotSongs + 1
        easterPlayer.pause()
        easterPlayer.next_source()
        playedEasterSongs = playedEasterSongs + 1
        clapPlayer.pause()
        clapPlayer.next_source()
        playedClapSongs = playedClapSongs + 1
        input = ''
        checkQueues()
        nothingPlayer.play()
        playedNothingSongs = playedNothingSongs + 1
        print('nothingPlayer.play()')
        print('n = ' + str(n) + ', playedNothingSongs = ' + str(playedNothingSongs))
		
		
@songPlayer.event
def on_eos(): #The songPlayer has reached the end of the current songPlayer song
    print('@songPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global s
    global songPlayer
    songPlayer.pause()
    s.write(b'F')
    initiate()
    
    
@vocalPlayer.event
def on_eos(): #The vocalPlayer has reached the end of the current vocalPlayer song
    print('@vocalPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global vocalPlayer
    vocalPlayer.pause()
		

@zotPlayer.event
def on_eos(): #The zotPlayer has reached the end of the current zotPlayer song
    print('@zotPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global zotPlayer
    zotPlayer.pause()
   
   
@easterPlayer.event
def on_eos(): #The easterPlayer has reached the end of the current easterPlayer song
    print('@easterPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global easterPlayer
    easterPlayer.pause()
	
	
@nothingPlayer.event
def on_eos(): #The nothingPlayer has reached the end of the current nothingPlayer song
    print('@nothingPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global nothingPlayer
    nothingPlayer.pause()
    initiate()

@clapPlayer.event
def on_eos(): #The clapPlayer has reached the end of the current clapPlayer song
    print('@clapPlayer.event() - thread in here is ' + str(threading.currentThread().getName()))
    global clapPlayer
    clapPlayer.pause()
		
        
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