# Visualizzazione del Memory Leak in expandTree

Ti mostro passo per passo cosa succede alla memoria durante l'espansione dell'albero.

## Situazione Iniziale

Supponiamo di avere un albero con `ub = 15` e vogliamo inserire `key = 50`.

**Albero originale (qdig->root):**
```
       [0,15]
       /    \
    [0,7]  [8,15]
     / \      / \
   ...  ... ...  ...
```

## Passo 1: Creazione del Nuovo Albero Temporaneo

```c
QDigest tmp = qdigestCreate(qdig->k, 63); // Nuovo albero [0,63]
```

**Albero tmp (vuoto):**
```
[0,63]
```

## Passo 2: Chiamata a _insert per Creare il Percorso

```c
_insert(*tmp, 15, 1, false, S);  // Inserisce chiave 15
```

Questo crea un percorso completo dalla radice fino alla foglia `[15,15]`:

**Albero tmp dopo _insert:**
```
                [0,63]
                /    \
            [0,31]  [32,63]
            /    \
        [0,15]  [16,31]
        /    \
    [0,7]  [8,15]
           /    \
       [8,11] [12,15]
              /    \
          [12,13] [14,15]
                  /    \
              [14,14] [15,15] ← count=1
```

**Nodi creati in memoria:** ~10+ nodi allocati dinamicamente

## Passo 3: Navigazione al Punto di Innesto

```c
struct QDigestNode *n = tmp->root;
while (n->ub != qdig->root->ub) {
  n = n->left;
}
```

**Troviamo n:**
```
                [0,63]
                /    
            [0,31]  
            /    
    n --> [0,15]  ← n->ub == 15
        /    \
    [0,7]  [8,15]
           /    \
       [8,11] [12,15]
              /    \
          [12,13] [14,15]
                  /    \
              [14,14] [15,15]
```

## Passo 4: Salviamo il Parent e Contiamo i Nodi da Rimuovere

```c
struct QDigestNode *par = n->parent;
int to_remove = 0;
while (n) {
  n = n->right;
  ++to_remove;
}
```

**Visualizzazione:**
```
            par --> [0,31]
                    /    \
           n --> [0,15]  [16,31]
                 /    \
             [0,7]  [8,15]      ← n = n->right
                    /    \
                [8,11] [12,15]  ← n = n->right
                       /    \
                   [12,13] [14,15]  ← n = n->right
                           /    \
                       [14,14] [15,15]  ← n = n->right
                                         ↓
                                      n = NULL (fine loop)

to_remove = 4 (contati: [8,15], [12,15], [14,15], [15,15])
```

**⚠️ Ma attenzione!** `n` originale (**Commento di Ettore**-> con n originale il bro Claude secondo me intende *par, che era stato dichiaro come n->parent prima del while) punta ancora a `[0,15]` che ha anche un sottoalbero sinistro `[0,7]`!

## Passo 5: IL PROBLEMA - Sovrascriviamo il Puntatore

```c
par->left = qdig->root;  // ← SOVRASCRIVIAMO!
```

**Prima dell'assegnazione (in memoria):**
```
HEAP MEMORY:

par -> [0,31] 
        |
        | par->left punta a:
        ↓
       [0,15] ← QUESTO NODO E TUTTO IL SUO SOTTOALBERO
       /    \     SARANNO PERSI!
   [0,7]  [8,15]
          /    \
      [8,11] [12,15]
             /    \
         [12,13] [14,15]
                 /    \
             [14,14] [15,15]
```

**Dopo l'assegnazione:**
```
HEAP MEMORY:

par -> [0,31]
        |
        | par->left ora punta a:
        ↓
  [0,15] (ALBERO ORIGINALE DI qdig)
  /    \
[vecchi nodi dell'albero qdig...]


MEMORIA ORFANA (LEAK!):
💀 [0,15] (vecchio tmp)
   /    \
  [0,7]  [8,15]
         /    \
     [8,11] [12,15]
            /    \
        [12,13] [14,15]
                /    \
            [14,14] [15,15]

❌ Nessun puntatore può più raggiungere questi nodi!
❌ Impossibile liberare questa memoria!
❌ Rimangono allocati fino alla terminazione del programma!
```

## Conteggio del Leak

**Nodi persi in memoria:**

1. `[0,15]` (vecchio tmp) + sottoalbero sinistro
2. `[0,7]` + suoi figli
3. Percorso destro: `[8,15]` → `[12,15]` → `[14,15]` → `[15,15]`

**Totale:** Circa 10+ nodi persi ad ogni espansione!

## La Soluzione Corretta

```c
struct QDigestNode *par = n->parent;

// PRIMA: Salva il riferimento al nodo che sarà sostituito
struct QDigestNode *old_subtree = par->left;

// Innesta l'albero originale
par->left = qdig->root;
par->left->parent = par;
qdig->root = NULL;

// DOPO: Libera ricorsivamente il vecchio sottoalbero temporaneo
qdigestnodeRelease(old_subtree);

// Aggiorna i conteggi
tmp->num_nodes -= to_remove;
tmp->num_nodes += qdig->num_nodes;
tmp->N = qdig->N;

qdig->swap(tmp);
```
