# Assertions et exceptions

## Objectif 1 - Crash des avions

Actuellement, quand un avion s'écrase, une exception de type `AircraftCrash` (qui est un alias de `std::runtime_error` déclaré dans `config.hpp`) est lancée.

1. Faites en sorte que le programme puisse continuer de s'exécuter après le crash d'un avion. Pour cela, remontez l'erreur jusqu'à un endroit approprié pour procéder à la suppression de cet avion (assurez-vous bien que plus personne ne référence l'avion une fois l'exception traitée). Vous afficherez également le message d'erreur de l'exception dans `cerr`.
> Le traitement de l'erreur est fait dans AircraftManager::move_aircraft.
2. Introduisez un compteur qui est incrémenté chaque fois qu'un avion s'écrase. Choisissez une touche du clavier qui n'a pas encore été utilisée (`m` par exemple ?) et affichez ce nombre dans la console lorsque l'utilisateur appuie dessus.
> Ajout d'un champs + de l'incrémentation dans le catch de AircraftManager::move_aircraft.
3. Si vous avez fini d'implémenter la gestion du kérosène (Task_2 - Objectif 2 - A), lancez une exception de type `AircraftCrash` lorsqu'un avion tombe à court d'esssence. Normalement, cette exception devrait être traitée de la même manière que lorsqu'un avion s'écrase parce qu'il a attérit trop vite.
> Fait, l'incrémentation est aussi faite 
> (je sais pas s'il fallait mais s'il fallait pas il suffit de récupérer la `reason` et de tester sa valeur).
4. **BONUS** Rédéfinissez `AircraftCrash` en tant que classe héritant de `std::runtime_error`, plutôt qu'en tant qu'alias. Arrangez-vous pour que son constructeur accepte le numéro de vol, la position, la vitesse de l'avion au moment du crash, et la raison du crash (`"out of fuel"` / `"bad landing"`). Vous utiliserez toutes ces informations pour générer le joli message de l'exception.
> Fait, cf. aircraftCrash.hpp

## Objectif 2 - Détecter les erreurs de programmation

Pour sécuriser votre code, repassez sur les différentes fonctions de votre programme et ajoutez des assertions permettant de vérifier qu'elles sont correctement utilisées.
Voici quelques idées :
- A: fonctions d'initialisation appelées une seule fois
- B: état attendu d'un objet lorsqu'une fonction est appelée dessus
- C: vérification de certains paramètres de fonctions
- D: Destructor & Constructors (base, copy, assign);

|         | A | B | C | D |
|---------|---|---|---|---|
|Aircraft | ☑ | ☑ | ☑ | ☑ |
|ACType   | ☑ | ☑ | ☑ | ☑ |
|ACFactory| ☑ | ☑ | ☑ | ☑ |
|ACManager| ☑ | ☑ | ☑ | ☑ |
|Airport  | ☑ | ☑ | ☑ | ☑ |
|APType   | ☑ | ☑ | ☑ | ☑ |
|Config   | ☑ | ☑ | ☑ | ☑ |
|Geometry | ☑ | ☑ | ☑ | ☑ |
|main     | ☑ | ☑ | ☑ | ☑ |
|runway   | ☑ | ☑ | ☑ | ☑ |
|Terminal | ☑ | ☑ | ☑ | ☑ |
|Tower    | ☑ | ☑ | ☑ | ☑ |
|TowerSim | ☑ | ☑ | ☑ | ☑ |
|Waypoint | ☑ | ☑ | ☑ | ☑ |