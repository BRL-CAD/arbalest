*.theme files store the color mapping etc. for a qss file in key value pair format.
Syntax highlighting for SCSS files should work with *.theme
Syntax example
    $primary-color: #333333;
    $border-color : #ff1111;    // the color used for borders

I could have used a simpler format (ex: "primary-color = #ccc") but this way I can
use SCSS settings in the IDE (live color gutter icon etc.)

QSSPreprocessor class uses the mapping in a .theme file to process a .qss file.
It simply replaces "$primary-color" (qss tag must have quotation marks)  with #333333 in the .qss

Keys or values in .theme cannot contain following characters and space character
; :