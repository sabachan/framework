#include "stdafx.h"

#include "Chess.h"

#include <Core/Cast.h>
#include <Core/For.h>

namespace sg {
namespace game {
namespace chess {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_King_IgnoreCheck(Board const& iBoard, int2 iBegin, int2 iEnd)
{
    SG_UNUSED(iBoard);
    SG_ASSERT(Board::IsInside(iBegin));
    SG_ASSERT(Board::IsInside(iEnd));
    SG_ASSERT(iBegin != iEnd);
    int2 const delta = iEnd - iBegin;
    int2 const absdelta = componentwise::abs(delta);
    if(!AllLessEqual(absdelta, int2(1)))
        return false;
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_Rook_IgnoreCheck(Board const& iBoard, int2 iBegin, int2 iEnd)
{
    SG_ASSERT(Board::IsInside(iBegin));
    SG_ASSERT(Board::IsInside(iEnd));
    SG_ASSERT(iBegin != iEnd);
    int2 const delta = iEnd - iBegin;
    if(0 != delta.x())
    {
        if(0 != delta.y())
            return false;
        int const sign = 0 < delta.x() ? 1 : -1;
        for(int i = sign; i != delta.x(); i += sign)
        {
            Square const square = iBoard[iBegin + int2(i, 0)];
            bool const isOccupied = square.IsOccupied();
            if(isOccupied)
                return false;
        }
        return true;
    }
    else
    {
        SG_ASSERT(0 != delta.y());
        int const sign = 0 < delta.y() ? 1 : -1;
        for(int i = sign; i != delta.y(); i += sign)
        {
            Square const square = iBoard[iBegin + int2(0, i)];
            bool const isOccupied = square.IsOccupied();
            if(isOccupied)
                return false;
        }
        return true;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_Bishop_IgnoreCheck(Board const& iBoard, int2 iBegin, int2 iEnd)
{
    SG_ASSERT(Board::IsInside(iBegin));
    SG_ASSERT(Board::IsInside(iEnd));
    SG_ASSERT(iBegin != iEnd);
    int2 const delta = iEnd - iBegin;
    int2 const absdelta = componentwise::abs(delta);
    bool const isDiagonal = absdelta.x() == absdelta.y();
    if(!isDiagonal)
        return false;
    int2 const sign = int2(0 < delta.x() ? 1 : -1, 0 < delta.y() ? 1 : -1);
    int const n = absdelta.x();
    SG_ASSERT(0 < n);
    for(int i = 1; i < n; ++i)
    {
        Square const square = iBoard[iBegin + i * sign];
        bool const isOccupied = square.IsOccupied();
        if(isOccupied)
            return false;
    }
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_Queen_IgnoreCheck(Board const& iBoard, int2 iBegin, int2 iEnd)
{
    return CanMove_Rook_IgnoreCheck(iBoard, iBegin, iEnd)
        || CanMove_Bishop_IgnoreCheck(iBoard, iBegin, iEnd);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_Knight_IgnoreCheck(Board const& iBoard, int2 iBegin, int2 iEnd)
{
    SG_UNUSED(iBoard);
    SG_ASSERT(Board::IsInside(iBegin));
    SG_ASSERT(Board::IsInside(iEnd));
    SG_ASSERT(iBegin != iEnd);
    int2 const delta = iEnd - iBegin;
    int2 const absDelta = componentwise::abs(delta);
    if(absDelta == int2(2,1) || absDelta == int2(1,2))
        return true;
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_Pawn_IgnoreCheckAndEnPassant(Board const& iBoard, Color iColorToPlay, int2 iBegin, int2 iEnd)
{
    SG_ASSERT(Board::IsInside(iBegin));
    SG_ASSERT(Board::IsInside(iEnd));
    SG_ASSERT(iBegin != iEnd);
    int2 const delta = iEnd - iBegin;
    int2 const absDelta = componentwise::abs(delta);
    switch(iColorToPlay)
    {
    case Color::White:
        if(delta.y() != 1)
        {
            if(delta == int2(0,2) && iBegin.y() == 1)
            {
                if(iBoard[iBegin+int2(0,1)].IsOccupied())
                    return false;
            }
            else
                return false;
        }
        break;
    case Color::Black:
        if(delta.y() != -1)
        {
            if(delta == int2(0,-2) && iBegin.y() == 6)
            {
                if(iBoard[iBegin+int2(0,-1)].IsOccupied())
                    return false;
            }
            else
                return false;
        }
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }
    if(0 == delta.x())
    {
        Square const square = iBoard[iEnd];
        bool const isOccupied = square.IsOccupied();
        if(isOccupied)
            return false;
        else
            return true;
    }
    else if(1 == absDelta.x())
    {
        Square const square = iBoard[iEnd];
        bool const isOccupied = square.IsOccupied();
        if(isOccupied && square.GetColor() != iColorToPlay)
            return true;
        else
            return false;
    }
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool CanMove_IgnoreCheckAndEnPassantAndCastle(Board const& iBoard, Color iColorToPlay, int2 iBegin, int2 iEnd)
{
    SG_ASSERT(Board::IsInside(iBegin));
    SG_ASSERT(Board::IsInside(iEnd));
    SG_ASSERT(iBegin != iEnd);
    Square const beginSquare = iBoard[iBegin];
    SG_ASSERT(beginSquare.IsOccupied());
    // TODO: remove one or the other
    SG_ASSERT(beginSquare.GetColor() == iColorToPlay);
    if(beginSquare.GetColor() != iColorToPlay)
        return false;
    Square const endSquare = iBoard[iEnd];
    bool const isEndSquareOccupied = endSquare.IsOccupied();
    if(isEndSquareOccupied)
    {
        if(endSquare.GetColor() == iColorToPlay)
            return false;
    }

    Piece const piece = beginSquare.GetPiece();
    switch(piece) {
    case Piece::King:
        return CanMove_King_IgnoreCheck(iBoard, iBegin, iEnd);
    case Piece::Queen:
        return CanMove_Queen_IgnoreCheck(iBoard, iBegin, iEnd);
    case Piece::Rook:
        return CanMove_Rook_IgnoreCheck(iBoard, iBegin, iEnd);
    case Piece::Bishop:
        return CanMove_Bishop_IgnoreCheck(iBoard, iBegin, iEnd);
    case Piece::Knight:
        return CanMove_Knight_IgnoreCheck(iBoard, iBegin, iEnd);
    case Piece::Pawn:
        return CanMove_Pawn_IgnoreCheckAndEnPassant(iBoard, iColorToPlay, iBegin, iEnd);
    default:
        SG_ASSERT_NOT_REACHED();
    }
    return false;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
void State::Reset()
{
    m_ply = 0;
    m_enPassantSquareIndex = u8(-1);
    m_allowedCastleAndState = K | Q | k | q;
    m_board.Reset();
}
//=============================================================================
bool State::CanCastleQueenside() const
{
    // Castle Rules:
    // Neither the king nor the rook have previously moved during the game (1).
    // There cannot be any pieces between the king and the rook (2).
    // The king cannot be in check (3), nor can the king pass through squares
    // that are under attack by enemy pieces (4), or move to a square where it
    // would result in a check (5).

    // Rule (3)
    if(IsCheck())
        return false;
    bool const castleAllowed = Color::White == ColorToPlay() ? (m_allowedCastleAndState & Q) != 0 : (m_allowedCastleAndState & q) != 0;
    // Rule (1)
    if(!castleAllowed)
        return false;

    int const row = Color::White == ColorToPlay() ? 0 : 7;
    SG_ASSERT(m_board[int2(4, row)].IsOccupied());
    SG_ASSERT(m_board[int2(4, row)].GetColor() == ColorToPlay());
    SG_ASSERT(m_board[int2(4, row)].GetPiece() == Piece::King);
    SG_ASSERT(m_board[int2(0, row)].GetColor() == ColorToPlay());
    SG_ASSERT(m_board[int2(0, row)].GetPiece() == Piece::Rook);

    // Rule (2)
    for(int col = 3; col > 0; --col)
    {
        if(m_board[int2(col, row)].IsOccupied())
            return false;
    }
    // Rule (4)
    for(int col = 3; col > 1; --col)
    {
        Board tmpBoard = m_board;
        tmpBoard[int2(4, row)].SetEmpty();
        tmpBoard[int2(col, row)].SetPiece(Piece::King, ColorToPlay());
        bool const isInCheck = ComputeIsCheck(tmpBoard, ColorToPlay());
        if(isInCheck)
            return false;
    }
    // Rule (5)
    Board tmpBoard = m_board;
    tmpBoard[int2(4, row)].SetEmpty();
    tmpBoard[int2(0, row)].SetEmpty();
    tmpBoard[int2(2, row)].SetPiece(Piece::King, ColorToPlay());
    tmpBoard[int2(3, row)].SetPiece(Piece::Rook, ColorToPlay());
    bool const isInCheck = ComputeIsCheck(tmpBoard, ColorToPlay());
    if(isInCheck)
        return false;
    return true;
}
//=============================================================================
bool State::CanCastleKingside() const
{
    // Castle Rules:
    // Neither the king nor the rook have previously moved during the game (1).
    // There cannot be any pieces between the king and the rook (2).
    // The king cannot be in check (3), nor can the king pass through squares
    // that are under attack by enemy pieces (4), or move to a square where it
    // would result in a check (5).

    // Rule (3)
    if(IsCheck())
        return false;
    bool const castleAllowed = Color::White == ColorToPlay() ? (m_allowedCastleAndState & K) != 0 : (m_allowedCastleAndState & k) != 0;
    // Rule (1)
    if(!castleAllowed)
        return false;

    int const row = Color::White == ColorToPlay() ? 0 : 7;
    SG_ASSERT(m_board[int2(4, row)].IsOccupied());
    SG_ASSERT(m_board[int2(4, row)].GetColor() == ColorToPlay());
    SG_ASSERT(m_board[int2(4, row)].GetPiece() == Piece::King);
    SG_ASSERT(m_board[int2(7, row)].IsOccupied());
    SG_ASSERT(m_board[int2(7, row)].GetColor() == ColorToPlay());
    SG_ASSERT(m_board[int2(7, row)].GetPiece() == Piece::Rook);

    // Rule (2)
    for(int col = 5; col < 7; ++col)
    {
        if(m_board[int2(col, row)].IsOccupied())
            return false;
    }
    // Rule (4)
    for(int col = 5; col < 7; ++col)
    {
        Board tmpBoard = m_board;
        tmpBoard[int2(4, row)].SetEmpty();
        tmpBoard[int2(col, row)].SetPiece(Piece::King, ColorToPlay());
        bool const isInCheck = ComputeIsCheck(tmpBoard, ColorToPlay());
        if(isInCheck)
            return false;
    }
    // Rule (5)
    Board tmpBoard = m_board;
    tmpBoard[int2(4, row)].SetEmpty();
    tmpBoard[int2(7, row)].SetEmpty();
    tmpBoard[int2(6, row)].SetPiece(Piece::King, ColorToPlay());
    tmpBoard[int2(5, row)].SetPiece(Piece::Rook, ColorToPlay());
    bool const isInCheck = ComputeIsCheck(tmpBoard, ColorToPlay());
    if(isInCheck)
        return false;
    return true;
}
//=============================================================================
bool State::CanCaptureEnPassant(int2 iBegin, int2 iEnd) const
{
    SG_ASSERT(Board::GetSquareIndex(iEnd) == m_enPassantSquareIndex);
    SG_ASSERT(m_board[iBegin].IsOccupied());
    SG_ASSERT(ColorToPlay() == m_board[iBegin].GetColor());
    SG_ASSERT(Piece::Pawn == m_board[iBegin].GetPiece());
    SG_ASSERT(iBegin != iEnd);
    int2 const delta = iEnd - iBegin;
    int2 const absDelta = componentwise::abs(delta);
    if(absDelta.x() != 1)
        return false;
    switch(ColorToPlay())
    {
    case Color::White:
        if(delta.y() != 1)
            return false;
        break;
    case Color::Black:
        if(delta.y() != -1)
            return false;
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    int2 const capturedPawnPos = iEnd + int2(0, Color::White == ColorToPlay() ? -1 : 1);
    SG_ASSERT(m_board[capturedPawnPos].IsOccupied());
    SG_ASSERT(ColorToPlay() != m_board[capturedPawnPos].GetColor());
    SG_ASSERT(Piece::Pawn == m_board[capturedPawnPos].GetPiece());

    // Est-ce que la position finale met le roi en echec ?
    Board tmpBoard = m_board;
    tmpBoard[iBegin].SetEmpty();
    tmpBoard[iEnd].SetPiece(Piece::Pawn, ColorToPlay());
    tmpBoard[capturedPawnPos].SetEmpty();
    bool const isInCheck = ComputeIsCheck(tmpBoard, ColorToPlay());
    if(isInCheck)
        return false;

    return true;
}
//=============================================================================
bool State::CanSimpleMove(int2 iBegin, int2 iEnd) const
{
    // Est-ce que la piece peut effectuer ce mouvement ? Y-a-t-il des pièces sur le chemin ?
    bool const canMove_IgnoreCheck = CanMove_IgnoreCheckAndEnPassantAndCastle(m_board, ColorToPlay(), iBegin, iEnd);
    if(!canMove_IgnoreCheck) return false;

    // Est-ce que la position finale met le roi en echec ?
    SG_ASSERT(m_board[iBegin].GetColor() == ColorToPlay());
    Piece const piece = m_board[iBegin].GetPiece();
    Board tmpBoard = m_board;
    tmpBoard[iBegin].SetEmpty();
    tmpBoard[iEnd].SetPiece(piece, ColorToPlay());

    bool const isInCheck = ComputeIsCheck(tmpBoard, ColorToPlay());
    if(isInCheck) {
        return false;
    }

    return true;
}
//=============================================================================
bool State::CanMove(int2 iBegin, int2 iEnd) const
{
    return MoveType::None != GetMoveType(iBegin, iEnd);
}
//=============================================================================
MoveType State::GetMoveType(int2 iBegin, int2 iEnd) const
{
    SG_ASSERT(m_board[iBegin].IsOccupied());
    SG_ASSERT(m_board[iBegin].GetColor() == ColorToPlay());
    Piece const piece = m_board[iBegin].GetPiece();

    if(CanSimpleMove(iBegin, iEnd))
    {
        bool const dstIsOccupied = m_board[iEnd].IsOccupied();
        if(Piece::Pawn == piece)
        {
            if(0 == iEnd.y() || 7 == iEnd.y())
            {
                if(iBegin.x() == iEnd.x())
                {
                    SG_ASSERT(!dstIsOccupied);
                    return MoveType::Promote;
                }
                else
                {
                    SG_ASSERT(dstIsOccupied);
                    SG_ASSERT(std::abs(iEnd.x() - iBegin.x()) == 1);
                    return MoveType::CaptureAndPromote;
                }
            }
            else
            {
                if(iBegin.x() == iEnd.x())
                {
                    SG_ASSERT(!dstIsOccupied);
                    return MoveType::Simple;
                }
                else
                {
                    SG_ASSERT(dstIsOccupied);
                    SG_ASSERT(std::abs(iEnd.x() - iBegin.x()) == 1);
                    return MoveType::Capture;
                }
            }
        }
        if(dstIsOccupied)
            return MoveType::Capture;
        else
            return MoveType::Simple;
    }


    // Est-ce un roque ?
    switch(ColorToPlay())
    {
    case Color::White:
        if(int2(4,0) == iBegin && Piece::King == piece)
        {
            if(int2(6,0) == iEnd)
            {
                if(CanCastleKingside())
                    return MoveType::CastleKingside;
            }
            else if(int2(2,0) == iEnd)
            {
                if(CanCastleQueenside())
                    return MoveType::CastleQueenside;
            }
        }
        break;
    case Color::Black:
        if(int2(4,7) == iBegin && Piece::King == piece)
        {
            if(int2(6,7) == iEnd)
            {
                if(CanCastleKingside())
                    return MoveType::CastleKingside;
            }
            else if(int2(2,7) == iEnd)
            {
                if(CanCastleQueenside())
                    return MoveType::CastleQueenside;
            }
        }
        break;
    default:
        SG_ASSUME_NOT_REACHED();
    }

    // Est-ce une prise en passant ?
    if(Piece::Pawn == piece && Board::GetSquareIndex(iEnd) == m_enPassantSquareIndex)
        if(CanCaptureEnPassant(iBegin, iEnd))
            return MoveType::EnPassant;

    return MoveType::None;
}
//=============================================================================
void State::CastleQueenside()
{
    SG_ASSERT(CanCastleQueenside());
    BeforeApplyingHalfMove();

    Color const colorToPlay = ColorToPlay();
    int const row = Color::White == colorToPlay ? 0 : 7;
    SG_ASSERT(Piece::King == m_board[int2(4, row)].GetPiece());
    SG_ASSERT(Piece::Rook == m_board[int2(0, row)].GetPiece());
    m_board[int2(4, row)].SetEmpty();
    m_board[int2(0, row)].SetEmpty();
    m_board[int2(2, row)].SetPiece(Piece::King, colorToPlay);
    m_board[int2(3, row)].SetPiece(Piece::Rook, colorToPlay);

    AfterApplyingHalfMove();
}
//=============================================================================
void State::CastleKingside()
{
    SG_ASSERT(CanCastleKingside());
    BeforeApplyingHalfMove();

    Color const colorToPlay = ColorToPlay();
    int const row = Color::White == colorToPlay ? 0 : 7;
    SG_ASSERT(Piece::King == m_board[int2(4, row)].GetPiece());
    SG_ASSERT(Piece::Rook == m_board[int2(7, row)].GetPiece());
    m_board[int2(4, row)].SetEmpty();
    m_board[int2(7, row)].SetEmpty();
    m_board[int2(6, row)].SetPiece(Piece::King, colorToPlay);
    m_board[int2(5, row)].SetPiece(Piece::Rook, colorToPlay);

    AfterApplyingHalfMove();
}
//=============================================================================
void State::CaptureEnPassant(int2 iBegin, int2 iEnd)
{
    SG_ASSERT(CanCaptureEnPassant(iBegin, iEnd));
    BeforeApplyingHalfMove();

    Color const colorToPlay = ColorToPlay();
    int2 const capturedPawnPos = int2(iEnd.x(), Color::White == colorToPlay ? 4 : 3);
    SG_ASSERT(Piece::Pawn == m_board[capturedPawnPos].GetPiece());
    SG_ASSERT(Piece::Pawn == m_board[iBegin].GetPiece());

    m_board[iBegin].SetEmpty();
    m_board[capturedPawnPos].SetEmpty();
    m_board[iEnd].SetPiece(Piece::Pawn, colorToPlay);

    AfterApplyingHalfMove();
}
//=============================================================================
void State::SimpleMove(int2 iBegin, int2 iEnd)
{
    SG_ASSERT(CanSimpleMove(iBegin, iEnd));
    BeforeApplyingHalfMove();

    Color const colorToPlay = ColorToPlay();
    SG_ASSERT(colorToPlay == m_board[iBegin].GetColor());
    Piece const piece = m_board[iBegin].GetPiece();
    m_board[iBegin].SetEmpty();
    m_board[iEnd].SetPiece(piece, colorToPlay);

    AfterApplyingHalfMove();
}
//=============================================================================
void State::MoveAndPromote(int2 iBegin, int2 iEnd, Piece iPieceID)
{
    SG_ASSERT(CanSimpleMove(iBegin, iEnd));
    BeforeApplyingHalfMove();

    Color const colorToPlay = ColorToPlay();
    SG_ASSERT(iEnd.y() == (Color::White == colorToPlay ? 7 : 0));
    SG_ASSERT(colorToPlay == m_board[iBegin].GetColor());
    SG_ASSERT(Piece::Pawn == m_board[iBegin].GetPiece());
    m_board[iBegin].SetEmpty();
    m_board[iEnd].SetPiece(iPieceID, colorToPlay);

    AfterApplyingHalfMove();
}
//=============================================================================
void State::Move(int2 iBegin, int2 iEnd)
{
    SG_ASSERT(CanMove(iBegin, iEnd));
    Color const colorToPlay = ColorToPlay();
    SG_ASSERT(colorToPlay == m_board[iBegin].GetColor());

    u8 enPassantSquareIndex_ToSet = u8(-1);

    Piece const piece = m_board[iBegin].GetPiece();

    // Si on déplace le roi ou une tour, il faut mettre à jour la capacité à roquer.
    // Si on déplace le roi, est-ce un roque ?
    // Si on déplace un pion de 2 case, mettre à jour l'état pour la capture "en passant".
    if(iBegin.y() == 0)
    {
        if(iBegin.x() == 0)
        {
            m_allowedCastleAndState &= ~WHITE_QUEENSIDE_CASTLE_ALLOWED;
        }
        else if(iBegin.x() == 4)
        {
            if(iEnd.x() == 6 && Piece::King == piece)
            {
                CastleKingside();
                m_allowedCastleAndState &= ~(WHITE_QUEENSIDE_CASTLE_ALLOWED | WHITE_KINGSIDE_CASTLE_ALLOWED);
                return;
            }
            else if(iEnd.x() == 2 && Piece::King == piece)
            {
                CastleQueenside();
                m_allowedCastleAndState &= ~(WHITE_QUEENSIDE_CASTLE_ALLOWED | WHITE_KINGSIDE_CASTLE_ALLOWED);
                return;
            }
            m_allowedCastleAndState &= ~(WHITE_QUEENSIDE_CASTLE_ALLOWED | WHITE_KINGSIDE_CASTLE_ALLOWED);
        }
        else if(iBegin.x() == 7)
        {
            m_allowedCastleAndState &= ~WHITE_KINGSIDE_CASTLE_ALLOWED;
        }
    }
    else if(iBegin.y() == 7)
    {
        if(iBegin.x() == 0)
        {
            m_allowedCastleAndState &= ~BLACK_QUEENSIDE_CASTLE_ALLOWED;
        }
        else if(iBegin.x() == 4)
        {
            if(iEnd.x() == 6 && Piece::King == piece)
            {
                CastleKingside();
                m_allowedCastleAndState &= ~(BLACK_QUEENSIDE_CASTLE_ALLOWED | BLACK_KINGSIDE_CASTLE_ALLOWED);
                return;
            }
            else if(iEnd.x() == 2 && Piece::King == piece)
            {
                CastleQueenside();
                m_allowedCastleAndState &= ~(BLACK_QUEENSIDE_CASTLE_ALLOWED | BLACK_KINGSIDE_CASTLE_ALLOWED);
                return;
            }
            m_allowedCastleAndState &= ~(BLACK_QUEENSIDE_CASTLE_ALLOWED | BLACK_KINGSIDE_CASTLE_ALLOWED);
        }
        else if(iBegin.x() == 7)
        {
            m_allowedCastleAndState &= ~BLACK_KINGSIDE_CASTLE_ALLOWED;
        }
    }
    else if(Piece::Pawn == piece)
    {
        SG_ASSERT_MSG(iEnd.y() != 7 && iEnd.y() != 0, "utiliser la méthode moveAndPromote pour la promotion des pions.");
        if(iBegin.y() == 1 && iEnd.y() == 3)
        {
            SG_ASSERT(Color::White == colorToPlay);
            SG_ASSERT(iBegin.x() == iEnd.x());
            enPassantSquareIndex_ToSet = checked_numcastable(Board::GetSquareIndex(int2(iBegin.x(), 2)));
        }
        else if(iBegin.y() == 6 && iEnd.y() == 4)
        {
            SG_ASSERT(Color::Black == colorToPlay);
            SG_ASSERT(iBegin.x() == iEnd.x());
            enPassantSquareIndex_ToSet = checked_numcastable(Board::GetSquareIndex(int2(iBegin.x(), 5)));
        }
        else
        {
            if(Board::GetSquareIndex(iEnd) == m_enPassantSquareIndex)
            {
                CaptureEnPassant(iBegin, iEnd);
                return;
            }
        }
    }

    SimpleMove(iBegin, iEnd);
    m_enPassantSquareIndex = enPassantSquareIndex_ToSet;
}
//=============================================================================
bool State::CanClaimFiftyMoveRule() const
{
    SG_ASSERT_NOT_IMPLEMENTED();
    return false;
}
//=============================================================================
bool State::CanClaimThreefoldRepetition() const
{
    SG_ASSERT_NOT_IMPLEMENTED();
    return false;
}
//=============================================================================
void State::ClaimFiftyMoveRule()
{
    SG_ASSERT_NOT_IMPLEMENTED();
}
//=============================================================================
void State::ClaimThreefoldRepetition()
{
    SG_ASSERT_NOT_IMPLEMENTED();
}
//=============================================================================
bool State::ComputeIsCheck(Board const& iBoard, Color iColorToPlay)
{
    // récuperer la position du roi de la couleur.
    int2 kingpos = int2(-1);
    bool king_found = false;
    while(!king_found)
    {
        ++kingpos.y();
        SG_ASSERT(kingpos.y() < 8);
        kingpos.x() = -1;
        while(!king_found && kingpos.x() < 7)
        {
            ++kingpos.x();
            SG_ASSERT(kingpos.x() < 8);
            Square const square = iBoard[kingpos];
            if(square.IsOccupied() && iColorToPlay == square.GetColor() && Piece::King == square.GetPiece())
                king_found = true;
        }
    }
    Color const nextColorToPlay = Color::White == iColorToPlay ? Color::Black : Color::White;

    // pour toutes les pièces adverses.
    //   vérifier si elles peuvent capturer le roi.
    for_range(int, r, 0, 8)
    {
        for_range(int, c, 0, 8)
        {
            int2 const pos = int2(r,c);
            Square const square = iBoard[pos];
            if(square.IsOccupied() && iColorToPlay != square.GetColor())
            {
                bool const canAttackKing = CanMove_IgnoreCheckAndEnPassantAndCastle(iBoard, nextColorToPlay, pos, kingpos);
                if(canAttackKing)
                    return true;
            }
        }
    }
    return false;
}
//=============================================================================
bool State::ComputeIsMat() const
{
    // pour chaque piece,
    //   tester tous les déplacements possible,
    // Note : On peut utiliser ce code pour compter ou parcourir les coups possibles.
    // Note : C'est un algo brute-force. On pourrait envisager de prendre en compte
    //   les propriétés de la piece rencontrée pour ne pas parcourir toutes les cases.
    for_range(int, r0, 0, 8)
    {
        for_range(int, c0, 0, 8)
        {
            int2 const pos0 = int2(r0,c0);
            Square const square = m_board[pos0];
            if(square.IsOccupied() && ColorToPlay() == square.GetColor())
            {
                for_range(int, r1, 0, 8)
                {
                    for_range(int, c1, 0, 8)
                    {
                        int2 const pos1 = int2(r1,c1);
                        if(pos1 == pos0)
                            continue;
                        bool const canMove = CanMove(pos0, pos1);
                        if(canMove)
                            return false;
                    }
                }
            }
        }
    }

    return true;
}
//=============================================================================
bool State::ComputeMaterialDraw(Board const& iBoard)
{
    u8 const WHITE_KNIGHT       = 0x01;
    u8 const WHITE_LIGHT_BISHOP = 0x02;
    u8 const WHITE_DARK_BISHOP  = 0x04;
    u8 const BLACK_KNIGHT       = 0x10;
    u8 const BLACK_LIGHT_BISHOP = 0x20;
    u8 const BLACK_DARK_BISHOP  = 0x40;
    u8 flags = 0;

    for_range(int, r, 0, 8)
    {
        for_range(int, c, 0, 8)
        {
            int2 const pos = int2(r,c);
            Square const square = iBoard[pos];
            if(square.IsOccupied())
            {
                switch(square.GetPiece())
                {
                case Piece::King:
                    break;
                case Piece::Queen:
                case Piece::Rook:
                case Piece::Pawn:
                    return false;
                case Piece::Knight:
                    switch(square.GetColor())
                    {
                    case Color::White:
                        if(flags != 0) return false;
                        else flags |= WHITE_KNIGHT;
                        break;
                    case Color::Black:
                        if(flags != 0) return false;
                        else flags |= BLACK_KNIGHT;
                        break;
                    default:
                        SG_ASSUME_NOT_REACHED();
                    }
                case Piece::Bishop:
                    if((flags & (WHITE_KNIGHT|BLACK_KNIGHT)) != 0)
                        return false;
                    switch(square.GetColor())
                    {
                    case Color::White:
                        if(Board::IsDarkSquare(pos))
                            flags |= WHITE_DARK_BISHOP;
                        else
                            flags |= WHITE_LIGHT_BISHOP;
                        break;
                    case Color::Black:
                        if(Board::IsDarkSquare(pos))
                            flags |= BLACK_DARK_BISHOP;
                        else
                            flags |= BLACK_LIGHT_BISHOP;
                        break;
                    default:
                        SG_ASSUME_NOT_REACHED();
                    }
                    if((flags & (WHITE_LIGHT_BISHOP)) != 0)
                    {
                        if((flags & (WHITE_DARK_BISHOP)) != 0)
                            return false;
                        if((flags & (BLACK_DARK_BISHOP)) != 0)
                            return false;
                    }
                    else if((flags & (WHITE_DARK_BISHOP)) != 0)
                    {
                        if((flags & (BLACK_LIGHT_BISHOP)) != 0)
                            return false;
                    }
                    else if((flags & (BLACK_LIGHT_BISHOP)) != 0)
                    {
                        if((flags & (BLACK_DARK_BISHOP)) != 0)
                            return false;
                    }
                }
            }
        }
    }
    return true;
}
//=============================================================================
void State::BeforeApplyingHalfMove()
{
    m_enPassantSquareIndex = u8(-1);
}
//=============================================================================
void State::AfterApplyingHalfMove()
{
    m_ply ++;
    // vérifier l'echec
    bool const isCheck = ComputeIsCheck(m_board, ColorToPlay());
    // vérifier le mat
    bool const isMat = ComputeIsMat();

    m_allowedCastleAndState = m_allowedCastleAndState & ~GAME_STATE_MASK;
    if(isCheck)
    {
        if(isMat)
            m_allowedCastleAndState = m_allowedCastleAndState | CHECK_MATE;
        else
            m_allowedCastleAndState = m_allowedCastleAndState | CHECK;
    }
    else if(isMat)
    {
        m_allowedCastleAndState = m_allowedCastleAndState | STALE_MATE;
    }
    else
    {
        // vérifier le matériel
        bool const isMaterialDraw = ComputeMaterialDraw(m_board);
        if(isMaterialDraw)
            m_allowedCastleAndState = m_allowedCastleAndState | MATERIAL_DRAW;
    }
}
//=============================================================================
}
}
}
