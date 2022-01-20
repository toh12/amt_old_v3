# Awesomenauts Modding Toolkit v3

This is the code designed to run the AMT user interface. The project was
started to improve my C++ coding skills, and is still in development.
This version is available here as a code sample only, and will not be
developed any further.

The next version is making large changes in the framework: by moving all of
the file management tools into the APCL library, adding limiters preventing
patched games from entering the matchmaking queue, creating a distributable
mod files and allowing for dynamic loading of files by the editor.

Out of consideration to the developers, the cryptography and patching library
cannot be made public at this time, as the patching of files of a online PvP
game could lead to an incease in hackers in a game that is still being 
monatized. As a result the code cannot be compiled unless you were to reverse
engineer the game and implement the APCL library yourself. However, d
demonstrations are available on request.

# Comments on the code

There are many techniques I have learnt and used in making this code:
- Library creation, compilation and linkage techinques (APCL)
- Forward declaration of classes to avoid circular references
- Iteratiors with and in algoritms (APCL & Localization Parser)
- Smart pointers (mainly unique_ptrs in amtApp)
- Experimented more with lambda functions (TreeMap in interface)
- Became comfortable with brace initilization and virtual functions
- Learnt about mutable, constexpr and inline keywords
- Improved my usage and understanding of const
- Improved my usage of templates
- Better understood the value of following a naming convention

However, I also have found several issues with the current code:

- Class design for the interface is trying to do too much with too few classes:
  The Container class in framework.h lacked a clear functionality, such
  should have been clear to me from the inability to name it well.

- Code is poorly formatted in many places, with no line limit kept to:
  it was designed in mind as being a tool for just myself, so I didn't
  consider viewing it on anything other than my widescreen monitor.
  Since this project, I've been keeping to an 80 column limit when coding
  in my free time, as it makes it easier to share.

- Code is poorly commented, and some comments are having to be added
  retrospectively. As a result of this project, I'm writing comments before 
  coding when possible; a mix of a comment before each function and one line 
  comments to plan out more complicated functions where needed.

- Took on too much work by myself; I should have looked to at least making
  the GUI side public. The main reason I decided to make a new version
  was to move the cryptography & patching code into a library (partially 
  complete) and make the GUI side of the project open source.
