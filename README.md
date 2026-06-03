# Zadanie 2 – Pipeline CI/CD z wieloarchitekturą i testem CVE

Pipeline w GitHub Actions buduje obraz kontenera z kodem źródłowym z zadania 1 dla dwóch architektur: linux/amd64 i linux/arm64. Wykorzystuje cache na DockerHubie, a przed publikacją w ghcr.io skanuje obraz narzędziem Trivy. Jeśli zostaną znalezione podatności o poziomie krytycznym lub wysokim, obraz nie zostanie wypchnięty.

## Tagowanie obrazów

Obraz otrzymuje trzy typy tagów:
- **latest** – zawsze wskazuje najnowszy obraz zbudowany z gałęzi main. Dzięki temu można łatwo pobrać ostatnią działającą wersję.
- **sha-xxxxxxx** (np. sha-1a2b3c4) – tag oparty na krótkim identyfikatorze commita. Jest niezmienny i pozwala jednoznacznie powiązać obraz z konkretnym punktem w historii repozytorium.
- **vX.Y.Z** – tworzony automatycznie, gdy wypchnięty zostanie tag git (wydanie). Ułatwia śledzenie wersji.

Takie podejście łączy wygodę (latest) z powtarzalnością i bezpieczeństwem (sha, tagi wydań). Rezygnuję z tagów opartych na nazwach gałęzi, żeby nie zaśmiecać rejestru niepotrzebnymi wpisami. Inspiracją były zalecenia dokumentacji Docker oraz akcji docker/metadata-action.

## Cache warstw

Cache jest przechowywany w publicznym repozytorium na DockerHub: `lukaszp29/cache-zad2`. Używam tagu `cache`, który przy każdym budowaniu jest nadpisywany.

- **Eksporter registry** – dane cache zapisywane są bezpośrednio w rejestrze DockerHub, bez potrzeby stawiania własnego serwera cache.
- **Tryb max** – eksportowane są wszystkie warstwy pośrednie, nie tylko ostateczny obraz. Dzięki temu przy kolejnych kompilacjach Buildx może ponownie wykorzystać dużo więcej danych, nawet jeśli coś w Dockerfile się zmieni.

Zastosowanie `mode=max` w połączeniu z backendem registry jest rekomendowane przez dokumentację Docker Buildx dla środowisk CI/CD – znacząco skraca czas budowania, co było widać już przy drugim uruchomieniu pipeline’u.

## Skanowanie CVE

Do testowania obrazu wybrałem Trivy zamiast Docker Scout, głównie dlatego, że Trivy jest całkowicie darmowy, otwarty i nie ma limitów na liczbę skanowań. W pipeline używam oficjalnej akcji `aquasecurity/trivy-action` z ustawionym filtrem `--severity CRITICAL,HIGH`. Dzięki temu, jeśli w obrazie pojawi się poważna podatność, krok kończy się błędem i cały job jest zatrzymywany – obraz nie trafia do ghcr.io. Docker Scout w planie free narzuca ograniczenia, co mogłoby utrudnić spełnienie wymagań zadania.

## Konfiguracja i uruchomienie

Aby pipeline działał, trzeba:

1. Na DockerHub założyć publiczne repozytorium `cache-zad2`.
2. W ustawieniach repozytorium GitHub dodać sekrety:
   - `DOCKERHUB_USERNAME` – nazwa użytkownika DockerHub
   - `DOCKERHUB_TOKEN` – token z uprawnieniami do odczytu i zapisu
3. Wypchnąć kod na gałąź `main` – workflow uruchomi się automatycznie.

## Potwierdzenie działania

W zakładce **Actions** znajduje się historia udanych uruchomień. Po pomyślnym przejściu wszystkich kroków obraz jest dostępny w GitHub Packages (ghcr.io) z tagami `latest` i `sha-...`. Cache można podejrzeć w repozytorium `lukaszp29/cache-zad2` na DockerHub w tagu `cache`.

Autor: Łukasz Pisula