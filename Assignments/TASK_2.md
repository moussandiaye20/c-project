# Algorithmes

## Objectif 1 - Refactorisation de l'existant

### A - Structured Bindings

`TowerSimulation::display_help()` est chargé de l'affichage des touches disponibles.
Dans sa boucle, remplacez `const auto& ks_pair` par un structured binding adapté.
> On peut utiliser `const auto& [key, action]` 

### B - Algorithmes divers

1. `AircraftManager::move()` (ou bien `update()`) supprime les avions de la `move_queue` dès qu'ils sont "hors jeux".
En pratique, il y a des opportunités pour des pièges ici. Pour les éviter, `<algorithm>` met à disposition la fonction `std::remove_if`.
Remplacez votre boucle avec un appel à `std::remove_if`.

**Attention**: pour cela c'est necessaire que `AircraftManager` stocke les avion dans un `std::vector` ou `std::list` (c'est déjà le cas pour la solution filé).

2. Pour des raisons de statistiques, on aimerait bien être capable de compter tous les avions de chaque airline.
A cette fin, rajoutez des callbacks sur les touches `0`..`7` de manière à ce que le nombre d'avions appartenant à `airlines[x]` soit affiché en appuyant sur `x`.
Rendez-vous compte de quelle classe peut acquérir cet information. Utilisez la bonne fonction de `<algorithm>` pour obtenir le résultat.
> La classe pouvant compter les avions appartenant à une ligne est AircraftManager. J'utilise std::count_if pour compter
> les avions dont la ligne commence par le code de l'airline

### C - Relooking de Point3D

La classe `Point3D` présente beaucoup d'opportunités d'appliquer des algorithmes.
Particulairement, des formulations de type `x() = ...; y() = ...; z() = ...;` se remplacent par un seul appel à la bonne fonction de la librairie standard.
Remplacez le tableau `Point3D::values` par un `std::array` et puis,
remplacez le code des fonctions suivantes en utilisant des fonctions de `<algorithm>` / `<numeric>`:

1. `Point3D::operator*=(const float scalar)`
> Utilisation de std::transform(src_start, src_end, dest_start, fnct)
2. `Point3D::operator+=(const Point3D& other)` et `Point3D::operator-=(const Point3D& other)`
> Utilisation de std::transform(src1_start, src1_end, src2_start, dest_start, fnct)
3. `Point3D::length() const`
> Utilisation de std::accumulate(start, end, initial_value, fnct)

---

## Objectif 2 - Rupture de kérosène

Vous allez introduire la gestion de l'essence dans votre simulation.\
Comme le but de ce TP est de vous apprendre à manipuler les algorithmes de la STL, avant d'écrire une boucle, demandez-vous du coup s'il n'existe pas une fonction d'`<algorithm>` ou de `<numeric>` qui permet de faire la même chose.

La notation tiendra compte de votre utilisation judicieuse de la librairie standard. 

### A - Consommation d'essence

Ajoutez un attribut `fuel` à `Aircraft`, et initialisez-le à la création de chaque avion avec une valeur aléatoire comprise entre `150` et `3'000`.\
Décrémentez cette valeur dans `Aircraft::update` si l'avion est en vol.\
Lorsque cette valeur atteint 0, affichez un message dans la console pour indiquer le crash, et faites en sorte que l'avion soit supprimé du manager.
> Suppression -> retourner `true` dans move/update

N'hésitez pas à adapter la borne `150` - `3'000`, de manière à ce que des avions se crashent de temps en temps.

### B - Minimiser les crashs

Afin de minimiser les crashs, on veut changer la strategie avec laquelle le `Tower` distribue des terminals aux avions.
Actuellement, des avions demandent un nouveau chemin du `Tower` dès qu'ils arrivent au dernier `Waypoint`.
Si disponible, le `Tower` leur donne le chemin au prochain `Terminal` libre, si non `Tower` les envoit à un circle autour du `Airport`.
Pour pouvoir prioretiser les avions avec moins d'essence, on veut qu'ils demandent un `Terminal` tout le temps s'ils sont en train de circuler.
A ce fin,
1. introduisez une fonction `bool Aircraft::is_circling() const` qui indique si l'avion est en train de circuler
2. introduisez une fonction `bool Aircraft::has_terminal() const` qui indique si l'avion a été assigné un `Terminal` (`waypoints.back().type` peut aider pour faire cette décision).
3. introduisez une fonction `WaypointQueue Tower::reserve_terminal(Aircraft& aircraft)` qui s'occupe de la reservation d'un `Terminal` et retourne un chemin vers ce `Terminal` si possible, et le chemin vide sinon. Vous pouvez re-utiliser le code de reservation trouvé dans `Tower::get_instructions`.
4. changez la fonction `move()` (ou bien `update()`) de `Aircraft` afin qu'elle appelle `Tower::reserve_terminal` si l'avion est en train de circuler et n'a pas encore été assigné à un `Terminal`.

Avec ce changement, il est désormais possible qu'un `Terminal` disponible est donné à l'avion le plus désesperé (qui a le moins d'essence) entre tout les avions circulant.
Pour ça, il suffit d'appeler leur `move()` (ou `update()`) dans un ordre intelligent:
Tout d'abord, les avions qui ont déjà réservé un terminal (afin qu'ils pourront libérer leurs terminaux avant que vous mettiez à jour les avions qui essayeront de les réserver). La suite sera ordonnée selon le niveau d'essence respectif de chaque avion.

Par exemple :
```b
A - Reserved / Fuel: 100
B - NotReserved / Fuel: 50
C - NotReserved / Fuel: 300
D - NotReserved / Fuel: 150
E - Reserved / Fuel: 2500
```
pourra être réordonné en
```b
A - Reserved / Fuel: 100
E - Reserved / Fuel: 2500
B - NotReserved / Fuel: 50
D - NotReserved / Fuel: 150
C - NotReserved / Fuel: 300
```

Faites en sort que votre `AircraftManager` stocke les avions dans un `vector<unique_ptr<Aircraft>>` ou `list<unique_ptr<Aircraft>>` et triez le au debut du `move()` (ou `update()`) de l'`AircraftManager` (en utilisant la bonne fonction de la STL) pour qu'ils **soient** traités en bon ordre après.
> J'utilise l'opérateur de comparaison (par ce que de toute façon c'est le seul moment ou on souhaite comparer 
> 2 Aircraft mais on pourrait tout à fait déclarer une autre fonction qui fait la même chose).
> Et j'utilise std::sort pour trier les avions.

### C - Réapprovisionnement 

Afin de pouvoir repartir en toute sécurité, les avions avec moins de `200` unités d'essence doivent être réapprovisionnés par l'aéroport pendant qu'ils sont au terminal.

1. Ajoutez une fonction `bool Aircraft::is_low_on_fuel() const`, qui renvoie `true` si l'avion dispose de moins de `200` unités d'essence.\
Modifiez le code de `Terminal` afin que les avions qui n'ont pas suffisamment d'essence restent bloqués.\
Testez votre programme pour vérifier que certains avions attendent bien indéfiniment au terminal.
Si ce n'est pas le cas, essayez de faire varier la constante `200`.
> Ca fonctionne bien.

2. Dans `AircraftManager`, implémentez une fonction `get_required_fuel`, qui renvoie la somme de l'essence manquante (le plein, soit `3'000`, moins la quantité courante d'essence) pour les avions vérifiant les conditions suivantes :\
\- l'avion est bientôt à court d'essence\
\- l'avion n'est pas déjà reparti de l'aéroport.

3. Ajoutez deux attributs `fuel_stock` et `ordered_fuel` dans la classe `Airport`, que vous initialiserez à 0.\
Ajoutez également un attribut `next_refill_time`, aussi initialisé à 0.\
Enfin, faites en sorte que la classe `Airport` ait accès à votre `AircraftManager` de manière à pouvoir l'interroger.
   
4. Ajoutez une fonction `refill` à la classe `Aircraft`, prenant un paramètre `fuel_stock` par référence non-constante.
Cette fonction rempliera le réservoir de l'avion en soustrayant ce dont il a besoin de `fuel_stock`.
Bien entendu, `fuel_stock` ne peut pas devenir négatif.\
Indiquez dans la console quel avion a été réapprovisionné ainsi que la quantité d'essence utilisée.

5. Définissez maintenant une fonction `refill_aircraft_if_needed` dans la classe `Terminal`, prenant un paramètre `fuel_stock` par référence non-constante.
Elle devra appeler la fonction `refill` sur l'avion actuellement au terminal, si celui-ci a vraiment besoin d'essence.  

6. Modifiez la fonction `Aircraft::update`, afin de mettre-en-oeuvre les étapes suivantes.\
\- Si `next_refill_time` vaut 0 :\
    \* `fuel_stock` est incrémenté de la valeur de `ordered_full`.\
    \* `ordered_fuel` est recalculé en utilisant le minimum entre `AircraftManager::get_required_fuel()` et `5'000` (il s'agit du volume du camion citerne qui livre le kérosène).\
    \* `next_refill_time` est réinitialisé à `100`.\
    \* La quantité d'essence reçue, la quantité d'essence en stock et la nouvelle quantité d'essence commandée sont affichées dans la console.\
\- Sinon `next_refill_time` est décrémenté.\
\- Les avions de chacun des terminaux sont réapprovionnés s'ils doivent l'être.
> Le ""soucis"" de çette façon de faire c'est que du coup on commande beaucoup de surplus 
> par rapport à ce qu'on utilise vu que au début beaucoup d'avions ont besoin de fuel mais 
> que le temps qu'ils attérissent, soient traité et reparte on a déjà commandé le prochain
> camion citerne. La façon de corriger ça serait de garder en mémoire ceux ayant déjà indiqué 
> leur besoin en fuel pour ne pas les commander la prochaine fois (ce que je n'ai pas fait)

### D - Paramétrage (optionnel)

Pour le moment, tous les avions ont la même consommation d'essence (1 unité / trame) et la même taille de réservoir (`3'000`).

1. Arrangez-vous pour que ces deux valeurs soient maintenant déterminées par le type de chaque avion (`AircraftType`).
> Fait et donc on ajoute deux nouvelles valeurs dans les fichiers de config des avions: la consommation et la quantité maximum

2. Pondérez la consommation réelle de l'avion par sa vitesse courante.
La consommation définie dans `AircraftType` ne s'appliquera que lorsque l'avion est à sa vitesse maximale.
> Fait on divise la vitesse actuelle par la vitesse maximale puis on multiplie le résultat à la consommation

3. Un avion indique qu'il a besoin d'essence lorsqu'il a moins de `200` unités.
Remplacez cette valeur pour qu'elle corresponde à la quantité consommée en 10s à vitesse maximale.\
Si vous n'avez pas fait la question bonus de TASK_0, notez bien que la fonction `update` de chaque avion devrait être appelée `DEFAULT_TICKS_PER_SEC` fois par seconde. 
> Fait, dans AircraftType il y a une fonction permettant de calculer la consommation pour 10 secondes à vitesse maximale.
> Elle n'est pas un champs puisque le framerate et la vitesse de simulation peuvent changer.