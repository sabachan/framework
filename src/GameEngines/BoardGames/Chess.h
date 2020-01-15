#ifndef GameEngines_BoardGames_Chess_H
#define GameEngines_BoardGames_Chess_H

#include <Math/Box.h>
#include <Math/Vector.h>

namespace sg {
namespace game {
namespace chess {
//=============================================================================
enum class Piece
{
    // Note : la représentation d'une pièce est sur 4 bits. En particulier :
    //          0 représente la capacité à se déplacer en ligne droite,
    //          1 représente la capacité à se déplacer en diagonale.
    King   = 0x0B,
    Queen  = 0x03,
    Rook   = 0x01,
    Bishop = 0x02,
    Knight = 0x04,
    Pawn   = 0x08,
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
inline bool IsValid(Piece p)
{
    switch(p)
    {
    case Piece::King  :
    case Piece::Queen :
    case Piece::Rook  :
    case Piece::Bishop:
    case Piece::Knight:
    case Piece::Pawn  :
        return true;
    default:
        return false;
    }
}
//=============================================================================
enum class Color
{
    White,
    Black,
};
//=============================================================================
class Square
{
public:
    Square() : m_data(0) {}
    Square(Square const&) = default;
    Square& operator=(Square const&) = default;

    bool IsOccupied() const { return (m_data & flagIsOccupied) != 0; }
    //bool IsBlackPiece() { SG_ASSERT(IsOccupied()); return (m_data & flagIsBlack) != 0; }
    Color GetColor() const { SG_ASSERT(IsOccupied()); return (m_data & flagIsBlack) == 0 ? Color::White : Color::Black; }
    Piece GetPiece() const { SG_ASSERT(IsOccupied()); Piece const p = Piece(m_data & maskPiece); SG_ASSERT(IsValid(p)); return p; }
    void SetEmpty() { m_data = 0; }
    void SetPiece(Piece iPiece, Color color) {
        m_data = u8(iPiece) | flagIsOccupied | (Color::Black == color ? flagIsBlack : 0);
    }
private:
    // Note : la représentation d'une case est sur 8 bits :
    //   [0-3]  représente la pièce présente.
    //   4      est à 1 si une pièce est présente.
    //   5      est à 1 si la pièce présente est noire.
    static size_t const maskPiece = 0x0F;
    static size_t const flagIsOccupied = 0x10;
    static size_t const flagIsBlack = 0x20;
private:
    u8 m_data;
};
//=============================================================================
class Board
{
public:
    static size_t const gridSize = 8;
    static size_t const squareCount = gridSize * gridSize;

    static bool IsInside(int2 iPos) { return AllLessStrict(uint2(iPos), uint2(gridSize)); }
    static bool IsDarkSquare(int2 iPos) { return ((iPos.x() + iPos.y()) % 2) == 0; }
    static int GetSquareIndex(int2 iPos) { SG_ASSERT(IsInside(iPos)); return iPos.x() + iPos.y() * gridSize; }
    static int2 PositionFromSquareIndex(int iSquareIndex) { SG_ASSERT(size_t(iSquareIndex) < squareCount); return int2(iSquareIndex % gridSize, iSquareIndex / gridSize); }

    Board() { Reset(); }
    Board(Board const&) = default;
    Board& operator=(Board const&) = default;

    Square operator[](int2 iPos) const { return m_data[GetSquareIndex(iPos)]; }
    Square& operator[](int2 iPos) { return m_data[GetSquareIndex(iPos)]; }

    void Reset()
    {
        m_data[0        ].SetPiece(Piece::Rook,   Color::White);
        m_data[1        ].SetPiece(Piece::Knight, Color::White);
        m_data[2        ].SetPiece(Piece::Bishop, Color::White);
        m_data[3        ].SetPiece(Piece::Queen,  Color::White);
        m_data[4        ].SetPiece(Piece::King,   Color::White);
        m_data[5        ].SetPiece(Piece::Bishop, Color::White);
        m_data[6        ].SetPiece(Piece::Knight, Color::White);
        m_data[7        ].SetPiece(Piece::Rook,   Color::White);

        m_data[0 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[1 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[2 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[3 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[4 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[5 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[6 +     8].SetPiece(Piece::Pawn,   Color::White);
        m_data[7 +     8].SetPiece(Piece::Pawn,   Color::White);

        for(int i = 2*8; i < 6*8; ++i) { m_data[i].SetEmpty(); }

        m_data[0 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[1 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[2 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[3 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[4 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[5 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[6 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);
        m_data[7 + 6 * 8].SetPiece(Piece::Pawn,   Color::Black);

        m_data[0 + 7 * 8].SetPiece(Piece::Rook,   Color::Black);
        m_data[1 + 7 * 8].SetPiece(Piece::Knight, Color::Black);
        m_data[2 + 7 * 8].SetPiece(Piece::Bishop, Color::Black);
        m_data[3 + 7 * 8].SetPiece(Piece::Queen,  Color::Black);
        m_data[4 + 7 * 8].SetPiece(Piece::King,   Color::Black);
        m_data[5 + 7 * 8].SetPiece(Piece::Bishop, Color::Black);
        m_data[6 + 7 * 8].SetPiece(Piece::Knight, Color::Black);
        m_data[7 + 7 * 8].SetPiece(Piece::Rook,   Color::Black);
    }
private:
    Square m_data[squareCount];
};
//=============================================================================
enum class MoveType { None, Simple, Promote, CaptureAndPromote, Capture, EnPassant, CastleQueenside, CastleKingside };
//=============================================================================
class State
{
public:
    State() { Reset(); }
    void Reset();
    Color ColorToPlay() const { return m_ply % 2 == 0 ? Color::White : Color::Black; }
    Color NextColorToPlay() const { return m_ply % 2 == 1 ? Color::White : Color::Black; }
    size_t GetMove() const { return m_ply / 2; }
    Board& GetBoard() { return m_board; }
    Board const& GetBoard() const { return m_board; }

    bool IsGameFinished() const { return (m_allowedCastleAndState & GAME_FINISHED_MASK) != 0; }
    bool IsCheck() const { return (m_allowedCastleAndState & GAME_STATE_MASK) == CHECK; }
    bool IsCheckmate() const { return (m_allowedCastleAndState & GAME_STATE_MASK) == CHECK_MATE; }
    bool IsStaleMate() const { return (m_allowedCastleAndState & GAME_STATE_MASK) == STALE_MATE; }
    bool IsInsufficientMaterialDraw() const { return (m_allowedCastleAndState & GAME_STATE_MASK) == MATERIAL_DRAW; }

    bool IsResign() const { return false; }
    bool isAgreedDraw() const { return false; }

    bool CanCastleQueenside() const;
    bool CanCastleKingside() const;
    bool CanCaptureEnPassant(int2 iBegin, int2 iEnd) const;
    bool CanSimpleMove(int2 iBegin, int2 iEnd) const;
    bool CanMove(int2 iBegin, int2 iEnd) const;
    MoveType GetMoveType(int2 iBegin, int2 iEnd) const;

    void CastleQueenside();
    void CastleKingside();
    void CaptureEnPassant(int2 iBegin, int2 iEnd);
    void SimpleMove(int2 iBegin, int2 iEnd);
    void MoveAndPromote(int2 iBegin, int2 iEnd, Piece iPieceID);

    void Move(int2 iBegin, int2 iEnd);

    //void Resign() {}
    //void OfferDraw() {}
    //void AcceptDraw() {}
    bool CanClaimFiftyMoveRule() const;
    bool CanClaimThreefoldRepetition() const;
    void ClaimFiftyMoveRule();
    void ClaimThreefoldRepetition();
private:
    static bool ComputeIsCheck(Board const& iBoard, Color iColorToPlay);
    static bool ComputeMaterialDraw(Board const& iBoard);
    bool ComputeIsMat() const;
    void BeforeApplyingHalfMove();
    void AfterApplyingHalfMove();

private:
    // Allowed castle
    static u8 const WHITE_KINGSIDE_CASTLE_ALLOWED  = 0x10;
    static u8 const WHITE_QUEENSIDE_CASTLE_ALLOWED = 0x20;
    static u8 const BLACK_KINGSIDE_CASTLE_ALLOWED  = 0x40;
    static u8 const BLACK_QUEENSIDE_CASTLE_ALLOWED = 0x80;
    static u8 const K = WHITE_KINGSIDE_CASTLE_ALLOWED;
    static u8 const Q = WHITE_QUEENSIDE_CASTLE_ALLOWED;
    static u8 const k = BLACK_KINGSIDE_CASTLE_ALLOWED;
    static u8 const q = BLACK_QUEENSIDE_CASTLE_ALLOWED;

    static u8 const CHECK         = 0x08;
    static u8 const CHECK_MATE    = 0x01;
    static u8 const RESIGNED      = 0x02;
    static u8 const STALE_MATE    = 0x03;
    static u8 const MATERIAL_DRAW = 0x04;
    static u8 const OTHER_DRAW    = 0x05;

    static u8 const GAME_FINISHED_MASK = 0x07;
    static u8 const GAME_STATE_MASK    = 0x0F;

private:
    int m_ply; // nombre de demi-coups. cf. http://en.wikipedia.org/wiki/Glossary_of_chess
    //u8 m_halfMoveClock; // nombre de demi-coups depuis la dernière capture ou déplacement de pion.
    u8 m_enPassantSquareIndex;
    u8 m_allowedCastleAndState;
    Board m_board;
};
//=============================================================================
}
}
}

#endif
