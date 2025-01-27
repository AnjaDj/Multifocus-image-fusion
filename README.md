# Projekat: Fuzija multifokusiranih slika primjenom EMD na ADSP-21489 platformi

## Opis projekta
U okviru ovog projekta razvija se sistem za generisanje **Empirijske vremensko-frekvencijske dekompozicije signala (EMD)** na razvojnom okruženju **ADSP-21489**. Sistem se testira na problemu fuzije multifokusiranih slika, kako bi se dobila slika u punom fokusu.

Empirijski način dekompozicije omogućava adaptivno razlaganje nelinearnih i nestacionarnih signala na **unutrašnje mod funkcije (IMFs)**, koje ukazuju na frekvencijski sadržaj signala.

## Struktura sistema
Sistem se sastoji od tri glavna dijela:

1. **Učitavanje podataka:**
   - Učitavanje dvije multifokusirane slike (.jpg ili .bmp formata).
   - Pretvaranje slika u grayscale format i formiranje 1D vektora iz redova/kolona.

2. **EMD dekompozicija:**
   - Primjena EMD algoritma za izdvajanje prvog nivoa dekompozicije (prvih IMFs) za ulazne vektore.
   - Rekonstrukcija novih slika iz dobijenih IMFs u 2D formatu.

3. **Generisanje maske odlučivanja i fuzija slika:**
   - Računanje lokalne varijanse na osnovu IMFs.
   - Kreiranje maske koja definiše najbolje fokusirane dijelove.
   - Fuzija slika u punom fokusu na osnovu maske.

