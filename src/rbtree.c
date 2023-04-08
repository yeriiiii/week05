#include "rbtree.h"
#include <stdlib.h>
#include <stdio.h>

// 정적 함수 선언
static void rotate_left(rbtree *t, node_t *n);
static void rotate_right(rbtree *t, node_t *n);
static void transplant(rbtree *t, node_t *u, node_t *v);
static node_t *subtree_min(const rbtree *t, node_t *n);
static void delete_node(node_t *NIL, node_t *n);
static void rbtree_insert_fixup(rbtree *t, node_t *n);
static void rbtree_erase_fixup(rbtree *t, node_t *n);

rbtree *new_rbtree(void)
{ // TODO: initialize struct if needed
  rbtree *newtree = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *NIL = (node_t *)calloc(1, sizeof(node_t));
  NIL->color = RBTREE_BLACK;
  newtree->root = NIL;
  newtree->nil = NIL;
  return newtree;
}

void delete_rbtree(rbtree *t)
{ // TODO: reclaim the tree nodes's memory
  delete_node(t->nil, t->root);
  free(t->nil);
  free(t);
}

static void delete_node(node_t *NIL, node_t *n)
{
  if (n != NIL)
  {
    delete_node(NIL, n->left);
    delete_node(NIL, n->right);
    free(n);
  }
}

node_t *rbtree_insert(rbtree *t, const key_t key)
{ // TODO: implement insert
  node_t *y = t->nil;
  node_t *x = t->root;
  // 삽입할 key의 위치 찾아가기
  while (x != t->nil)
  {
    y = x;
    if (key < x->key)
      x = x->left;
    else
      x = x->right;
  }
  // 새 노드 할당하기
  node_t *newNode = (node_t *)calloc(1, sizeof(node_t));
  newNode->key = key;
  newNode->left = newNode->right = t->nil;
  newNode->parent = y;
  newNode->color = RBTREE_RED;

  if (y == t->nil)
    t->root = newNode;
  else if (key < y->key)
    y->left = newNode;
  else
    y->right = newNode;

  rbtree_insert_fixup(t, newNode);

  return t->root;
}

static void rbtree_insert_fixup(rbtree *t, node_t *z)
{
  node_t *y;
  // [CASE 3-5] parent가 붉은색 노드인 경우: RBtree의 속성 깨짐
  while (z->parent->color == RBTREE_RED)
  {
    if (z->parent == z->parent->parent->left) // *z가 parent의 왼쪽 자식인 경우*
    {
      y = z->parent->parent->right; 
      if (y->color == RBTREE_RED) // [CASE 3] parent: RED, uncle: parent의 오른쪽 자식, RED
      {
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent;
      }
      else  // [CASE 4] parent: RED, uncle: BLACK (<)
      {
        if (z == z->parent->right)
        {
          z = z->parent;
          rotate_left(t, z);
        }
        // [CASE 5] parent: RED, uncle: BLACK (/)
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        rotate_right(t, z->parent->parent);
      }
    }
    else // *z가 parent의 오른쪽 자식인 경우*
    {
      y = z->parent->parent->left;
      if (y->color == RBTREE_RED) // [CASE 3] parent: RED, uncle: parent의 왼쪽 자식, RED
      {
        z->parent->color = RBTREE_BLACK;
        y->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        z = z->parent->parent;
      }
      else // [CASE 4] parent: RED, uncle: BLACK (>)
      {
        if (z == z->parent->left)
        {
          z = z->parent;
          rotate_right(t, z);
        }
        // [CASE 5] parent: RED, uncle: BLACK (\)
        z->parent->color = RBTREE_BLACK;
        z->parent->parent->color = RBTREE_RED;
        rotate_left(t, z->parent->parent);
      }
    }
  }
  // [CASE 1] root를 검은색으로 바꿔줌
  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{ // TODO: implement find
  node_t *p = t->root;
  while (p != t->nil)
  {
    if (p->key == key)
      return p;
    else if (p->key < key)
      p = p->right;
    else
      p = p->left;
  }
  return NULL;
}

node_t *rbtree_min(const rbtree *t)
{ // TODO: implement find
  node_t *p = t->root;
  node_t *parent = p;
  while (p != t->nil)
  {
    parent = p;
    p = p->left;
  }
  return parent;
}

node_t *rbtree_max(const rbtree *t)
{ // TODO: implement find
  node_t *p = t->root;
  node_t *parent = p;
  while (p != t->nil)
  {
    parent = p;
    p = p->right;
  }
  return parent;
}

int rbtree_erase(rbtree *t, node_t *p)
{ // TODO: implement erase
  node_t *NIL = t->nil;
  node_t *victim = p;
  color_t victim_color = victim->color; // 실제로 삭제되는 노드의 색 저장
  node_t *x;                            // victim의 자리를 대체하는 노드

  // 자식이 0개 또는 1개 : 자식이 노드 p 자리 대체
  if (p->left == NIL)
  {
    x = p->right;
    transplant(t, p, p->right);
  }
  else if (p->right == NIL)
  {
    x = p->left;
    transplant(t, p, p->left);
  }
  // 자식이 2개 : successor(오른쪽 서브트리에서 min)가 p 자리 대체
  else
  {
    victim = subtree_min(t, p->right);
    victim_color = victim->color;
    x = victim->right;
    if (victim->parent == p)
    { // victim 부모가 노드 p인 경우
      x->parent = victim;
    }
    else
    {
      transplant(t, victim, victim->right);
      victim->right = p->right;
      victim->right->parent = victim;
    }
    transplant(t, p, victim);
    victim->left = p->left;
    victim->left->parent = victim;
    victim->color = p->color;
  }
  if (victim_color == RBTREE_BLACK)
    rbtree_erase_fixup(t, x);

  free(p);
  return 0;
}

static void rbtree_erase_fixup(rbtree *t, node_t *x)
{
  while ((x != t->root) && (x->color == RBTREE_BLACK))
  {
    //***x가 왼쪽 자식일 때***
    if (x == x->parent->left)
    {
      node_t *w = x->parent->right; // w: x의 오른쪽 형제
      // [CASE 1] w가 RED
      if (w->color == RBTREE_RED)
      {
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        rotate_left(t, x->parent);
        w = x->parent->right;
      }
      // [CASE 2] w가 BLACK이면서, w의 왼쪽, 오른쪽 자녀가 모두 BLACK
      if ((w->left->color == RBTREE_BLACK) && (w->right->color == RBTREE_BLACK))
      {
        w->color = RBTREE_RED;
        x = x->parent;
      }
      else
      {
        // [CASE 3] w가 BLACK이면서, w의 왼쪽 자녀가 RED, 오른쪽 자녀가 BLACK
        if (w->right->color == RBTREE_BLACK)
        {
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          rotate_right(t, w);
          w = x->parent->right;
        }
        // [CASE 4] w가 BLACK이면서, w의 오른쪽 자녀가 RED
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        rotate_left(t, x->parent);
        x = t->root;
      }
    }
    // ***x가 오른쪽 자식일 때***
    else
    {
      node_t *w = x->parent->left; // w: x의 왼쪽 형제
      // [CASE 1] w가 RED
      if (w->color == RBTREE_RED)
      {
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        rotate_right(t, x->parent);
        w = x->parent->left;
      }
      // [CASE 2] w가 BLACK이면서, w의 왼쪽, 오른쪽 자녀가 모두 BLACK
      if ((w->left->color == RBTREE_BLACK) && (w->right->color == RBTREE_BLACK))
      {
        w->color = RBTREE_RED;
        x = x->parent;
      }
      else
      {
        // [CASE 3] w가 BLACK이면서, w의 오른쪽 자녀가 RED, 왼쪽 자녀가 BLACK
        if (w->left->color == RBTREE_BLACK)
        {
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          rotate_left(t, w);
          w = x->parent->left;
        }
        // [CASE 4] w가 BLACK이면서, w의 왼쪽 자녀가 RED
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        rotate_right(t, x->parent);
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK;
}

// 오름차순으로 탐색해서 그 결과를 n만큼 반환

// 전위 순회
// void preorder_search(const rbtree *t, node_t *p)
// {
//   if (p == t->nil)
//     return;
//   printf("%d-%d ", p->key, p->color);
//   preorder_search(t, p->left);
//   preorder_search(t, p->right);
// }

// 중위 순회
int inorder_search(const rbtree *t, node_t *p, int idx, key_t *arr, int n)
{
  if (p == t->nil || idx >= n)
    return idx;
  printf("%d ", p->key);
  idx = inorder_search(t, p->left, idx, arr, n);
  arr[idx++] = p->key;
  idx = inorder_search(t, p->right, idx, arr, n);
  return idx;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  inorder_search(t, t->root, 0, arr, n);
  return 0;
}

// Helper Functions
// static : 함수의 사용범위를 해당 파일 내로 한정 짓겠다

static void rotate_left(rbtree *t, node_t *x)
{
  node_t *y = x->right;
  x->right = y->left;
  if (y->left != t->nil)
  {
    y->left->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == t->nil)
  {
    t->root = y;
  }
  else if (x == x->parent->left)
  {
    x->parent->left = y;
  }
  else
  {
    x->parent->right = y;
  }
  y->left = x;
  x->parent = y;
}

static void rotate_right(rbtree *t, node_t *x)
{
  node_t *y = x->left;
  x->left = y->right;
  if (y->right != t->nil)
  {
    y->right->parent = x;
  }
  y->parent = x->parent;
  if (x->parent == t->nil)
  {
    t->root = y;
  }
  else if (x == x->parent->right)
  {
    x->parent->right = y;
  }
  else
    x->parent->left = y;
  y->right = x;
  x->parent = y;
}

static void transplant(rbtree *t, node_t *u, node_t *v)
{
  if (u->parent == t->nil)
    t->root = v;
  else if (u == u->parent->left)
    u->parent->left = v;
  else
    u->parent->right = v;
  v->parent = u->parent;
}

static node_t *subtree_min(const rbtree *t, node_t *n)
{
  node_t *parent = n;
  while (n != t->nil)
  {
    parent = n;
    n = n->left;
  }
  return parent;
}
