#ifndef ToolsUI_TreeView_H
#define ToolsUI_TreeView_H

#include <Core/Config.h>
#if !SG_ENABLE_TOOLS
#error "This file should be included only with SG_ENABLE_TOOLS"
#endif

#include "Common.h"
#include <UserInterface/Container.h>
#include <UserInterface/Component.h>
#include <UserInterface/FitMode.h>
#include <UserInterface/Focusable.h>
#include <UserInterface/FrameProperty.h>
#include <UserInterface/ListLayout.h>
#include <UserInterface/Movable.h>
#include <UserInterface/SensitiveArea.h>
#include <UserInterface/Text.h>
#include <Core/Cast.h>
#include <Core/Observer.h>

namespace sg {
namespace ui {
}
namespace toolsui {
//=============================================================================
class Label;
//=============================================================================
// TODO: make a generic ui::Foldable
class Foldable : public ui::Container
               , public ui::IMovable
               , public UnsharableObservable<Foldable>
               , private ui::IFocusable
               , private ui::ISensitiveAreaListener
{
    PARENT_SAFE_COUNTABLE(ui::Container)
    typedef ui::Container parent_type;
    typedef ui::IFocusable focusable_parent_type;
public:
    enum Framing
    {
        NoFrame,
        FrameHeaderOnly,
        FrameAll,
    };
    Foldable();
    Foldable(ui::FitMode2 iFitMode);
    ~Foldable();

    void SetFraming(Framing iFraming) { m_framing = iFraming; }
    void Unfold(bool iUnfold);
    void FlipFolding() { Unfold(!m_isUnfolded); }
    bool IsUnfolded() const { return m_isUnfolded; }
    void SetNonFoldableContent(ui::IMovable* iContent);
    void SetFoldableContent(ui::IMovable* iContent);

    void SetFrameProperty(ui::FrameProperty const& iFrameProperty)  { InvalidatePlacement(); m_frameProperty = iFrameProperty; }
    void SetFitMode(ui::FitMode2 iFitMode) { InvalidatePlacement(); m_fitMode = iFitMode; }
protected:
    virtual void VirtualResetOffset() override;
    virtual void VirtualAddOffset(float2 const& iOffset) override;
    virtual void VirtualOnPointerEvent(ui::PointerEventContext const& iContext, ui::PointerEvent const& iPointerEvent) override;
    virtual void OnButtonUpToDown(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void OnButtonDownToUp(SG_UI_SENSITIVE_AREA_LISTENER_PARAMETERS_ONE_BUTTON) override;
    virtual void VirtualOnDraw(ui::DrawContext const& iContext) override;
    virtual void VirtualUpdatePlacement() override;
    virtual void VirtualOnFocusableEvent(ui::FocusableEvent const& iFocusableEvent) override;
    virtual bool VirtualMoveFocusReturnHasMoved(ui::FocusDirection iDirection) override;
    virtual ui::Component* VirtualAsComponent() override { return this; }
    virtual IFocusable* VirtualAsFocusableIFP() override { return this; }
    virtual void VirtualOnInsertInUI() override { parent_type::VirtualOnInsertInUI(); OnInsertFocusableInUI(); }
    virtual void VirtualOnRemoveFromUI() override { OnRemoveFocusableFromUI(); parent_type::VirtualOnRemoveFromUI(); }
    ui::SensitiveArea const& SensitiveArea() const { return m_sensitiveArea; }
private:
    bool IsHover() const { return m_sensitiveArea.IsPointerInsideFree(); }
    bool IsClicked() const { return m_sensitiveArea.IsClickedValidable(0); }
#if SG_ENABLE_ASSERT
    void CheckConstraintsOnContent(ui::IMovable* iContent);
#endif
private:
    ui::FrameProperty m_frameProperty;
    ui::SensitiveArea m_sensitiveArea;
    ui::BoxArea m_boxArea;
    refptr<ui::HorizontalListLayout> m_hlist;
    refptr<Label> m_foldIcon;
    ui::IMovableHolder m_nonFoldableContent;
    ui::IMovableHolder m_foldableContent;
    ui::FitMode2 m_fitMode;
    Framing m_framing;
    bool m_isUnfolded;
};
//=============================================================================
class FoldableList : public Foldable
{
    typedef Foldable parent_type;
public:
    FoldableList();
    FoldableList(ui::FitMode2 iFitMode);
    ~FoldableList();
    void AppendFoldableContent(ui::IMovable* iItem) { m_list->AppendItem(iItem); }
    void RemoveFoldableContent(ui::Component* iItem) { m_list->RemoveItem(iItem); }
    void RemoveAllFoldableContent() { m_list->RemoveAllItems(); }
    ui::Component* GetFocusedItemIFP() const { return m_list->GetFocusedItemIFP(); }
private:
    SG_HIDE_METHOD_FROM_PARENT(SetFoldableContent);
private:
    refptr<ui::VerticalListLayout> m_list;
};
//=============================================================================
// A TreeView generates automaticaly foldable items which display node names.
// However, each node may me replaced by a user provided node. This is often
// the case for terminal nodes. In that case, the user provided node should
// display its own name.
class TreeView : public ui::VerticalListLayout
{
    typedef ui::VerticalListLayout parent_type;
public:
    TreeView();
    ~TreeView();

    void Reserve(size_t iCapacity) { m_nodes.Reserve(iCapacity); }
    void Insert(std::string const& iPath, ui::IMovable* iContent);
    void Insert(ArrayView<std::string const> iPath, ui::IMovable* iContent);
    ui::IMovable* GetIFP(std::string const& iPath);
    ui::IMovable* GetIFP(ArrayView<std::string const> iPath);
    void Remove(std::string const& iPath);
    void Remove(ArrayView<std::string const> iPath);
protected:
    virtual void VirtualUpdatePlacement() override;
private:
    struct Node
    {
        std::string name;
        ArrayList<size_t> children;
        refptr<FoldableList> foldableList;
        refptr<Label> label;
        ui::IMovableHolder content;
        size_t nextFreeNodeIndex = all_ones;
        bool needRefresh = false;
    };
    void ExplicitPath(ArrayList<std::string>& oPath, std::string const& iPath);
    bool InsertReturnMustRefreshChildren(ArrayList<size_t>& ioChildren, ArrayView<std::string const> iPath, ui::IMovable* iContent);
    ui::IMovable* GetIFP(ArrayList<size_t>& ioChildren, ArrayView<std::string const> iPath);
    bool RemoveReturnMustRefreshChildren(ArrayList<size_t>& ioChildren, ArrayView<std::string const> iPath);
    void RequestRefreshNode(size_t iNodeIndex);
    void RequestRefresh();
    void SortChildren(ArrayList<size_t>& ioChildren);
    void RefreshNode(Node& iNode);
    void Refresh();
    void FreeNode(size_t iIndex);
    SG_HIDE_METHOD_FROM_PARENT(AppendItem);
    SG_HIDE_METHOD_FROM_PARENT(AppendExpansibleItem);
private:
    ArrayList<size_t> m_children;
    ArrayList<Node> m_nodes;
    size_t m_freeNodeIndex = all_ones;
    ArrayList<size_t> m_nodesToRefresh;
    bool m_needRefresh;
};
//=============================================================================
}
}

#endif
