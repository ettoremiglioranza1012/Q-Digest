# Visualizzazione Completa di expandTree con Fix del Memory Leak

## Scenario Completo

Albero originale con `ub = 15`, inseriamo `key = 50` → serve espansione a `ub = 63`

---

## 📍 STEP 0: Situazione Iniziale

**Albero Originale (qdig->root):**
```
         [0,15] count=0
         /    \
     [0,7]    [8,15]
     /  \      /   \
  [0,3][4,7][8,11][12,15]
   ...  ...   ...   ...
   
Total nodes: ~20 nodi
qdig->N = 1000 (esempio)
```

---

## 📍 STEP 1: Chiamata a expandTree

```c
expandTree(qdig, 64, S);  // 64 è exclusive, diventa 63 inclusive
```

**Creazione nuovo albero temporaneo:**
```c
--ub;  // 64 → 63
QDigest tmp = qdigestCreate(qdig->k, 63);
```

**tmp->root (VUOTO):**
```
[0,63] count=0

tmp->num_nodes = 1
tmp->N = 0
```

---

## 📍 STEP 2: _insert per Creare il Path Placeholder

```c
_insert(*tmp, *qdig->root->ub, 1, false, S);
//            ^^^^^^^^^^^^^^^^  ^
//            15               count=1
```

**Percorso creato da _insert(tmp, 15, 1):**
```
                    [0,63]
                    /    \
                [0,31]  [32,63]
                /    \
            [0,15]  [16,31]  ← n->ub == 15 (PUNTO DI INNESTO)
            /    \
        [0,7]  [8,15]
               /    \
           [8,11]  [12,15]
                   /    \
               [12,13]  [14,15]
                        /    \
                    [14,14]  [15,15] count=1 ← foglia con count
```

**Stato tmp dopo _insert:**
```
tmp->num_nodes = ~11 nodi
tmp->N = 1 (aggiunto da _insert!)
⚠️ PROBLEMA DATI: tmp->N = 1 ma dovrebbe essere 0 (sarà sovrascritto dopo)
```

---

## 📍 STEP 3: Navigazione al Punto di Innesto

```c
QDigestNode *n = tmp->root;
while (n->ub != *qdig->root->ub) {
  n = n->left;
}
```

**Percorso di navigazione:**
```
tmp->root
    ↓
[0,63]
    ↓ left
[0,31]
    ↓ left
[0,15] ← STOP! n->ub == qdig->root->ub (15)
```

**Risultato:**
```
n → [0,15]
```

---

## 📍 STEP 4: Salvataggio Parent e Conteggio Nodi da Rimuovere

```c
QDigestNode *par = n->parent;
int to_remove = 0;
while (n) {
  n = n->right; 
  ++to_remove;
}
```

**Visualizzazione del conteggio:**
```
            par → [0,31]
                  /    \
         n → [0,15]  [16,31]
             /    \
         [0,7]  [8,15] ← n = n->right (to_remove = 1)
                /    \
            [8,11]  [12,15] ← n = n->right (to_remove = 2)
                    /    \
                [12,13]  [14,15] ← n = n->right (to_remove = 3)
                         /    \
                     [14,14]  [15,15] ← n = n->right (to_remove = 4)
                                       ↓
                                   n = NULL (fine)
```

**Risultato:**
```
par → [0,31]
to_remove = 4
n = NULL (dopo il loop)
```

---

## 📍 STEP 5: Salvataggio Riferimento al Sottoalbero da Liberare

```c
struct QDigestNode *old_subtree = par->left;
```

**Memoria a questo punto:**
```
HEAP:

par → [0,31]
       |
       | par->left
       ↓
old_subtree → [0,15] ← SALVIAMO QUESTO RIFERIMENTO!
              /    \
          [0,7]  [8,15]
                 /    \
             [8,11]  [12,15]
                     /    \
                 [12,13]  [14,15]
                          /    \
                      [14,14]  [15,15] count=1
```

---

## 📍 STEP 6: Innesto dell'Albero Originale

```c
par->left = qdig->root;
par->left->parent = par;
qdig->root = NULL;
```

**Prima dell'innesto - Due alberi separati:**
```
TMP TREE:                          ORIGINAL TREE (qdig):
    [0,63]                              [0,15] ← qdig->root
    /    \                              /    \
[0,31]  [32,63]                     [0,7]  [8,15]
/    \                               /  \    /   \
[0,15] [16,31]                   [0,3][4,7][8,11][12,15]
/    \                            con dati reali, count vari
[0,7] [8,15]                      qdig->num_nodes = ~20
      /    \                      qdig->N = 1000
  [8,11] [12,15]
         /    \
     [12,13] [14,15]
              /    \
          [14,14] [15,15] count=1
```

**Dopo l'innesto:**
```
TMP TREE (con albero originale innestato):

                    [0,63]
                    /    \
                [0,31]  [32,63]
                /    \
    par->left  /      \
        ↓     /        \
    [0,15]  [16,31] ← ALBERO ORIGINALE INNESTATO QUI!
    /    \
[0,7]  [8,15] ← QUESTI SONO I NODI ORIGINALI con dati reali
/  \    /   \
[0,3][4,7][8,11][12,15]
...con count reali...

qdig->root = NULL ← pointer azzerato
```

**Memoria orfana (PRIMA del fix):**
```
❌ SENZA FIX - MEMORIA PERSA:

old_subtree → [0,15] (vecchio placeholder)
              /    \
          [0,7]  [8,15]
                 /    \
             [8,11]  [12,15]
                     /    \
                 [12,13]  [14,15]
                          /    \
                      [14,14]  [15,15] count=1

Nessun puntatore raggiunge questi nodi!
```

---

## 📍 STEP 7: 🔧 FIX - Liberazione Memoria Orfana

```c
qdigestnodeRelease(old_subtree);
```

**Funzione ricorsiva libera tutto il sottoalbero:**
```
qdigestnodeRelease([0,15]):
  ├─ qdigestnodeRelease([0,7])
  │   ├─ free([0,7]) ✅
  │   └─ return
  ├─ qdigestnodeRelease([8,15])
  │   ├─ qdigestnodeRelease([8,11])
  │   │   └─ free([8,11]) ✅
  │   ├─ qdigestnodeRelease([12,15])
  │   │   ├─ qdigestnodeRelease([12,13])
  │   │   │   └─ free([12,13]) ✅
  │   │   ├─ qdigestnodeRelease([14,15])
  │   │   │   ├─ free([14,14]) ✅
  │   │   │   ├─ free([15,15]) ✅
  │   │   │   └─ free([14,15]) ✅
  │   │   └─ free([12,15]) ✅
  │   └─ free([8,15]) ✅
  └─ free([0,15]) ✅

✅ MEMORIA LIBERATA CORRETTAMENTE!
```

---

## 📍 STEP 8: Aggiornamento Metadati

```c
tmp->num_nodes -= to_remove;
tmp->num_nodes += qdig->num_nodes;
tmp->N = qdig->N;
```

**Calcolo:**
```
tmp->num_nodes prima: ~11
tmp->num_nodes -= 4 (to_remove)
tmp->num_nodes += 20 (qdig->num_nodes)
tmp->num_nodes finale: ~27

tmp->N = 1000 (da qdig->N, sovrascrive l'1 errato)
```

**Albero tmp finale:**
```
                    [0,63]
                    /    \
                [0,31]  [32,63]
                /    \
            [0,15]  [16,31]
            /    \
        [0,7]  [8,15]  ← Albero originale con dati reali
        /  \    /   \
     [0,3][4,7][8,11][12,15]
     
tmp->num_nodes = 27
tmp->N = 1000
```

---

## 📍 STEP 9: Swap Finale

```c
qdig->swap(tmp);
```

**Cosa succede:**
```
PRIMA:
qdig → [0,15] (vecchio, ora NULL)
tmp  → [0,63] (nuovo espanso)

DOPO swap:
qdig → [0,63] (nuovo espanso) ✅
tmp  → NULL (verrà distrutto)
```

**Risultato finale in qdig:**
```
                    [0,63]
                    /    \
                [0,31]  [32,63]
                /    \
            [0,15]  [16,31]
            /    \
        [0,7]  [8,15]
        /  \    /   \
     [0,3][4,7][8,11][12,15]
     
qdig->root->ub = 63 ✅
qdig->num_nodes = 27 ✅
qdig->N = 1000 ✅
```

---

## 📊 Confronto: CON vs SENZA Fix

### ❌ SENZA Fix (Old Code):
```
Memoria dopo ogni espansione:
├─ Albero funzionante: ✅
├─ Nodi placeholder liberati: ❌ (LEAK!)
├─ Memoria persa: ~4-10 nodi per espansione
└─ Dopo 10 espansioni: ~50-100 nodi leaked
```

### ✅ CON Fix (New Code):
```
Memoria dopo ogni espansione:
├─ Albero funzionante: ✅
├─ Nodi placeholder liberati: ✅
├─ Memoria persa: 0 nodi
└─ Dopo 10 espansioni: 0 nodi leaked
```

---

## 🎯 Riepilogo Finale

**Il fix risolve:**

- ✅ Memory leak dei nodi placeholder
- ✅ Liberazione ricorsiva di tutto il sottoalbero
- ✅ Gestione corretta dei metadati

**L'ordine è critico:**
```c
// 1. Salva riferimento PRIMA di sovrascrivere
old_subtree = par->left;

// 2. Innesta nuovo albero
par->left = qdig->root;

// 3. Libera vecchio albero DOPO l'innesto
qdigestnodeRelease(old_subtree);

// 4. Aggiorna metadati
tmp->num_nodes -= to_remove;
```

⚠️ **Se liberi PRIMA di innestare, perdi l'albero originale!** 💀
