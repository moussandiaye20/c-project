# Gestion des ressources

## Analyse de la gestion des avions

La création des avions est aujourd'hui gérée par les fonctions `TowerSimulation::create_aircraft` et `TowerSimulation::create_random_aircraft`.
Chaque avion créé est ensuite placé dans les files `GL::display_queue` et `GL::move_queue`.

Si à un moment quelconque du programme, vous souhaitiez accéder à l'avion ayant le numéro de vol "AF1250", que devriez-vous faire ?
> chercher dans la move_queue ou dans la display_queue

---

## Objectif 1 - Référencement des avions

### A - Choisir l'architecture

Pour trouver un avion particulier dans le programme, ce serait pratique d'avoir une classe qui référence tous les avions et qui peut donc nous renvoyer celui qui nous intéresse.

Vous avez 2 choix possibles :
- créer une nouvelle classe, `AircraftManager`, qui assumera ce rôle,
- donner ce rôle à une classe existante.

Réfléchissez aux pour et contre de chacune de ces options.
> AircraftManager -> une classe = une responssabilité mais force à trainer un objet de plus dans tous le programme  
> Dans une classe existante -> on a déjà des classes présente dans tous le programme mais on regroupe plusieurs responssabilitées dans une seul classe

Pour le restant de l'exercice, vous partirez sur le premier choix.

### B - Déterminer le propriétaire de chaque avion

Vous allez introduire une nouvelle liste de références sur les avions du programme.
Il serait donc bon de savoir qui est censé détruire les avions du programme, afin de déterminer comment vous allez pouvoir mettre à jour votre gestionnaire d'avions lorsque l'un d'entre eux disparaît.

Répondez aux questions suivantes :
1. Qui est responsable de détruire les avions du programme ? (si vous ne trouvez pas, faites/continuez la question 4 dans TASK_0)
> opengl_interface.cpp dans la move_queue

2. Quelles autres structures contiennent une référence sur un avion au moment où il doit être détruit ?
> GL::display_queue et reserved_terminals

3. Comment fait-on pour supprimer la référence sur un avion qui va être détruit dans ces structures ?
> On doit faire une recherche et supprimer manuellement

4. Pourquoi n'est-il pas très judicieux d'essayer d'appliquer la même chose pour votre `AircraftManager` ?
> Parce qu'on veut que AircraftManager controlle la durée de vie des avions

Pour simplifier le problème, vous allez déplacer l'ownership des avions dans la classe `AircraftManager`.
Vous allez également faire en sorte que ce soit cette classe qui s'occupe de déplacer les avions, et non plus la fonction `timer`.

### C - C'est parti !

Ajoutez un attribut `aircrafts` dans le gestionnaire d'avions.
Choisissez un type qui met bien en avant le fait que `AircraftManager` est propriétaire des avions.
> std::unique_ptr<Aircraft> 

Ajoutez un nouvel attribut `aircraft_manager` dans la classe `TowerSimulation`.

Modifiez ensuite le code afin que `timer` passe forcément par le gestionnaire d'avions pour déplacer les avions.
Faites le nécessaire pour que le gestionnaire supprime les avions après qu'ils aient décollé.
> On fait donc en sorte que Aircraft_Manager soit un DynamicObject pour qu'il puisse bouger par la suite les avions.
> On peut supprimer le retour de move de DynamicObject puisqu'elle n'est utile que pour les Aircraft
> qui auront donc une autre fonction de déplacement (la même qu'avant mais sans dépendre de DynamicObject)

Enfin, faites ce qu'il faut pour que `create_aircraft` donne l'avion qu'elle crée au gestionnaire.
Testez que le programme fonctionne toujours.

---

## Objectif 2 - Usine à avions

### A - Création d'une factory

La création des avions est faite à partir des composants suivants :
- `create_aircraft`
- `create_random_aircraft`
- `airlines`
- `aircraft_types`.

Pour éviter l'usage de variables globales, vous allez créer une classe `AircraftFactory` dont le rôle est de créer des avions.

Définissez cette classe, instanciez-la à l'endroit qui vous paraît le plus approprié, et refactorisez le code pour l'utiliser.
Vous devriez du coup pouvoir supprimer les variables globales `airlines` et `aircraft_types`.

### B - Conflits

Il est rare, mais possible, que deux avions soient créés avec le même numéro de vol.
Ajoutez un conteneur dans votre classe `AircraftFactory` contenant tous les numéros de vol déjà utilisés.
Faites maintenant en sorte qu'il ne soit plus possible de créer deux fois un avion avec le même numéro de vol.
> Ajout d'un unordered_set dans Aircraft_Factory stoquant les lignes déjà prises.


### C - Data-driven AircraftType (optionnel)

On aimerait pouvoir charger les paramètres des avions depuis un fichier.

Définissez un format de fichier qui contiendrait les informations de chaque `AircraftType` disponible dans le programme.\
Ajoutez une fonction `AircraftFactory::LoadTypes(const MediaPath&)` permettant de charger ce fichier.
Les anciens `AircraftTypes` sont supprimés.
> Le format du fichier est :
> ground_speed air_speed acceleration image  
> (Attention ça devient `ground_speed air_speed acceleration fuel_consumption max_fuel image` à la fin)

Modifiez ensuite le `main`, afin de permettre à l'utilisateur de passer le chemin de ce fichier via les paramètres du programme.
S'il ne le fait pas, on utilise la liste de type par défaut.
> Le seul fichier existant pour l'instant est 'media/data_files/default' qui contient les valeurs paar défaut.
> Le fichier peut être saisi au lancement du programme dans les paramètres du fichier.
> Si le fichier n'existe pas ou si aucun fichier n'est saisi, les valeurs par défaut sont prises. 
> Si le fichier ne correspond pas à ce qui est attendu un erreur est levée.

Si vous voulez de nouveaux sprites, vous pouvez en trouver sur [cette page](http://www.as-st.com/ttd/planes/planes.html)
(un peu de retouche par GIMP est necessaire)
> Il n'y a pas de nouveaux sprites.

---

## Objectif 3 - Pool de textures (optionnel)

> Dans mon implémentation les types ne sont créés qu'une seule fois dans 
> Aircraft_Factory et les Aircrafts contiennent une référence sur le bon type
> (il ne devrait pas y avoir de dangling reference puisque la durée de vie de 
> Aircraft_Factory est celle de tout le programme).
> Mon implémentation est correct tant qu'il n'y a pas une trop grosse quantité de sprites et/ou
> qu'ils ne sont pas trop volumineux.
> 
> Mais si je devais implémenter la pool de texture j'utiliserai un shared_ptr pour
> que la texture n'existe qu'une seule fois tant qu'elle est utilisée.

Pour le moment, chacun des `AircraftType` contient et charge ses propres sprites.
On pourrait néanmoins avoir différents `AircraftType` qui utilisent les mêmes sprites.
Ils seraient donc chargés plusieurs fois depuis le disque pour rien.

Pour rendre le programme un peu plus performant, implémentez une classe `TexturePool` qui s'occupe de charger, stocker et fournir les textures.
Réfléchissez bien au type que vous allez utiliser pour référencer les textures, afin d'exprimer correctement l'ownership.

Listez les classes qui ont besoin de `TexturePool`.
Sachant que vous n'aurez qu'une seule instance de `TexturePool` dans votre programme, quelle classe devra assumer l'ownership de cet objet ?\
Instanciez `TexturePool` au bon endroit et refactorisez le code afin que tous les chargements de textures utilisent ce nouvel objet.
Assurez-vous que votre implémentation ne génère pas des fuites de mémoire au moment de sa destruction.
