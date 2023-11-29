"""The notes program, a python version of the ardino game
https://youtu.be/in30QGYNcX0?si=UwNil7z2R6xFIUf3
"""
#sudo apt-get install python3-vlc
import random
import time
from pynput import keyboard
import vlc

IS_CHEAT = False

# MP3 file constants
SOUND_CORRECT = 8
SOUND_WRONG = 9
SOUND_GAME_OVER = 10
SOUND_YOUR_TURN = 11
USER_NOTE = None   # key pressed from callback
END_GAME = False

def get_file(note):
  """Returns the filename of the sound file."""
  return f"000{note}.mp3"

def play_audio(note):
  """Plays an audio file and waits for finish."""
  player = vlc.MediaPlayer(get_file(note))
  player.play()
  while player.get_state() != 6: # ENDED
    time.sleep(50/1000)
  player.stop()

def generate_random(length):
  """Gets the random sequence of notes for gameplay."""
  notes = []
  for _ in range(0, length):
    notes.append(random.randint(1,7))
  return notes

def play_notes(notes, level):
  """Play the computer generated sequence."""
  if IS_CHEAT:
    print(f"Notes Says: {notes[0:level]}")
  for i in range(0, level):
    play_audio(notes[i])
  print("Your turn. What did Notes Say[1-7,q]?")

# Handle the keyboard input
def get_user_note():
  """Start listening for user input."""
  with (keyboard.Listener(on_press = on_key_press)
        as press_listener): #setting code for listening key-press
    press_listener.join()

  with (keyboard.Listener(on_release = on_key_release)
        as release_listener): #setting code for listening key-release
    release_listener.join()
  if USER_NOTE:
    return int(USER_NOTE)
  return None

def on_key_release(_):
  """what to do on key-release."""
  return False #stop detecting more key-releases

def on_key_press(key):
  """what to do on key-press."""
  global USER_NOTE, END_GAME
  if key.char == 'q':
    END_GAME = True
    return False
  if key.char not in ['1', '2', '3', '4', '5', '6', '7']:
    print(f"Invalid input: {key.char}")
    return True
  USER_NOTE = int(str(key.char))
  return False #stop detecting more key-presses

def play_user(notes, level):
  """User's turn, get input, check."""
  for cnt in range(0, level):
    user_note = get_user_note()
    if END_GAME:
      return False
    play_audio(user_note)

    if notes[cnt] != user_note:
      print(f"  Ooops. Wrong pattern. Start Over. {user_note} v {notes[cnt]}")
      play_audio(SOUND_WRONG)
      level=0
      return False
  print("  Woohoo. You're an ACE. Great Job!")
  play_audio(SOUND_CORRECT)
  return True

def main():
  """Main."""
  level = 0
  while not END_GAME:
    if level == 0:
      notes = generate_random(100)
    level += 1
    print(f"--Level: {level}")
    play_notes(notes, level)
    if not play_user(notes, level):
      level = 0

main()
