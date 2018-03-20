//========================================================================
//
// GooHash.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2017 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "gmem.h"
#include "GooString.h"
#include "GooHash.h"
#include "GooLikely.h"

//------------------------------------------------------------------------

struct GooHashBucket {
  GooString *key;
  union {
    void *p;
    int i;
  } val;
  GooHashBucket *next;
};

struct GooHashIter {
  int h;
  GooHashBucket *p;
};

//------------------------------------------------------------------------

GooHash::GooHash(GBool deleteKeysA) {
  int h;

  deleteKeys = deleteKeysA;
  size = 7;
  tab = (GooHashBucket **)gmallocn(size, sizeof(GooHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = nullptr;
  }
  len = 0;
}

GooHash::~GooHash() {
  GooHashBucket *p;
  int h;

  for (h = 0; h < size; ++h) {
    while (tab[h]) {
      p = tab[h];
      tab[h] = p->next;
      if (deleteKeys) {
	delete p->key;
      }
      delete p;
    }
  }
  gfree(tab);
}

void GooHash::add(GooString *key, void *val) {
  GooHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GooHashBucket;
  p->key = key;
  p->val.p = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GooHash::add(GooString *key, int val) {
  GooHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GooHashBucket;
  p->key = key;
  p->val.i = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GooHash::replace(GooString *key, void *val) {
  GooHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.p = val;
    if (deleteKeys) {
      delete key;
    }
  } else {
    add(key, val);
  }
}

void GooHash::replace(GooString *key, int val) {
  GooHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.i = val;
    if (deleteKeys) {
      delete key;
    }
  } else {
    add(key, val);
  }
}

void *GooHash::lookup(GooString *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return nullptr;
  }
  return p->val.p;
}

int GooHash::lookupInt(GooString *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GooHash::lookup(const char *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return nullptr;
  }
  return p->val.p;
}

int GooHash::lookupInt(const char *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GooHash::remove(GooString *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return nullptr;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GooHash::removeInt(GooString *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void *GooHash::remove(const char *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return nullptr;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GooHash::removeInt(const char *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void GooHash::startIter(GooHashIter **iter) {
  *iter = new GooHashIter;
  (*iter)->h = -1;
  (*iter)->p = nullptr;
}

GBool GooHash::getNext(GooHashIter **iter, GooString **key, void **val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = nullptr;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.p;
  return gTrue;
}

GBool GooHash::getNext(GooHashIter **iter, GooString **key, int *val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = nullptr;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.i;
  return gTrue;
}

void GooHash::killIter(GooHashIter **iter) {
  delete *iter;
  *iter = nullptr;
}

void GooHash::expand() {
  GooHashBucket **oldTab;
  GooHashBucket *p;
  int oldSize, h, i;

  oldSize = size;
  oldTab = tab;
  size = 2*size + 1;
  tab = (GooHashBucket **)gmallocn(size, sizeof(GooHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = nullptr;
  }
  for (i = 0; i < oldSize; ++i) {
    while (oldTab[i]) {
      p = oldTab[i];
      oldTab[i] = oldTab[i]->next;
      h = hash(p->key);
      p->next = tab[h];
      tab[h] = p;
    }
  }
  gfree(oldTab);
}

GooHashBucket *GooHash::find(GooString *key, int *h) {
  GooHashBucket *p;

  if (unlikely(!key))
    return nullptr;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return nullptr;
}

GooHashBucket *GooHash::find(const char *key, int *h) {
  GooHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return nullptr;
}

int GooHash::hash(GooString *key) {
  const char *p;
  unsigned int h;
  int i;

  h = 0;
  for (p = key->getCString(), i = 0; i < key->getLength(); ++p, ++i) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}

int GooHash::hash(const char *key) {
  const char *p;
  unsigned int h;

  h = 0;
  for (p = key; *p; ++p) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}
