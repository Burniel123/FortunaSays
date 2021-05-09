# FortunaSays V1.0
A variant of "Simon Says" for the LaFortuna microcontroller, built using FortunaOS. Licensed under the [Creative Commons Attribution License](https://creativecommons.org/licenses/by/4.0/).

Uses a rotary encoder library by Steve Gunn, as well as functionality provided by FortunaOS (containing code by Steve Gunn, Klaus-Peter Zauner, and Peter Dannegger).

## Compilation & Usage
The included Makefile (as provided by Klaus-Peter Zauner) can be used to compile the game and write it to your LaFortuna.

This game makes use of the UP/DOWN/LEFT/RIGHT buttons on the LaFortuna as well as the CENTER button. The rotary encoder itself is not used.

Press the CENTER button to start a game. Remember the series of flashing arrows. When the top-left of the screen reads "GO!" re-enter the sequence as it flashed. The game repeats, getting progressively more difficult, until an incorrect response is provided three times. At this point, a score for the game is displayed. Press CENTER to play again.

If an SD Card is inserted into the LaFortuna's dedicated port, scores are automatically saved at the end of each game. To view the 10 highest scores, press DOWN at either the start or game over screen.

## Known Issues
Occasionally, inputting the sequence very quickly can result in dropped inputs. This generally does not affect gameplay unless the user is deliberately spamming buttons to try and break the game.

## Future Extensions
Possible areas for further development include:
- Ability to include names or three-letter abbreviations alongside saved scores (as per retro arcade machines).
- Greater variation in difficulty modes, scoring, etc.
- Fancier menu screens and/or arrows.
- Requiring specific timing/rhythm with inputs and/or developing this concept to create a rhythm game.

External contributions are welcomed!
