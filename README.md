# Zadanie 2 – Pipeline CI/CD z wieloarchitekturą i testem CVE

## Opis
Pipeline buduje obraz kontenera dla `linux/amd64` i `linux/arm64`, 
wykorzystuje cache na DockerHub (`lukaszp29/cache-zad2`) oraz skanuje 
obraz Trivy pod kątem podatności CRITICAL i HIGH. 
Bezpieczny obraz trafia do `ghcr.io/LP101-inf/cloud-lab2`.

## Tagowanie
- `latest` – najnowszy stabilny obraz z gałęzi `main`
- `sha-<krótki>` – niezmienny identyfikator commita
- `vX.Y.Z` – wersja wydania (tag git)

## Cache
Przechowywany w publicznym repozytorium DockerHub w trybie `registry/max`.
Zapewnia to maksymalne ponowne wykorzystanie warstw.

## Skanowanie CVE
Trivy (Aqua Security) – darmowe, bez limitów. Pipeline zostanie przerwany,
jeśli wykryje podatność CRITICAL lub HIGH.

## Konfiguracja
1. Utwórz publiczne repozytorium `cache-zad2` na DockerHub.
2. W GitHub dodaj sekrety: `DOCKERHUB_USERNAME`, `DOCKERHUB_TOKEN`.
3. Wypchnij kod na `main` – pipeline uruchomi się automatycznie.

## Autor
Łukasz Pisula
