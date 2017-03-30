# Ordonnancement avec POSIX

## Utilisation
Compiler avec flag -lrt à la fin

Executer avec taskset -c X avec X votre numéro de coeur (partant de 0)

Par exemple : taskset -c 0 ./a.out

## Exercice 1
L'intérêt était d'essayer l'ordonnancement avec POSIX, c'est-à-dire utiliser un timer régulier pour qu'un processus affiche à l'écran

## Exercice 2
Dans ce cas on utilise deux processus qui s'alterne la charge. On calibre d'ailleurs la charge.

### Question 2
On affiche les échéances ratées

### Question 3
On retire l'affichage et on remarque qu'il y a moins d'échéances ratées.

### Question 4
On change la façon d'afficher avec une mémoire partager.

### Question 5
On ajoute une tâche supplémentaire.
