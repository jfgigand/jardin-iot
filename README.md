# Arrosage automatique de jardin

## Le système CAPTEUR

Émet les mesures sur le 433 MHz (voir common/jardin.h).

Prend les mesures suivantes :

* Température/humidité (DHT) voire pression (BMC)
* Luminosité (photorésistance)
* Humidité dans le soil (moisture sensor)
* Présence de pluie (sur plaque PCB)
* Pluviomètre (mesure quantité de pluie dans la durée)

Informe, via led WS2812B (ruban) :

* Pluviométrie : 1 bip lumineux par 0.2mm de pluie ?


## Le système CUVE

Actionne, via carte relais :

* Pompe
* Électro-vanne 


Prend les mesures :

* Niveau d'eau cuve
* Détecteur cuve vide/pleine (flotteur), en redondance du niveau
* Pression de l'eau (réseau eau de pluie)
* Débit de l’eau en sortie de pompe

Informe, via led WS2812B (ruban) :

* Du fonctionnement de la pompe et l'électro-vanne
* De la pression de l’eau

## Idées

* Utiliser quelques LED WS2812B en ruban pour donner des
  informations : pluviométrie

