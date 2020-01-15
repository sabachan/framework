import "oslib:/std.oslib"
import "oslib:/math.oslib"
import "oslib:/math_vector.oslib"
import "oslib:/math_quaternion.oslib"

function GlyphPos(i, j) { return [i * 5 + 1, j * 9 + 1] }

export font is sg::image::tool::Font
{
    famillyName : "tech"
    bold : false
    italic : false
    advance : 4
    baseline : 5
    glyphSize : [3, 7]
    kernings : []
    glyphsImageFile : "Font3x7.png"
    imageGlyphs : [
        { pos : GlyphPos( 0, 0)      character : "\0" },
        { pos : GlyphPos( 1, 0)      character : "a" },
        { pos : GlyphPos( 2, 0)      character : "b" },
        { pos : GlyphPos( 3, 0)      character : "c" },
        { pos : GlyphPos( 4, 0)      character : "d" },
        { pos : GlyphPos( 5, 0)      character : "e" },
        { pos : GlyphPos( 6, 0)      character : "f" advanceReduction:1},
        { pos : GlyphPos( 7, 0)      character : "g" },
        { pos : GlyphPos( 8, 0)      character : "h" },
        { pos : GlyphPos( 9, 0)      character : "i" advanceReduction:2},
        { pos : GlyphPos(10, 0)      character : "j" advanceReduction:1},
        { pos : GlyphPos(11, 0)      character : "k" },
        { pos : GlyphPos(12, 0)      character : "l" advanceReduction:1},
        { pos : GlyphPos(13, 0)      character : "m" },
        { pos : GlyphPos(14, 0)      character : "n" },
        { pos : GlyphPos(15, 0)      character : "o" },
        { pos : GlyphPos(16, 0)      character : "p" },
        { pos : GlyphPos(17, 0)      character : "q" },
        { pos : GlyphPos(18, 0)      character : "r" },
        { pos : GlyphPos(19, 0)      character : "s" },
        { pos : GlyphPos(20, 0)      character : "t" advanceReduction:1},
        { pos : GlyphPos(21, 0)      character : "u" },
        { pos : GlyphPos(22, 0)      character : "v" },
        { pos : GlyphPos(23, 0)      character : "w" },
        { pos : GlyphPos(24, 0)      character : "x" },
        { pos : GlyphPos(25, 0)      character : "y" },
        { pos : GlyphPos(26, 0)      character : "z" },
        { pos : GlyphPos(27, 0)      character : "?" },
        { pos : GlyphPos(28, 0)      character : "." },
        { pos : GlyphPos(29, 0)      character : "," },
        { pos : GlyphPos(30, 0)      character : "(" },
        { pos : GlyphPos(31, 0)      character : ")" },
        { pos : GlyphPos( 0, 1)      character : " " advanceReduction:1},
        { pos : GlyphPos( 1, 1)      character : "A" },
        { pos : GlyphPos( 2, 1)      character : "B" },
        { pos : GlyphPos( 3, 1)      character : "C" },
        { pos : GlyphPos( 4, 1)      character : "D" },
        { pos : GlyphPos( 5, 1)      character : "E" },
        { pos : GlyphPos( 6, 1)      character : "F" },
        { pos : GlyphPos( 7, 1)      character : "G" },
        { pos : GlyphPos( 8, 1)      character : "H" },
        { pos : GlyphPos( 9, 1)      character : "I" advanceReduction:2},
        { pos : GlyphPos(10, 1)      character : "J" },
        { pos : GlyphPos(11, 1)      character : "K" },
        { pos : GlyphPos(12, 1)      character : "L" },
        { pos : GlyphPos(13, 1)      character : "M" },
        { pos : GlyphPos(14, 1)      character : "N" },
        { pos : GlyphPos(15, 1)      character : "O" },
        { pos : GlyphPos(16, 1)      character : "P" },
        { pos : GlyphPos(17, 1)      character : "Q" },
        { pos : GlyphPos(18, 1)      character : "R" },
        { pos : GlyphPos(19, 1)      character : "S" },
        { pos : GlyphPos(20, 1)      character : "T" },
        { pos : GlyphPos(21, 1)      character : "U" },
        { pos : GlyphPos(22, 1)      character : "V" },
        { pos : GlyphPos(23, 1)      character : "W" },
        { pos : GlyphPos(24, 1)      character : "X" },
        { pos : GlyphPos(25, 1)      character : "Y" },
        { pos : GlyphPos(26, 1)      character : "Z" },
        { pos : GlyphPos(27, 1)      character : "!" advanceReduction:2},
        { pos : GlyphPos(28, 1)      character : "{" advanceReduction:1},
        { pos : GlyphPos(29, 1)      character : "}" advanceReduction:1},
        { pos : GlyphPos(30, 1)      character : "[" advanceReduction:1},
        { pos : GlyphPos(31, 1)      character : "]" advanceReduction:1},
        { pos : GlyphPos( 0, 2)      character : "@" },
        { pos : GlyphPos( 1, 2)      character : "%" },
        { pos : GlyphPos( 2, 2)      character : "1" },
        { pos : GlyphPos( 3, 2)      character : "2" },
        { pos : GlyphPos( 4, 2)      character : "3" },
        { pos : GlyphPos( 5, 2)      character : "4" },
        { pos : GlyphPos( 6, 2)      character : "5" },
        { pos : GlyphPos( 7, 2)      character : "6" },
        { pos : GlyphPos( 8, 2)      character : "7" },
        { pos : GlyphPos( 9, 2)      character : "8" },
        { pos : GlyphPos(10, 2)      character : "9" },
        { pos : GlyphPos(11, 2)      character : "0" },
        { pos : GlyphPos(12, 2)      character : "+" },
        { pos : GlyphPos(13, 2)      character : "-" },
        { pos : GlyphPos(14, 2)      character : "*" },
        //{ pos : GlyphPos(15, 2)      character : "" }, // div
        { pos : GlyphPos(16, 2)      character : "/" },
        { pos : GlyphPos(17, 2)      character : "'" advanceReduction:2},
        { pos : GlyphPos(18, 2)      character : "\"" },
        { pos : GlyphPos(19, 2)      character : "|" },
        { pos : GlyphPos(20, 2)      character : ":" advanceReduction:2},
        { pos : GlyphPos(21, 2)      character : ";" advanceReduction:1},
        { pos : GlyphPos(22, 2)      character : "_" },
        { pos : GlyphPos(23, 2)      character : "=" },
        { pos : GlyphPos(24, 2)      character : "’" advanceReduction:2},
        { pos : GlyphPos(25, 2)      character : "\\" },
        { pos : GlyphPos(26, 2)      character : "°" advanceReduction:1},
        { pos : GlyphPos(27, 2)      character : "^" },
        //{ pos : GlyphPos(28, 2)      character : "" }, //...
        { pos : GlyphPos(29, 2)      character : "<" },
        { pos : GlyphPos(30, 2)      character : ">" },
        { pos : GlyphPos(31, 2)      character : "~" },
        { pos : GlyphPos( 0, 3)      character : "é" },
        { pos : GlyphPos( 1, 3)      character : "è" },
        { pos : GlyphPos( 2, 3)      character : "ë" },
        { pos : GlyphPos( 3, 3)      character : "ê" },
        { pos : GlyphPos( 4, 3)      character : "à" },
        { pos : GlyphPos( 5, 3)      character : "ï" },
        { pos : GlyphPos( 6, 3)      character : "î" },
        { pos : GlyphPos( 7, 3)      character : "ù" },
        { pos : GlyphPos( 8, 3)      character : "ç" },
        
        { pos : GlyphPos(24, 3)      character : "&" },
        { pos : GlyphPos(25, 3)      character : "$" },
        { pos : GlyphPos(26, 3)      character : "µ" },
        { pos : GlyphPos(27, 3)      character : "#" },
        //{ pos : GlyphPos(28, 3)      character : "" }, // square root
        //{ pos : GlyphPos(29, 3)      character : "" }, // less eq
        //{ pos : GlyphPos(30, 3)      character : "" }, // greater eq
    ]
}