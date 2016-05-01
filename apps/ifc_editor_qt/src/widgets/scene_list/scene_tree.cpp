#include <QMenu>
#include <editor_window.h>
#include <iostream>
#include <algorithm>
#include "system/object_manager.h"
#include "system/ifc_types.h"
#include <widgets/scene_list/context_menus/scene_cmenu_factory.h>

using namespace std;

SceneTree::SceneTree(QWidget* parent) :
    QTreeWidget(parent)
{
    setupContextMenu();
    setupBinding();
    setupRootItems();

    showPoints = true;
}

SceneTree::~SceneTree(){
    allItems.clear();

    std::vector<RootItem*> tmpTopRootItems = topRootItems;
    for(unsigned int i = 0; i < tmpTopRootItems.size(); i++){
        delete tmpTopRootItems[i];
        topRootItems.erase(remove(topRootItems.begin(),
                                  topRootItems.end(), tmpTopRootItems[i]),
                       topRootItems.end());
    }
}

//-----------------------------//
//  PRIVATE
//-----------------------------//

//-----------//
//  SETUP
//-----------//

void SceneTree::setupBinding(){
    /*
    this->connect(this, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
                 this, SLOT(myitemActivated(QTreeWidgetItem*,int)));
                 */

    this->connect(this, SIGNAL(itemSelectionChanged()),
                 this, SLOT(myitemSelectionChanged()));
}

void SceneTree::setupRootItems(){
    topRootItems.push_back(new RootItem(RB_TORUS_TYPE, "Toruses"));
    topRootItems.push_back(new RootItem(RB_POINT_TYPE, "Points"));
    topRootItems.push_back(new RootItem(RB_BEZIER_TYPE, "Bezier Curves"));
    topRootItems.push_back(new RootItem(RB_BSPLINE_TYPE, "B-Splines"));
    topRootItems.push_back(new RootItem(RB_BSPLINE_INTERPOLATING_TYPE, "B-Splines Interpolating"));
}

void SceneTree::setupContextMenu(){
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowContextMenu(const QPoint&)));

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setAcceptDrops(true);
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::InternalMove);
}

//-----------//
//  UTILITY
//-----------//

Item* SceneTree::getRootItem(const Type& type){
    for(unsigned int i = 0;i < topRootItems.size(); i++){
        RootItem* rootItem = topRootItems[i];
        if(rootItem->type == type)
            return rootItem;
    }
    return NULL;
}

std::vector<Item*> SceneTree::treeItemsToItems(QList<QTreeWidgetItem *>& treeItems){
    std::vector<Item*> items;

    for(int i = 0; i < treeItems.size();i++){
        Item* item = getItemByTree(treeItems[i]);
        if(item != NULL)
            items.push_back(item);
    }
    return items;
}

QList<QTreeWidgetItem *> SceneTree::filterOutTopRootSelectedItems(){
    QList<QTreeWidgetItem *> filteredItems;

    QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
    for(int i = 0; i < selectedItems.size(); i++){
        bool add = true;
        for(unsigned int j = 0; j <  topRootItems.size(); j++){
            RootItem* rootItem = topRootItems[j];
            if(selectedItems[i] == rootItem->treeItem) {
                add = false;
            }
        }
        if(add)
            filteredItems.push_back(selectedItems[i]);
    }

    return filteredItems;
}

QList<QTreeWidgetItem *> SceneTree::filterSelectedItems(const Type& type){
    QList<QTreeWidgetItem *> filteredItems;

    QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
    for(int i = 0; i < selectedItems.size(); i++){
        Item* item  = getItemByTree(selectedItems[i]);
        if(item != NULL && item->type == type)
            filteredItems.push_back(selectedItems[i]);
    }

    return filteredItems;
}

void SceneTree::deleteItem(Item* item){
    item->erase(allItems);

    for(unsigned int i = 0;i < topRootItems.size();i++){
        RootItem* rootItem = topRootItems[i];

        bool hasRemoved = rootItem->removeChild(item);

        if(rootItem->isEmpty()){
            rootItem->destroy();
        }
        if(hasRemoved) return;
    }
}

//-----------//
//  EVENTS
//-----------//

void SceneTree::dropEvent(QDropEvent * event){
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        event->setDropAction(Qt::IgnoreAction);
        return;
    }
    QTreeWidgetItem* destinationTreeItem = itemFromIndex(index);
    Item* destItem = getItemByTree(destinationTreeItem);

    // Points to Spline
    if(destItem != NULL && canAddChildren(destItem->type)) {
        ObjectManager& objManager = ObjectManager::getInstance();
        QList<QTreeWidgetItem *> selectedPoints = filterSelectedItems(RB_POINT_TYPE);

        for(auto* selectedTreeItem : selectedPoints){
            Item* selectedItem = getItemByTree(selectedTreeItem);
            objManager.addChildItem(destItem, selectedItem);
        }
    }
    return;
}

//-----------------------------//
//  PUBLIC
//-----------------------------//

void SceneTree::moveItemUpWithinParent(Item* item){
    Item* parent = item->parent;
    if(parent == NULL)
        throw new std::invalid_argument("Swapping top level not implemented");

    int i = parent->treeItem->indexOfChild(item->treeItem);
    if (i <= 0) return;

    parent->treeItem->removeChild(item->treeItem);
    parent->treeItem->insertChild(i-1, item->treeItem);
}

void SceneTree::moveItemDownWithinParent(Item* item){
    Item* parent = item->parent;
    if(parent == NULL)
        throw new std::invalid_argument("Swapping top level not implemented");

    int i = parent->treeItem->indexOfChild(item->treeItem);
    if (i < 0 || i == parent->treeItem->childCount() - 1) return;

    parent->treeItem->removeChild(item->treeItem);
    parent->treeItem->insertChild(i+1, item->treeItem);
}


bool SceneTree::objectExists(std::string name){
    for(unsigned int i = 0; i < allItems.size(); i++){
        Item* item = allItems[i];
        if (name == item->object->getName() && !item->isClone())
            return true;
    }
    return false;
}

Item* SceneTree::getItemByName(std::string name){
    for(unsigned int i = 0; i < allItems.size(); i++){
        Item* item = allItems[i];
        if (name == item->object->getName() && !item->isClone()){
            return item;
        }
    }
    return NULL;
}

Item* SceneTree::getItemByTree(QTreeWidgetItem* treeItem){
    for(unsigned int i = 0; i < allItems.size(); i++){
        Item* item = allItems[i];
        if (treeItem == item->treeItem)
            return item;
    }
    return NULL;
}

Item* SceneTree::addObject(RenderObject * object, const Type& type){
    Item* item = new Item(object, type);

    allItems.push_back(item);
    for(unsigned int i = 0;i < topRootItems.size();i++){
        RootItem* topRootItem = topRootItems[i];
        if(type == topRootItem->type){
            topRootItem->init(this);
            topRootItem->addChild(item);
        }
    }

    if(type == RB_POINT_TYPE){
        object->setShow(showPoints);
    }

    return item;
}

SceneID SceneTree::deleteObject(Item* item){
    if(item == NULL) {throw std::invalid_argument("No such Object");}
    SceneID id = item->object->getID();

    deleteItem(item);

    return id;
}

void SceneTree::changeName(Item* item, string dstName){
    if(item == NULL) throw std::invalid_argument("No such Object");

    item->setName(dstName);
}


void SceneTree::addChildItem(QTreeWidgetItem* bezierTreeItem,
                                 QTreeWidgetItem* pointTreeItem){
    Item* bezierItem = getItemByTree(bezierTreeItem);
    Item* pointItem = getItemByTree(pointTreeItem);
    if(bezierItem == NULL || pointItem == NULL ||
            bezierItem->type != RB_BEZIER_TYPE ||
            pointItem->type != RB_POINT_TYPE)
        throw new std::invalid_argument("Invalid Item types");

   addChildItem(bezierItem, pointItem);
}

void SceneTree::addChildItem(Item* bezierItem, Item* pointItem){
    Item* cloneItem = pointItem->makeClone();
    cloneItem->type = RB_POINT_CLONE_TYPE;

    bezierItem->addChild(cloneItem);
    allItems.push_back(cloneItem);
}

std::vector<Item*> SceneTree::getSelectedItems(const Type& type){
    std::vector<Item*> items;

    QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
    for(int i = 0; i < selectedItems.size(); i++){
        Item* item  = getItemByTree(selectedItems[i]);
        if(item != NULL && item->type == type)
            items.push_back(item);
    }

    return items;
}

std::vector<RenderObject*> SceneTree::getSelectedObjects(){
    std::vector<RenderObject*> objects;

    QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
    for(int i = 0; i < selectedItems.size(); i++){
        Item* item  = getItemByTree(selectedItems[i]);
        if(item != NULL){
            if(!(std::find(objects.begin(),
                         objects.end(), item->object) != objects.end())) {
                objects.push_back(item->object);
            }
        }
    }
    return objects;
}

void SceneTree::activateObject(RenderObject * renderBody){
    for(unsigned int i = 0; i < allItems.size(); i++){
        Item* item = allItems[i];
        if(item->object == renderBody) {
            item->treeItem->setSelected(!item->treeItem->isSelected());
        }
    }
}

void SceneTree::deactivateAll(){
    for(unsigned int i = 0; i < allItems.size(); i++){
        Item* item = allItems[i];
        item->treeItem->setSelected(false);
    }
}

const std::vector<Item*>& SceneTree::getAllItems(){
    return this->allItems;
}

//-----------------------------//
//  SLOTS
//-----------------------------//

void SceneTree::ShowContextMenu(const QPoint& pos){
    QList<QTreeWidgetItem *> selectedItems = filterOutTopRootSelectedItems();
    vector<Item*> items = treeItemsToItems(selectedItems);

    QPoint globalPos = this->mapToGlobal(pos);

    auto getItemByTreePointer =
            [this](QTreeWidgetItem* item){
        return this->getItemByTree(item);
    };
    auto getRootItemPointer =
            [this](const Type& type){
        return this->getRootItem(type);
    };

    SceneCMenuFactory& factory = SceneCMenuFactory::getInstance();

    SceneContextMenu* menu = factory.getProperMenu(selectedItems,
                                              getItemByTreePointer,
                                              getRootItemPointer);
    if(menu == NULL) return;
    QAction* action = menu->show(globalPos);
    menu->handle(action, items);
}

/*
void SceneTree::myitemActivated(QTreeWidgetItem* treeItem, int column){
    Item* item = getItemByTree(treeItem);
    if(item == NULL) return;
}
*/
void SceneTree::myitemSelectionChanged(){
    QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
    ObjectManager& objManager = ObjectManager::getInstance();

    for(unsigned int i = 0;i < allItems.size(); i++){
        Item* item = allItems[i];
        if(item == NULL || item->object == NULL) continue;
        RenderObject * body = item->object;
        const SceneID& id = body->getID();
        objManager.setDeactive(id);
    }

    for(int i = 0; i < selectedItems.size();i++){
        Item* item = getItemByTree(selectedItems[i]);
        if(item == NULL) continue;
        objManager.setActive(item->object->getID());
        /*
        for(unsigned int i = 0; i < item->children.size(); i++){
            Item* child = item->children[i];
            child->treeItem->setSelected(true);
            objManager.setActive(child->object->getID());
        }*/
    }
    if(selectedItems.size() > 0){
        Item* item = getItemByTree(selectedItems[selectedItems.size()-1]);
        if(item != NULL)
            if(!canAddChildren(item->type))
                objManager.moveCross(item);
    }
}

void SceneTree::showPointsToogled(bool value){
    showPoints = value;
    for(unsigned int i = 0; i < this->allItems.size(); i++){
        Item* item = allItems[i];
        if(item->type == RB_POINT_TYPE){
            item->object->setShow(value);
        }
    }
}

#include "moc_scene_tree.cpp"
