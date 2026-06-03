# Zadanie 2 – Pipeline CI/CD z wieloarchitekturą i testem CVE

## Opis rozwiązania
Pipeline GitHub Actions automatycznie buduje obraz kontenera dla dwóch architektur (`linux/amd64`, `linux/arm64`), wykorzystuje cache warstw przechowywany w publicznym repozytorium DockerHub, a następnie skanuje obraz narzędziem **Trivy** w poszukiwaniu krytycznych i wysokich podatności. Tylko bezpieczny obraz jest publikowany w rejestrze `ghcr.io`.

---

### Sposób tagowania obrazów
Przyjęto następującą konwencję tagowania (realizowaną przez akcję `docker/metadata-action`):

- **`latest`** – zawsze wskazuje ostatni obraz zbudowany z gałęzi `main`. Ułatwia szybkie pobranie najnowszej stabilnej wersji.
- **`sha-<krótki_skrót>`** (np. `sha-1a2b3c4`) – jednoznacznie identyfikuje obraz z konkretnym commitem. Zapewnia pełną powtarzalność i audytowalność.
- **`vX.Y.Z`** – tag tworzony automatycznie przy wypchnięciu tagu Git (wydania). Pozwala na semantyczne wersjonowanie.

**Uzasadnienie:**  
Zgodnie z dobrymi praktykami opisanymi w dokumentacji Docker i GitHub, niezmienne tagi (oparte na skrócie commita) umożliwiają odtworzenie dokładnego obrazu, podczas gdy `latest` pełni rolę wygodnego wskaźnika dla środowisk deweloperskich. Rezygnacja z tagów opartych na nazwie gałęzi (poza `main`) zapobiega zaśmiecaniu rejestru niepotrzebnymi wpisami. Źródło: [Docker Tag Best Practices](https://docs.docker.com/build/building/tagging/) oraz [docker/metadata-action docs](https://github.com/docker/metadata-action#usage).

---

### Strategia cache’owania
Cache warstw budowania przechowywany jest w publicznym repozytorium DockerHub (`lukaszp29/cache-zad2`) pod stałym tagiem `cache`. Zastosowano:

- **Eksporter:** `type=registry` – dane cache zapisywane są bezpośrednio w rejestrze kontenerów, co nie wymaga dodatkowej infrastruktury.
- **Backend:** `mode=max` – eksportuje wszystkie warstwy pośrednie (nie tylko końcowego obrazu), co maksymalizuje współczynnik trafień cache przy kolejnych buildach.

Każde uruchomienie pipeline’u nadpisuje tag `cache`, dzięki czemu zawsze dostępny jest najświeższy zestaw warstw. Działa to jednocześnie dla obu architektur, ponieważ Buildx scala warstwy dla każdej platformy.

**Uzasadnienie:**  
Zgodnie z oficjalną dokumentacją Docker Buildx ([Cache backends](https://docs.docker.com/build/cache/backends/registry/)), backend `registry` w trybie `max` jest zalecany dla środowisk CI/CD, gdyż przechowuje pełny graf budowania i znacząco skraca czas przebudowy (nawet o 60-80% w przypadku niewielkich zmian w Dockerfile). Wybór publicznego DockerHub jako miejsca przechowywania cache jest podyktowany jego dostępnością i brakiem dodatkowych kosztów.

---

### Test CVE – wybór narzędzia
Do skanowania podatności wykorzystano **Trivy** (Aqua Security).  
**Dlaczego Trivy, a nie Docker Scout?**
- Trivy jest w pełni darmowy i otwartoźródłowy, nie nakłada limitów na liczbę skanowań.
- Posiada oficjalną akcję GitHub Actions (`aquasecurity/trivy-action`), co upraszcza integrację.
- Umożliwia precyzyjne filtrowanie podatności według poziomu krytyczności (`--severity CRITICAL,HIGH`) i zwraca niezerowy kod wyjścia, co natychmiast przerywa pipeline.
Docker Scout w planie free ma ograniczenia, które mogłyby utrudnić realizację testu. Źródło: [Trivy GitHub Action](https://github.com/aquasecurity/trivy-action).

---

### Konfiguracja i uruchomienie
1. Utwórz publiczne repozytorium `cache-zad2` na DockerHub.
2. W ustawieniach repozytorium GitHub (`Settings → Secrets and variables → Actions`) dodaj dwa sekrety:
   - `DOCKERHUB_USERNAME` – nazwa użytkownika DockerHub (np. `lukaszp29`)
   - `DOCKERHUB_TOKEN` – token dostępu z odpowiednimi uprawnieniami
3. Wypchnij kod na gałąź `main` – workflow `Build, Scan, and Push` uruchomi się automatycznie.

---

###  Potwierdzenie działania
W zakładce **Actions** znajduje się udokumentowane wykonanie workflow, które zakończyło się sukcesem. Obraz z tagami `latest` i `sha-xxxx` jest dostępny w publicznym rejestrze GitHub Packages.

---

**Autor:** Łukasz Pisula