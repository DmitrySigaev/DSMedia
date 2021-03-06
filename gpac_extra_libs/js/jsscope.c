/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

/*
 * JS symbol tables.
 */
#include "jsstddef.h"
#include <stdlib.h>
#include <string.h>
#include "jstypes.h"
#include "jsarena.h"
#include "jsbit.h"
#include "jsclist.h"
#include "jsdhash.h"
#include "jsutil.h" /* Added by JSIFY */
#include "jsapi.h"
#include "jsatom.h"
#include "jscntxt.h"
#include "jsdbgapi.h"
#include "jslock.h"
#include "jsnum.h"
#include "jsscope.h"
#include "jsstr.h"

JSScope *
js_GetMutableScope(JSContext *cx, JSObject *obj)
{
    JSScope *scope, *newscope;

    scope = OBJ_SCOPE(obj);
    JS_ASSERT(JS_IS_SCOPE_LOCKED(scope));
    if (scope->object == obj)
        return scope;
    newscope = js_NewScope(cx, 0, scope->map.ops, LOCKED_OBJ_GET_CLASS(obj),
                           obj);
    if (!newscope)
        return NULL;
    JS_LOCK_SCOPE(cx, newscope);
    obj->map = js_HoldObjectMap(cx, &newscope->map);
    scope = (JSScope *) js_DropObjectMap(cx, &scope->map, obj);
    JS_TRANSFER_SCOPE_LOCK(cx, scope, newscope);
    return newscope;
}

/*
 * JSScope uses multiplicative hashing, _a la_ jsdhash.[ch], but specialized
 * to minimize footprint.  But if a scope has fewer than SCOPE_HASH_THRESHOLD
 * entries, we use linear search and avoid allocating scope->table.
 */
#define SCOPE_HASH_THRESHOLD    6

#define MIN_SCOPE_SIZE_LOG2     4
#define MIN_SCOPE_SIZE          JS_BIT(MIN_SCOPE_SIZE_LOG2)

#define SCOPE_TABLE_SIZE(n)     ((n) * sizeof(JSScopeProperty **))
#define MIN_SCOPE_TABLE_SIZE    SCOPE_TABLE_SIZE(MIN_SCOPE_SIZE)

static void
InitMinimalScope(JSScope *scope)
{
    scope->hashShift = JS_DHASH_BITS - MIN_SCOPE_SIZE_LOG2;
    scope->sizeLog2 = MIN_SCOPE_SIZE_LOG2;
    scope->entryCount = scope->removedCount = 0;
    scope->table = NULL;
    scope->lastProp = NULL;
}

static JSBool
CreateScopeTable(JSScope *scope)
{
    int sizeLog2;
    JSScopeProperty *sprop, **spp;

    JS_ASSERT(!scope->table);
    JS_ASSERT(scope->lastProp);

    if (scope->entryCount > SCOPE_HASH_THRESHOLD) {
        /*
         * Ouch: calloc failed at least once already -- let's try again,
         * overallocating to hold at least twice the current population.
         */
        sizeLog2 = JS_CeilingLog2(2 * scope->entryCount);
    } else {
        sizeLog2 = MIN_SCOPE_SIZE_LOG2;
    }

    scope->table = (JSScopeProperty **)
        calloc(JS_BIT(sizeLog2), sizeof(JSScopeProperty *));
    if (!scope->table)
        return JS_FALSE;

    scope->sizeLog2 = sizeLog2;
    scope->hashShift = JS_DHASH_BITS - sizeLog2;
    for (sprop = scope->lastProp; sprop; sprop = sprop->parent) {
        spp = js_SearchScope(scope, sprop->id, JS_TRUE);
        SPROP_STORE_PRESERVING_COLLISION(spp, sprop);
    }
    return JS_TRUE;
}

JSScope *
js_NewScope(JSContext *cx, jsrefcount nrefs, JSObjectOps *ops, JSClass *clasp,
            JSObject *obj)
{
    JSScope *scope;

    scope = (JSScope *) JS_malloc(cx, sizeof(JSScope));
    if (!scope)
        return NULL;

    js_InitObjectMap(&scope->map, nrefs, ops, clasp);
    scope->object = obj;
    InitMinimalScope(scope);

#ifdef JS_THREADSAFE
    scope->ownercx = cx;
    memset(&scope->lock, 0, sizeof scope->lock);

    /*
     * Set u.link = NULL, not u.count = 0, in case the target architecture's
     * null pointer has a non-zero integer representation.
     */
    scope->u.link = NULL;

#ifdef DEBUG
    scope->file[0] = scope->file[1] = scope->file[2] = scope->file[3] = NULL;
    scope->line[0] = scope->line[1] = scope->line[2] = scope->line[3] = 0;
#endif
#endif

    JS_RUNTIME_METER(cx->runtime, liveScopes);
    JS_RUNTIME_METER(cx->runtime, totalScopes);
    return scope;
}

#ifdef DEBUG_SCOPE_COUNT
extern void
js_unlog_scope(JSScope *scope);
#endif

void
js_DestroyScope(JSContext *cx, JSScope *scope)
{
#ifdef DEBUG_SCOPE_COUNT
    js_unlog_scope(scope);
#endif

#ifdef JS_THREADSAFE
    /*
     * Scope must be single-threaded at this point, so set scope->ownercx.
     * This also satisfies the JS_IS_SCOPE_LOCKED assertions in the _clear
     * implementations.
     */
    JS_ASSERT(scope->u.count == 0);
    scope->ownercx = cx;
    js_FinishLock(&scope->lock);
#endif
    if (scope->table)
        JS_free(cx, scope->table);

#ifdef DEBUG
    JS_LOCK_RUNTIME_VOID(cx->runtime,
                         cx->runtime->liveScopeProps -= scope->entryCount);
#endif
    JS_RUNTIME_UNMETER(cx->runtime, liveScopes);
    JS_free(cx, scope);
}

#ifdef DEBUG_brendan
typedef struct JSScopeStats {
    jsrefcount          searches;
    jsrefcount          steps;
    jsrefcount          hits;
    jsrefcount          misses;
    jsrefcount          stepHits;
    jsrefcount          stepMisses;
    jsrefcount          adds;
    jsrefcount          redundantAdds;
    jsrefcount          addFailures;
    jsrefcount          changeFailures;
    jsrefcount          compresses;
    jsrefcount          grows;
    jsrefcount          removes;
    jsrefcount          removeFrees;
    jsrefcount          uselessRemoves;
    jsrefcount          shrinks;
} JSScopeStats;

JS_FRIEND_DATA(JSScopeStats) js_scope_stats;

# define METER(x)       JS_ATOMIC_INCREMENT(&js_scope_stats.x)
#else
# define METER(x)       /* nothing */
#endif

/*
 * Double hashing needs the second hash code to be relatively prime to table
 * size, so we simply make hash2 odd.  The inputs to multiplicative hash are
 * the golden ratio, expressed as a fixed-point 32 bit fraction, and the int
 * property index or named property's atom number (observe that most objects
 * have either no indexed properties, or almost all indexed and a few names,
 * so collisions between index and atom number are unlikely).
 */
#define SCOPE_HASH0(id)                 (HASH_ID(id) * JS_GOLDEN_RATIO)
#define SCOPE_HASH1(hash0,shift)        ((hash0) >> (shift))
#define SCOPE_HASH2(hash0,log2,shift)   ((((hash0) << (log2)) >> (shift)) | 1)

JS_FRIEND_API(JSScopeProperty **)
js_SearchScope(JSScope *scope, jsid id, JSBool adding)
{
    JSHashNumber hash0, hash1, hash2;
    int hashShift, sizeLog2;
    JSScopeProperty *stored, *sprop, **spp, **firstRemoved;
    uint32 sizeMask;

    METER(searches);
    if (!scope->table) {
        /* Not enough properties to justify hashing: search from lastProp. */
        JS_ASSERT(!SCOPE_HAD_MIDDLE_DELETE(scope));
        for (spp = &scope->lastProp; (sprop = *spp); spp = &sprop->parent) {
            if (sprop->id == id) {
                METER(hits);
                return spp;
            }
        }
        METER(misses);
        return spp;
    }

    /* Compute the primary hash address. */
    hash0 = SCOPE_HASH0(id);
    hashShift = scope->hashShift;
    hash1 = SCOPE_HASH1(hash0, hashShift);
    spp = scope->table + hash1;

    /* Miss: return space for a new entry. */
    stored = *spp;
    if (SPROP_IS_FREE(stored)) {
        METER(misses);
        return spp;
    }

    /* Hit: return entry. */
    sprop = SPROP_CLEAR_COLLISION(stored);
    if (sprop && sprop->id == id) {
        METER(hits);
        return spp;
    }

    /* Collision: double hash. */
    sizeLog2 = scope->sizeLog2;
    hash2 = SCOPE_HASH2(hash0, sizeLog2, hashShift);
    sizeMask = JS_BITMASK(sizeLog2);

    /* Save the first removed entry pointer so we can recycle it if adding. */
    if (SPROP_IS_REMOVED(stored)) {
        firstRemoved = spp;
    } else {
        firstRemoved = NULL;
        if (adding && !SPROP_HAD_COLLISION(stored))
            SPROP_FLAG_COLLISION(spp, sprop);
    }

    for (;;) {
        METER(steps);
        hash1 -= hash2;
        hash1 &= sizeMask;
        spp = scope->table + hash1;

        stored = *spp;
        if (SPROP_IS_FREE(stored)) {
            METER(stepMisses);
            return (adding && firstRemoved) ? firstRemoved : spp;
        }

        sprop = SPROP_CLEAR_COLLISION(stored);
        if (sprop && sprop->id == id) {
            METER(stepHits);
            return spp;
        }

        if (SPROP_IS_REMOVED(stored)) {
            if (!firstRemoved)
                firstRemoved = spp;
        } else {
            if (adding && !SPROP_HAD_COLLISION(stored))
                SPROP_FLAG_COLLISION(spp, sprop);
        }
    }

    /* NOTREACHED */
    return NULL;
}

static JSBool
ChangeScope(JSContext *cx, JSScope *scope, int change)
{
    int oldlog2, newlog2;
    uint32 oldsize, newsize, nbytes;
    JSScopeProperty **table, **oldtable, **spp, **oldspp, *sprop;

    /* Grow, shrink, or compress by changing scope->table. */
    oldlog2 = scope->sizeLog2;
    newlog2 = oldlog2 + change;
    oldsize = JS_BIT(oldlog2);
    newsize = JS_BIT(newlog2);
    nbytes = newsize * sizeof(JSScopeProperty);
    table = (JSScopeProperty **) calloc(nbytes, 1);
    if (!table) {
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }

    /* Now that we have a new table allocated, update scope members. */
    scope->hashShift = JS_DHASH_BITS - newlog2;
    scope->sizeLog2 = newlog2;
    scope->removedCount = 0;
    oldtable = scope->table;
    scope->table = table;

    /* Copy only live entries, leaving removed and free ones behind. */
    for (oldspp = oldtable; oldsize != 0; oldspp++) {
        sprop = SPROP_FETCH(oldspp);
        if (sprop) {
            spp = js_SearchScope(scope, sprop->id, JS_TRUE);
            JS_ASSERT(SPROP_IS_FREE(*spp));
            *spp = sprop;
        }
        oldsize--;
    }

    /* Finally, free the old table storage. */
    JS_free(cx, oldtable);
    return JS_TRUE;
}

/*
 * Take care to exclude the mark and duplicate bits, in case we're called from
 * the GC, or we are searching for a property that has not yet been flagged as
 * a duplicate when making a duplicate formal parameter.
 */
#define SPROP_FLAGS_NOT_MATCHED (SPROP_MARK | SPROP_IS_DUPLICATE)

JS_STATIC_DLL_CALLBACK(JSDHashNumber)
js_HashScopeProperty(JSDHashTable *table, const void *key)
{
    const JSScopeProperty *sprop = (const JSScopeProperty *)key;
    JSDHashNumber hash;
    JSPropertyOp gsop;

    /* Accumulate from least to most random so the low bits are most random. */
    hash = 0;
    gsop = sprop->getter;
    if (gsop)
        hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4) ^ (jsword)gsop;
    gsop = sprop->setter;
    if (gsop)
        hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4) ^ (jsword)gsop;

    hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4)
           ^ (sprop->flags & ~SPROP_FLAGS_NOT_MATCHED);

    hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4) ^ sprop->attrs;
    hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4) ^ sprop->shortid;
    hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4) ^ sprop->slot;
    hash = (hash >> (JS_DHASH_BITS - 4)) ^ (hash << 4) ^ sprop->id;
    return hash;
}

#define SPROP_MATCH(sprop, child)                                             \
    SPROP_MATCH_PARAMS(sprop, (child)->id, (child)->getter, (child)->setter,  \
                       (child)->slot, (child)->attrs, (child)->flags,         \
                       (child)->shortid)

#define SPROP_MATCH_PARAMS(sprop, aid, agetter, asetter, aslot, aattrs,       \
                           aflags, ashortid)                                  \
    ((sprop)->id == (aid) &&                                                  \
     SPROP_MATCH_PARAMS_AFTER_ID(sprop, agetter, asetter, aslot, aattrs,      \
                                 aflags, ashortid))

#define SPROP_MATCH_PARAMS_AFTER_ID(sprop, agetter, asetter, aslot, aattrs,   \
                                    aflags, ashortid)                         \
    ((sprop)->getter == (agetter) &&                                          \
     (sprop)->setter == (asetter) &&                                          \
     (sprop)->slot == (aslot) &&                                              \
     (sprop)->attrs == (aattrs) &&                                            \
     (((sprop)->flags ^ (aflags)) & ~SPROP_FLAGS_NOT_MATCHED) == 0 &&         \
     (sprop)->shortid == (ashortid))

JS_STATIC_DLL_CALLBACK(JSBool)
js_MatchScopeProperty(JSDHashTable *table,
                      const JSDHashEntryHdr *hdr,
                      const void *key)
{
    const JSPropertyTreeEntry *entry = (const JSPropertyTreeEntry *)hdr;
    const JSScopeProperty *sprop = entry->child;
    const JSScopeProperty *kprop = (const JSScopeProperty *)key;

    return SPROP_MATCH(sprop, kprop);
}

static JSDHashTableOps PropertyTreeHashOps = {
    JS_DHashAllocTable,
    JS_DHashFreeTable,
    JS_DHashGetKeyStub,
    js_HashScopeProperty,
    js_MatchScopeProperty,
    JS_DHashMoveEntryStub,
    JS_DHashClearEntryStub,
    JS_DHashFinalizeStub,
    NULL
};

/*
 * A property tree node on rt->propertyFreeList overlays the following prefix
 * struct on JSScopeProperty.
 */
typedef struct FreeNode {
    jsid                id;
    JSScopeProperty     *next;
    JSScopeProperty     **prevp;
} FreeNode;

#define FREENODE(sprop) ((FreeNode *) (sprop))

#define FREENODE_INSERT(list, sprop)                                          \
    JS_BEGIN_MACRO                                                            \
        FREENODE(sprop)->next = (list);                                       \
        FREENODE(sprop)->prevp = &(list);                                     \
        if (list)                                                             \
            FREENODE(list)->prevp = &FREENODE(sprop)->next;                   \
        (list) = (sprop);                                                     \
    JS_END_MACRO

#define FREENODE_REMOVE(sprop)                                                \
    JS_BEGIN_MACRO                                                            \
        *FREENODE(sprop)->prevp = FREENODE(sprop)->next;                      \
        if (FREENODE(sprop)->next)                                            \
            FREENODE(FREENODE(sprop)->next)->prevp = FREENODE(sprop)->prevp;  \
    JS_END_MACRO

/* NB: Called with the runtime lock held. */
static JSScopeProperty *
NewScopeProperty(JSRuntime *rt)
{
    JSScopeProperty *sprop;

    sprop = rt->propertyFreeList;
    if (sprop) {
        FREENODE_REMOVE(sprop);
    } else {
        JS_ARENA_ALLOCATE_CAST(sprop, JSScopeProperty *,
                               &rt->propertyArenaPool,
                               sizeof(JSScopeProperty));
        if (!sprop)
            return NULL;
    }

    JS_RUNTIME_METER(rt, livePropTreeNodes);
    JS_RUNTIME_METER(rt, totalPropTreeNodes);
    return sprop;
}

#define CHUNKY_KIDS_TAG         ((jsuword)1)
#define KIDS_IS_CHUNKY(kids)    ((jsuword)(kids) & CHUNKY_KIDS_TAG)
#define KIDS_TO_CHUNK(kids)     ((PropTreeKidsChunk *)                        \
                                 ((jsuword)(kids) & ~CHUNKY_KIDS_TAG))
#define CHUNK_TO_KIDS(chunk)    ((JSScopeProperty *)                          \
                                 ((jsuword)(chunk) | CHUNKY_KIDS_TAG))
#define MAX_KIDS_PER_CHUNK      10

typedef struct PropTreeKidsChunk PropTreeKidsChunk;

struct PropTreeKidsChunk {
    JSScopeProperty     *kids[MAX_KIDS_PER_CHUNK];
    PropTreeKidsChunk   *next;
};

static PropTreeKidsChunk *
NewPropTreeKidsChunk(JSRuntime *rt)
{
    PropTreeKidsChunk *chunk;

    chunk = calloc(1, sizeof *chunk);
    if (!chunk)
        return NULL;
    JS_ASSERT(((jsuword)chunk & CHUNKY_KIDS_TAG) == 0);
    JS_RUNTIME_METER(rt, propTreeKidsChunks);
    return chunk;
}

static void
DestroyPropTreeKidsChunk(JSRuntime *rt, PropTreeKidsChunk *chunk)
{
    JS_RUNTIME_UNMETER(rt, propTreeKidsChunks);
    free(chunk);
}

/* NB: Called with the runtime lock held. */
static JSBool
InsertPropertyTreeChild(JSRuntime *rt, JSScopeProperty *parent,
                        JSScopeProperty *child)
{
    JSPropertyTreeEntry *entry;
    JSScopeProperty **childp, *kids, *sprop;
    PropTreeKidsChunk *chunk, **chunkp;
    uintN i;

    JS_ASSERT(!parent || child->parent != parent);

    if (!parent) {
        entry = (JSPropertyTreeEntry *)
            JS_DHashTableOperate(&rt->propertyTreeHash, child, JS_DHASH_ADD);
        if (!entry)
            return JS_FALSE;
        childp = &entry->child;
        sprop = *childp;
        if (!sprop) {
            *childp = child;
        } else {
            /*
             * A "Duplicate child" case.
             *
             * We can't do away with child, as at least one live scope entry
             * still points at it.  What's more, that scope's lastProp chains
             * through an ancestor line to reach child, and js_Enumerate and
             * others count on this linkage.  We must leave child out of the
             * hash table, and not require it to be there when we eventually
             * GC it (see RemovePropertyTreeChild, below).
             *
             * It is necessary to leave the duplicate child out of the hash
             * table to preserve entry uniqueness.  It is safe to leave the
             * child out of the hash table (unlike the duplicate child cases
             * below), because the child's parent link will be null, which
             * can't dangle.
             */
            JS_ASSERT(sprop != child && SPROP_MATCH(sprop, child));
            JS_RUNTIME_METER(rt, duplicatePropTreeNodes);
        }
    } else {
        childp = &parent->kids;
        kids = *childp;
        if (kids) {
            if (KIDS_IS_CHUNKY(kids)) {
                chunk = KIDS_TO_CHUNK(kids);
                do {
                    for (i = 0; i < MAX_KIDS_PER_CHUNK; i++) {
                        childp = &chunk->kids[i];
                        sprop = *childp;
                        if (!sprop)
                            goto insert;

                        JS_ASSERT(sprop != child);
                        if (SPROP_MATCH(sprop, child)) {
                            /*
                             * Duplicate child, see comment above.  In this
                             * case, we must let the duplicate be inserted at
                             * this level in the tree, so we keep iterating,
                             * looking for an empty slot in which to insert.
                             */
                            JS_ASSERT(sprop != child);
                            JS_RUNTIME_METER(rt, duplicatePropTreeNodes);
                        }
                    }
                    chunkp = &chunk->next;
                } while ((chunk = *chunkp) != NULL);

                chunk = NewPropTreeKidsChunk(rt);
                if (!chunk)
                    return JS_FALSE;
                *chunkp = chunk;
                childp = &chunk->kids[0];
            } else {
                sprop = kids;
                JS_ASSERT(sprop != child);
                if (SPROP_MATCH(sprop, child)) {
                    /*
                     * Duplicate child, see comment above.  Once again, we
                     * must let duplicates created by deletion pile up in a
                     * kids-chunk-list, in order to find them when sweeping
                     * and thereby avoid dangling parent pointers.
                     */
                    JS_RUNTIME_METER(rt, duplicatePropTreeNodes);
                }

                chunk = NewPropTreeKidsChunk(rt);
                if (!chunk)
                    return JS_FALSE;
                parent->kids = CHUNK_TO_KIDS(chunk);
                chunk->kids[0] = sprop;
                childp = &chunk->kids[1];
            }
        }
    insert:
        *childp = child;
    }

    child->parent = parent;
    return JS_TRUE;
}

/* NB: Called with the runtime lock held. */
static void
RemovePropertyTreeChild(JSRuntime *rt, JSScopeProperty *child)
{
    JSPropertyTreeEntry *entry;
    JSScopeProperty *parent, *kids, *kid;
    PropTreeKidsChunk *list, *chunk, **chunkp, *lastChunk;
    uintN i, j;

    parent = child->parent;
    if (!parent) {
        /*
         * Don't remove child if it is not in rt->propertyTreeHash, but only
         * matches a root child in the table that has compatible members. See
         * the "Duplicate child" comments in InsertPropertyTreeChild, above.
         */
        entry = (JSPropertyTreeEntry *)
            JS_DHashTableOperate(&rt->propertyTreeHash, child, JS_DHASH_LOOKUP);

        if (entry->child == child)
            JS_DHashTableRawRemove(&rt->propertyTreeHash, &entry->hdr);
    } else {
        kids = parent->kids;
        if (KIDS_IS_CHUNKY(kids)) {
            list = chunk = KIDS_TO_CHUNK(kids);
            chunkp = &list;

            do {
                for (i = 0; i < MAX_KIDS_PER_CHUNK; i++) {
                    if (chunk->kids[i] == child) {
                        lastChunk = chunk;
                        if (!lastChunk->next) {
                            j = i + 1;
                        } else {
                            j = 0;
                            do {
                                chunkp = &lastChunk->next;
                                lastChunk = *chunkp;
                            } while (lastChunk->next);
                        }
                        for (; j < MAX_KIDS_PER_CHUNK; j++) {
                            if (!lastChunk->kids[j])
                                break;
                        }
                        --j;
                        if (chunk != lastChunk || j > i)
                            chunk->kids[i] = lastChunk->kids[j];
                        lastChunk->kids[j] = NULL;
                        if (j == 0) {
                            *chunkp = NULL;
                            if (!list)
                                parent->kids = NULL;
                            DestroyPropTreeKidsChunk(rt, lastChunk);
                        }
                        return;
                    }
                }

                chunkp = &chunk->next;
            } while ((chunk = *chunkp) != NULL);
        } else {
            kid = kids;
            if (kid == child)
                parent->kids = NULL;
        }
    }
}

/*
 * Called *without* the runtime lock held, this function acquires that lock
 * only when inserting a new child.  Thus there may be races to find or add
 * a node that result in duplicates.  We expect such races to be rare!
 */
static JSScopeProperty *
GetPropertyTreeChild(JSContext *cx, JSScopeProperty *parent,
                     JSScopeProperty *child)
{
    JSRuntime *rt;
    JSPropertyTreeEntry *entry;
    JSScopeProperty *sprop;
    PropTreeKidsChunk *chunk;
    uintN i;

    rt = cx->runtime;
    if (!parent) {
        JS_LOCK_RUNTIME(rt);

        entry = (JSPropertyTreeEntry *)
            JS_DHashTableOperate(&rt->propertyTreeHash, child, JS_DHASH_ADD);
        if (!entry)
            goto out_of_memory;

        sprop = entry->child;
        if (sprop)
            goto out;
    } else {
        /*
         * Because chunks are appended at the end and never deleted except by
         * the GC, we can search without taking the runtime lock.  We may miss
         * a matching sprop added by another thread, and make a duplicate one,
         * but that is an unlikely, therefore small, cost.  The property tree
         * has extremely low fan-out below its root in popular embeddings with
         * real-world workloads.
         *
         * If workload changes so as to increase fan-out significantly below
         * the property tree root, we'll want to add another tag bit stored in
         * parent->kids that indicates a JSDHashTable pointer.
         */
        entry = NULL;
        sprop = parent->kids;
        if (sprop) {
            if (KIDS_IS_CHUNKY(sprop)) {
                chunk = KIDS_TO_CHUNK(sprop);
                do {
                    for (i = 0; i < MAX_KIDS_PER_CHUNK; i++) {
                        sprop = chunk->kids[i];
                        if (!sprop)
                            goto not_found;

                        if (SPROP_MATCH(sprop, child))
                            return sprop;
                    }
                } while ((chunk = chunk->next) != NULL);
            } else {
                if (SPROP_MATCH(sprop, child))
                    return sprop;
            }
        }

    not_found:
        JS_LOCK_RUNTIME(rt);
    }

    sprop = NewScopeProperty(rt);
    if (!sprop)
        goto out_of_memory;

    sprop->id = child->id;
    sprop->getter = child->getter;
    sprop->setter = child->setter;
    sprop->slot = child->slot;
    sprop->attrs = child->attrs;
    sprop->flags = child->flags;
    sprop->shortid = child->shortid;
    sprop->parent = sprop->kids = NULL;
    if (!parent) {
        entry->child = sprop;
    } else {
        if (!InsertPropertyTreeChild(rt, parent, sprop))
            goto out_of_memory;
    }

out:
    JS_UNLOCK_RUNTIME(rt);
    return sprop;

out_of_memory:
    JS_UNLOCK_RUNTIME(rt);
    JS_ReportOutOfMemory(cx);
    return NULL;
}

#ifdef DEBUG_notbrendan
#define CHECK_ANCESTOR_LINE(scope, sparse)                                    \
    JS_BEGIN_MACRO                                                            \
        if ((scope)->table) CheckAncestorLine(scope, sparse);                 \
    JS_END_MACRO

static void
CheckAncestorLine(JSScope *scope, JSBool sparse)
{
    uint32 size;
    JSScopeProperty **spp, **start, **end, *ancestorLine, *sprop, *aprop;
    uint32 entryCount, ancestorCount;

    ancestorLine = SCOPE_LAST_PROP(scope);
    if (ancestorLine)
        JS_ASSERT(SCOPE_HAS_PROPERTY(scope, ancestorLine));

    entryCount = 0;
    size = JS_BIT(scope->sizeLog2);
    start = scope->table;
    for (spp = start, end = start + size; spp < end; spp++) {
        sprop = SPROP_FETCH(spp);
        if (sprop) {
            entryCount++;
            for (aprop = ancestorLine; aprop; aprop = aprop->parent) {
                if (aprop == sprop)
                    break;
            }
            JS_ASSERT(aprop);
        }
    }
    JS_ASSERT(entryCount == scope->entryCount);

    ancestorCount = 0;
    for (sprop = ancestorLine; sprop; sprop = sprop->parent) {
        if (SCOPE_HAD_MIDDLE_DELETE(scope) &&
            !SCOPE_HAS_PROPERTY(scope, sprop)) {
            JS_ASSERT(sparse || (sprop->flags & SPROP_IS_DUPLICATE));
            continue;
        }
        ancestorCount++;
    }
    JS_ASSERT(ancestorCount == scope->entryCount);
}
#else
#define CHECK_ANCESTOR_LINE(scope, sparse) /* nothing */
#endif

JSScopeProperty *
js_AddScopeProperty(JSContext *cx, JSScope *scope, jsid id,
                    JSPropertyOp getter, JSPropertyOp setter, uint32 slot,
                    uintN attrs, uintN flags, intN shortid)
{
    JSScopeProperty **spp, *sprop, *overwriting, **spvec, **spp2, child;
    uint32 size, splen, i;
    int change;

    CHECK_ANCESTOR_LINE(scope, JS_TRUE);

    /*
     * Normalize stub getter and setter values for faster is-stub testing in
     * the SPROP_CALL_[GS]ETTER macros.
     */
    if (getter == JS_PropertyStub)
        getter = NULL;
    if (setter == JS_PropertyStub)
        setter = NULL;

    /*
     * Search for id in order to claim its entry, allocating a property tree
     * node if one doesn't already exist for our parameters.
     */
    spp = js_SearchScope(scope, id, JS_TRUE);
    sprop = overwriting = SPROP_FETCH(spp);
    if (!sprop) {
        /* Check whether we need to grow, if the load factor is >= .75. */
        JS_ASSERT(JS_IS_SCOPE_LOCKED(scope));
        size = JS_BIT(scope->sizeLog2);
        if (scope->entryCount + scope->removedCount >= size - (size >> 2)) {
            if (scope->removedCount >= size >> 2) {
                METER(compresses);
                change = 0;
            } else {
                METER(grows);
                change = 1;
            }
            if (!ChangeScope(cx, scope, change) &&
                scope->entryCount + scope->removedCount == size - 1) {
                METER(addFailures);
                return NULL;
            }
            spp = js_SearchScope(scope, id, JS_TRUE);
            JS_ASSERT(!SPROP_FETCH(spp));
        }
    } else {
        /* Property exists: js_SearchScope must have returned a valid entry. */
        JS_ASSERT(!SPROP_IS_REMOVED(*spp));

        /*
         * If all property members match, this is a redundant add and we can
         * return early.  If the caller wants to allocate a slot, but doesn't
         * care which slot, copy sprop->slot into slot so we can match sprop,
         * if all other members match.
         */
        if (!(attrs & JSPROP_SHARED) &&
            slot == SPROP_INVALID_SLOT &&
            SPROP_HAS_VALID_SLOT(sprop, scope)) {
            slot = sprop->slot;
        }
        if (SPROP_MATCH_PARAMS_AFTER_ID(sprop, getter, setter, slot, attrs,
                                        flags, shortid)) {
            METER(redundantAdds);
            return sprop;
        }

        /*
         * Duplicate formal parameters require us to leave the old property
         * on the ancestor line, so the decompiler can find it, even though
         * its entry in scope->table is overwritten to point at a new property
         * descending from the old one.  The SPROP_IS_DUPLICATE flag helps us
         * cope with the consequent disparity between ancestor line height and
         * scope->entryCount.
         */
        if (flags & SPROP_IS_DUPLICATE) {
            sprop->flags |= SPROP_IS_DUPLICATE;
        } else {
            /*
             * If we are clearing sprop to force an existing property to be
             * overwritten (apart from a duplicate formal parameter), we must
             * unlink it from the ancestor line at scope->lastProp, lazily if
             * sprop is not lastProp.  And we must remove the entry at *spp,
             * precisely so the lazy "middle delete" fixup code further below
             * won't find sprop in scope->table, in spite of sprop being on
             * the ancestor line.
             *
             * When we finally succeed in finding or creating a new sprop
             * and storing its pointer at *spp, we'll use the |overwriting|
             * local saved when we first looked up id to decide whether we're
             * indeed creating a new entry, or merely overwriting an existing
             * property.
             */
            if (sprop == SCOPE_LAST_PROP(scope)) {
                do {
                    SCOPE_REMOVE_LAST_PROP(scope);
                    if (!SCOPE_HAD_MIDDLE_DELETE(scope))
                        break;
                    sprop = SCOPE_LAST_PROP(scope);
                } while (sprop && !SCOPE_HAS_PROPERTY(scope, sprop));
            } else if (!SCOPE_HAD_MIDDLE_DELETE(scope)) {
                /*
                 * If we have no hash table yet, we need one now.  The middle
                 * delete code is simple-minded that way!
                 */
                if (!scope->table) {
                    if (!CreateScopeTable(scope)) {
                        JS_ReportOutOfMemory(cx);
                        return NULL;
                    }
                    spp = js_SearchScope(scope, id, JS_TRUE);
                    sprop = overwriting = SPROP_FETCH(spp);
                }
                SCOPE_SET_MIDDLE_DELETE(scope);
            }
        }

        /*
         * If we fail later on trying to find or create a new sprop, we will
         * goto fail_overwrite and restore *spp from |overwriting|.  Note that
         * we don't bother to keep scope->removedCount in sync, because we'll
         * fix up *spp and scope->entryCount shortly, no matter how control
         * flow returns from this function.
         */
        if (scope->table)
            SPROP_STORE_PRESERVING_COLLISION(spp, NULL);
        scope->entryCount--;
        CHECK_ANCESTOR_LINE(scope, JS_TRUE);
        sprop = NULL;
    }

    if (!sprop) {
        /*
         * If properties were deleted from the middle of the list starting at
         * scope->lastProp, we may need to fork the property tree and squeeze
         * all deleted properties out of scope's ancestor line.  Otherwise we
         * risk adding a node with the same id as a "middle" node, violating
         * the rule that properties along an ancestor line have distinct ids
         * (unless flagged SPROP_IS_DUPLICATE).
         */
        if (SCOPE_HAD_MIDDLE_DELETE(scope)) {
            JS_ASSERT(scope->table);
            CHECK_ANCESTOR_LINE(scope, JS_TRUE);

            splen = scope->entryCount;
            if (splen == 0) {
                scope->lastProp = NULL;
            } else {
                /*
                 * Enumerate live entries in scope->table using a temporary
                 * vector, by walking the (possibly sparse, due to deletions)
                 * ancestor line from scope->lastProp.
                 */
                spvec = (JSScopeProperty **)
                    JS_malloc(cx, splen * sizeof(JSScopeProperty *));
                if (!spvec)
                    goto fail_overwrite;
                i = splen;
                sprop = SCOPE_LAST_PROP(scope);
                JS_ASSERT(sprop);
                do {
                    /*
                     * NB: test SCOPE_GET_PROPERTY, not SCOPE_HAS_PROPERTY --
                     * the latter insists that sprop->id maps to sprop, while
                     * the former simply tests whether sprop->id is bound in
                     * scope.  We must allow for duplicate formal parameters
                     * along the ancestor line, and fork them as needed.
                     */
                    if (!SCOPE_GET_PROPERTY(scope, sprop->id))
                        continue;

                    JS_ASSERT(sprop != overwriting);
                    if (i == 0) {
                        /*
                         * If our original splen estimate, scope->entryCount,
                         * is less than the ancestor line height, there must
                         * be duplicate formal parameters in this (function
                         * object) scope.  Count remaining ancestors in order
                         * to realloc spvec.
                         */
                        do {
                            i++;
                        } while ((sprop = sprop->parent) != NULL);
                        spp2 = (JSScopeProperty **)
                            JS_realloc(cx, spvec,
                                       (splen + i) * sizeof(JSScopeProperty *));
                        if (!spp2) {
                            JS_free(cx, spvec);
                            goto fail_overwrite;
                        }

                        spvec = spp2;
                        memmove(spvec + i, spvec,
                                splen * sizeof(JSScopeProperty *));
                        splen += i;
                        sprop = spvec[i];
                    }

                    spvec[--i] = sprop;
                } while ((sprop = sprop->parent) != NULL);
                JS_ASSERT(i == 0);

                /*
                 * Now loop forward through spvec, forking the property tree
                 * whenever we see a "parent gap" due to deletions from scope.
                 * NB: sprop is null on first entry to the loop body.
                 */
                do {
                    if (spvec[i]->parent == sprop) {
                        sprop = spvec[i];
                    } else {
                        sprop = GetPropertyTreeChild(cx, sprop, spvec[i]);
                        if (!sprop) {
                            JS_free(cx, spvec);
                            goto fail_overwrite;
                        }

                        spp2 = js_SearchScope(scope, sprop->id, JS_FALSE);
                        JS_ASSERT(SPROP_FETCH(spp2) == spvec[i]);
                        SPROP_STORE_PRESERVING_COLLISION(spp2, sprop);
                    }
                } while (++i < splen);
                JS_free(cx, spvec);

                /*
                 * Now sprop points to the last property in scope, where the
                 * ancestor line from sprop to the root is dense w.r.t. scope:
                 * it contains no nodes not mapped by scope->table, apart from
                 * any stinking ECMA-mandated duplicate formal parameters.
                 */
                scope->lastProp = sprop;
                CHECK_ANCESTOR_LINE(scope, JS_FALSE);
                JS_RUNTIME_METER(cx->runtime, middleDeleteFixups);
            }
        }

        /*
         * Aliases share another property's slot, passed in the |slot| param.
         * Shared properties have no slot; unshared properties that do not
         * alias another property's slot get one here, but may lose it due to
         * a JS_ClearScope call.
         */
        if (!(flags & SPROP_IS_ALIAS)) {
            if (attrs & JSPROP_SHARED) {
                slot = SPROP_INVALID_SLOT;
            } else {
                /*
                 * We may have set slot from a nearly-matching sprop, above.
                 * If so, we're overwriting that nearly-matching sprop, so we
                 * can reuse its slot -- we don't need to allocate a new one.
                 * Callers should therefore pass SPROP_INVALID_SLOT for all
                 * non-alias, unshared property adds.
                 */
                if (slot != SPROP_INVALID_SLOT)
                    JS_ASSERT(overwriting);
                else if (!js_AllocSlot(cx, scope->object, &slot))
                    goto fail_overwrite;
            }
        }

        /*
         * Check for a watchpoint on a deleted property; if one exists, change
         * setter to js_watch_set.
         * XXXbe this could get expensive with lots of watchpoints...
         */
        if (!JS_CLIST_IS_EMPTY(&cx->runtime->watchPointList) &&
            js_FindWatchPoint(cx->runtime, scope, id)) {
            setter = js_WrapWatchedSetter(cx, id, attrs, setter);
            if (!setter)
                goto fail_overwrite;
        }

        /* Find or create a property tree node labeled by our arguments. */
        child.id = id;
        child.getter = getter;
        child.setter = setter;
        child.slot = slot;
        child.attrs = attrs;
        child.flags = flags;
        child.shortid = shortid;
        sprop = GetPropertyTreeChild(cx, scope->lastProp, &child);
        if (!sprop)
            goto fail_overwrite;

        /* Store the tree node pointer in the table entry for id. */
        if (scope->table)
            SPROP_STORE_PRESERVING_COLLISION(spp, sprop);
        scope->entryCount++;
        scope->lastProp = sprop;
        CHECK_ANCESTOR_LINE(scope, JS_FALSE);
        if (!overwriting) {
            JS_RUNTIME_METER(cx->runtime, liveScopeProps);
            JS_RUNTIME_METER(cx->runtime, totalScopeProps);
        }

        /*
         * If we reach the hashing threshold, try to allocate scope->table.
         * If we can't (a rare event, preceded by swapping to death on most
         * modern OSes), stick with linear search rather than whining about
         * this little set-back.  Therefore we must test !scope->table and
         * scope->entryCount >= SCOPE_HASH_THRESHOLD, not merely whether the
         * entry count just reached the threshold.
         */
        if (!scope->table && scope->entryCount >= SCOPE_HASH_THRESHOLD)
            (void) CreateScopeTable(scope);
    }

    METER(adds);
    return sprop;

fail_overwrite:
    if (overwriting) {
        /*
         * We may or may not have forked overwriting out of scope's ancestor
         * line, so we must check (the alternative is to set a flag above, but
         * that hurts the common, non-error case).  If we did fork overwriting
         * out, we'll add it back at scope->lastProp.  This means enumeration
         * order can change due to a failure to overwrite an id.
         * XXXbe very minor incompatibility
         */
        for (sprop = SCOPE_LAST_PROP(scope); ; sprop = sprop->parent) {
            if (!sprop) {
                sprop = SCOPE_LAST_PROP(scope);
                if (overwriting->parent == sprop) {
                    SCOPE_SET_LAST_PROP(scope, overwriting);
                } else {
                    sprop = GetPropertyTreeChild(cx, sprop, overwriting);
                    if (sprop) {
                        JS_ASSERT(sprop != overwriting);
                        SCOPE_SET_LAST_PROP(scope, sprop);
                    }
                    overwriting = sprop;
                }
                break;
            }
            if (sprop == overwriting)
                break;
        }
        if (overwriting) {
            if (scope->table)
                SPROP_STORE_PRESERVING_COLLISION(spp, overwriting);
            scope->entryCount++;
        }
        CHECK_ANCESTOR_LINE(scope, JS_TRUE);
    }
    METER(addFailures);
    return NULL;
}

JSScopeProperty *
js_ChangeScopePropertyAttrs(JSContext *cx, JSScope *scope,
                            JSScopeProperty *sprop, uintN attrs, uintN mask,
                            JSPropertyOp getter, JSPropertyOp setter)
{
    JSScopeProperty child, *newsprop, **spp;

    CHECK_ANCESTOR_LINE(scope, JS_TRUE);

    attrs |= sprop->attrs & mask;
    if (getter == JS_PropertyStub)
        getter = NULL;
    if (setter == JS_PropertyStub)
        setter = NULL;
    if (sprop->attrs == attrs &&
        sprop->getter == getter &&
        sprop->setter == setter) {
        return sprop;
    }

    child.id = sprop->id;
    child.getter = getter;
    child.setter = setter;
    child.slot = sprop->slot;
    child.attrs = attrs;
    child.flags = sprop->flags;
    child.shortid = sprop->shortid;

    if (SCOPE_LAST_PROP(scope) == sprop) {
        newsprop = GetPropertyTreeChild(cx, sprop->parent, &child);
        if (newsprop) {
            spp = js_SearchScope(scope, sprop->id, JS_FALSE);
            JS_ASSERT(SPROP_FETCH(spp) == sprop);

            if (scope->table)
                SPROP_STORE_PRESERVING_COLLISION(spp, newsprop);
            SCOPE_SET_LAST_PROP(scope, newsprop);
            CHECK_ANCESTOR_LINE(scope, JS_TRUE);
        }
    } else {
        /*
         * Let js_AddScopeProperty handle this |overwriting| case, including
         * the conservation of sprop->slot (if it's valid).  We must not call
         * js_RemoveScopeProperty here, it will free a valid sprop->slot and
         * js_AddScopeProperty won't re-allocate it.
         */
        newsprop = js_AddScopeProperty(cx, scope, child.id,
                                       child.getter, child.setter, child.slot,
                                       child.attrs, child.flags, child.shortid);
    }

#ifdef DEBUG_brendan
    if (!newsprop)
        METER(changeFailures);
#endif
    return newsprop;
}

JSBool
js_RemoveScopeProperty(JSContext *cx, JSScope *scope, jsid id)
{
    JSScopeProperty **spp, *stored, *sprop;
    uint32 size;

    JS_ASSERT(JS_IS_SCOPE_LOCKED(scope));
    CHECK_ANCESTOR_LINE(scope, JS_TRUE);
    METER(removes);

    spp = js_SearchScope(scope, id, JS_FALSE);
    stored = *spp;
    sprop = SPROP_CLEAR_COLLISION(stored);
    if (!sprop) {
        METER(uselessRemoves);
        return JS_TRUE;
    }

    /* Convert from a list to a hash so we can handle "middle deletes". */
    if (!scope->table && sprop != scope->lastProp) {
        if (!CreateScopeTable(scope)) {
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }
        spp = js_SearchScope(scope, id, JS_FALSE);
        stored = *spp;
        sprop = SPROP_CLEAR_COLLISION(stored);
    }

    /* First, if sprop is unshared and not cleared, free its slot number. */
    if (SPROP_HAS_VALID_SLOT(sprop, scope))
        js_FreeSlot(cx, scope->object, sprop->slot);

    /* Next, remove id by setting its entry to a removed or free sentinel. */
    if (SPROP_HAD_COLLISION(stored)) {
        JS_ASSERT(scope->table);
        *spp = SPROP_REMOVED;
        scope->removedCount++;
    } else {
        METER(removeFrees);
        if (scope->table)
            *spp = NULL;
    }
    scope->entryCount--;
    JS_RUNTIME_UNMETER(cx->runtime, liveScopeProps);

    /* Update scope->lastProp directly, or set its deferred update tag. */
    if (sprop == SCOPE_LAST_PROP(scope)) {
        do {
            SCOPE_REMOVE_LAST_PROP(scope);
            if (!SCOPE_HAD_MIDDLE_DELETE(scope))
                break;
            sprop = SCOPE_LAST_PROP(scope);
        } while (sprop && !SCOPE_HAS_PROPERTY(scope, sprop));
    } else if (!SCOPE_HAD_MIDDLE_DELETE(scope)) {
        SCOPE_SET_MIDDLE_DELETE(scope);
    }
    CHECK_ANCESTOR_LINE(scope, JS_TRUE);

    /* Last, consider shrinking scope's table if its load factor is <= .25. */
    size = JS_BIT(scope->sizeLog2);
    if (size > MIN_SCOPE_SIZE && scope->entryCount <= size >> 2) {
        METER(shrinks);
        (void) ChangeScope(cx, scope, -1);
    }

    return JS_TRUE;
}

void
js_ClearScope(JSContext *cx, JSScope *scope)
{
    CHECK_ANCESTOR_LINE(scope, JS_TRUE);
#ifdef DEBUG
    JS_LOCK_RUNTIME_VOID(cx->runtime,
                         cx->runtime->liveScopeProps -= scope->entryCount);
#endif

    if (scope->table)
        free(scope->table);
    InitMinimalScope(scope);
}

#ifdef DEBUG_brendan

#include <stdio.h>
#include <math.h>

uint32 js_nkids_max;
uint32 js_nkids_sum;
double js_nkids_sqsum;
uint32 js_nkids_hist[11];

static void
MeterKidCount(uintN nkids)
{
    if (nkids) {
        js_nkids_sum += nkids;
        js_nkids_sqsum += (double)nkids * nkids;
        if (nkids > js_nkids_max)
            js_nkids_max = nkids;
    }
    js_nkids_hist[JS_MIN(nkids, 10)]++;
}

static void
MeterPropertyTree(JSScopeProperty *node)
{
    uintN i, nkids;
    JSScopeProperty *kids, *kid;
    PropTreeKidsChunk *chunk;

    nkids = 0;
    kids = node->kids;
    if (kids) {
        if (KIDS_IS_CHUNKY(kids)) {
            for (chunk = KIDS_TO_CHUNK(kids); chunk; chunk = chunk->next) {
                for (i = 0; i < MAX_KIDS_PER_CHUNK; i++) {
                    kid = chunk->kids[i];
                    if (!kid)
                        break;
                    MeterPropertyTree(kid);
                    nkids++;
                }
            }
        } else {
            MeterPropertyTree(kids);
            nkids = 1;
        }
    }

    MeterKidCount(nkids);
}

JS_STATIC_DLL_CALLBACK(JSDHashOperator)
js_MeterPropertyTree(JSDHashTable *table, JSDHashEntryHdr *hdr, uint32 number,
                     void *arg)
{
    JSPropertyTreeEntry *entry = (JSPropertyTreeEntry *)hdr;

    MeterPropertyTree(entry->child);
    return JS_DHASH_NEXT;
}

#endif /* DEBUG_brendan */

void
js_SweepScopeProperties(JSRuntime *rt)
{
    JSArena **ap, *a;
    JSScopeProperty *limit, *sprop, *parent, *kids, *kid;
    uintN liveCount;
    PropTreeKidsChunk *chunk, *nextChunk;
    uintN i;

#ifdef DEBUG_brendan
    uint32 livePropCapacity, totalLiveCount = 0;
    static FILE *logfp;
    if (!logfp)
        logfp = fopen("/tmp/proptree.stats", "a");

    MeterKidCount(rt->propertyTreeHash.entryCount);
    JS_DHashTableEnumerate(&rt->propertyTreeHash, js_MeterPropertyTree, NULL);

    {
        double mean = 0., var = 0., sigma = 0.;
        double nodesum = rt->livePropTreeNodes;
        double kidsum = js_nkids_sum;
        if (nodesum > 0 && kidsum >= 0) {
            mean = kidsum / nodesum;
            var = nodesum * js_nkids_sqsum - kidsum * kidsum;
            if (var < 0.0 || nodesum <= 1)
                var = 0.0;
            else
                var /= nodesum * (nodesum - 1);

            /* Windows says sqrt(0.0) is "-1.#J" (?!) so we must test. */
            sigma = (var != 0.) ? sqrt(var) : 0.;
        }

        fprintf(logfp,
                "props %u nodes %g beta %g meankids %g sigma %g max %u",
                rt->liveScopeProps, nodesum, nodesum / rt->liveScopeProps,
                mean, sigma, js_nkids_max);
    }

    fprintf(logfp, " histogram %u %u %u %u %u %u %u %u %u %u %u",
            js_nkids_hist[0], js_nkids_hist[1],
            js_nkids_hist[2], js_nkids_hist[3],
            js_nkids_hist[4], js_nkids_hist[5],
            js_nkids_hist[6], js_nkids_hist[7],
            js_nkids_hist[8], js_nkids_hist[9],
            js_nkids_hist[10]);
    js_nkids_sum = js_nkids_max = 0;
    js_nkids_sqsum = 0;
    memset(js_nkids_hist, 0, sizeof js_nkids_hist);
#endif

    /* Mark watched scope properties hidden in the runtime before we sweep. */
    js_MarkWatchPoints(rt);

    ap = &rt->propertyArenaPool.first.next;
    while ((a = *ap) != NULL) {
        limit = (JSScopeProperty *) a->avail;
        liveCount = 0;
        for (sprop = (JSScopeProperty *) a->base; sprop < limit; sprop++) {
            /* If the id is null, sprop is already on the freelist. */
            if (sprop->id == JSVAL_NULL)
                continue;

            /* If the mark bit is set, sprop is alive, so we skip it. */
            if (sprop->flags & SPROP_MARK) {
                sprop->flags &= ~SPROP_MARK;
                liveCount++;
                continue;
            }

            /* Ok, sprop is garbage to collect: unlink it from its parent. */
            RemovePropertyTreeChild(rt, sprop);

            /* Take care to reparent all sprop's kids to their grandparent. */
            kids = sprop->kids;
            if (kids) {
                sprop->kids = NULL;
                parent = sprop->parent;
                if (KIDS_IS_CHUNKY(kids)) {
                    chunk = KIDS_TO_CHUNK(kids);
                    do {
                        for (i = 0; i < MAX_KIDS_PER_CHUNK; i++) {
                            kid = chunk->kids[i];
                            if (!kid)
                                break;
                            JS_ASSERT(kid->parent == sprop);
                            InsertPropertyTreeChild(rt, parent, kid);
                        }
                        nextChunk = chunk->next;
                        DestroyPropTreeKidsChunk(rt, chunk);
                    } while ((chunk = nextChunk) != NULL);
                } else {
                    kid = kids;
                    InsertPropertyTreeChild(rt, parent, kid);
                }
            }

            /* Clear id so we know (above) that sprop is on the freelist. */
            sprop->id = JSVAL_NULL;
            FREENODE_INSERT(rt->propertyFreeList, sprop);
            JS_RUNTIME_UNMETER(rt, livePropTreeNodes);
        }

        /* If a contains no live properties, return it to the malloc heap. */
        if (liveCount == 0) {
            for (sprop = (JSScopeProperty *) a->base; sprop < limit; sprop++)
                FREENODE_REMOVE(sprop);
            JS_ARENA_DESTROY(&rt->propertyArenaPool, a, ap);
        } else {
#ifdef DEBUG_brendan
            livePropCapacity += limit - (JSScopeProperty *) a->base;
            totalLiveCount += liveCount;
#endif
            ap = &a->next;
        }
    }

#ifdef DEBUG_brendan
    fprintf(logfp, " arenautil %g%%\n",
            (totalLiveCount * 100.) / livePropCapacity);
    fflush(logfp);
#endif
}

JSBool
js_InitPropertyTree(JSRuntime *rt)
{
    if (!JS_DHashTableInit(&rt->propertyTreeHash, &PropertyTreeHashOps, NULL,
                           sizeof(JSPropertyTreeEntry), JS_DHASH_MIN_SIZE)) {
        rt->propertyTreeHash.ops = NULL;
        return JS_FALSE;
    }
    JS_InitArenaPool(&rt->propertyArenaPool, "properties",
                     256 * sizeof(JSScopeProperty), sizeof(void *));
    return JS_TRUE;
}

void
js_FinishPropertyTree(JSRuntime *rt)
{
    if (rt->propertyTreeHash.ops) {
        JS_DHashTableFinish(&rt->propertyTreeHash);
        rt->propertyTreeHash.ops = NULL;
    }
    JS_FinishArenaPool(&rt->propertyArenaPool);
}
