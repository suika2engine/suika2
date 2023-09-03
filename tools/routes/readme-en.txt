Suika2 Route Search Tool
========================

This program identifies all the endings in the Suika2 game and writes in Excel
all the combinations of options that lead to the endings.

# Preparation

1. Insert `:STORY_START` at the beginning of the story
  - It should be after menu and before common story
2. Insert `:GOAL_Ending-Name` at the line where a ending is determined
  - They should be at the first lines of branch routes.

# Notice

1. Note when there is a loop in the choices
  - Once an option is selected, it will not be selected a second time even if it loops
2. If the combination of choices is too complex, the program will give up the search
  - Point: Do not make a loop of choices
  - Point: Use up to 3 options in a choice

# Run

Run `suika-routes.exe` and it will generate `routes.csv`.
Open `routes.csv` in Excel.
