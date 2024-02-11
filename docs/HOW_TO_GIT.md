# Git command cheatsheet 
Sprawdź [git-cheat-sheet-education](https://education.github.com/git-cheat-sheet-education.pdf).

## TOC
- [Pojęcia](#pojęcia)
- [Ścieżka zdrowia](#ścieżka-zdrowia) (podstawowe komendy)
- [Inne przydatne komendy](#inne-przydatne-komendy)
- [Plik .gitignore](#plik-gitignore)
- [Instalacja Gita](#instalacja-gita)
  - [Dodatkowe rzeczy, które możesz zrobić](#dodatkowe-rzeczy-które-możesz-zrobić)

## Pojęcia 
- **branch** - wydzielony flow gdzie można pracować niezależnie od innych branchy
- **commit** - zapisany stan projektu w danym momencie
- **stage** - miejsce gdzie git przechowuje zmiany, które zostaną dodane do commita
- **lokalne repo** (local) - repozytorium na twoim komputerze
- **zdalne repo** (remote) - repozytorium na serwerze (np. github)
- **merge** - "dodanie" zmian z jednego brancha do drugiego
- **pull request**/**merge request** - prośba o zmergowanie brancha do innego brancha (na zdalnym repo)
- **stash** - lokalny schowek, by zapisać zmiany na później (np. gdy chcemy zmienić branch, ale nie chcemy commitować zmian)
- **tracked** - plik, który jest śledzony przez git (np. plik, który był dodany do commita)

## Ścieżka zdrowia
```bash
# status - pokazuje aktualnie wybrany branch i niezapisane zmiany
git status

# zmienia branch na inny
git checkout <branch-name>
## przykładowo na branch o nazwie "jacek/first-project"
git checkout jacek/first-project

# tworzy nowy branch
git checkout -b <branch-name>
## przykładowo na branch o nazwie "jacek/first-project"
git checkout -b jacek/first-project

# dodaje zmiany na "stage"
git add <plik>
## przykładowo dodaje wszystko w tym folderze i podfolderach
git add .

# tworzy commit z zmiana na "stage"
git commit -m "nazwa commita"

# tworzy commit z wszystkich zmian (nie trzeba robic git add)
git commit -am "nazwa commita"

# wysyła zmiany do zdalnego repo (np. github)
git push

# jeżeli to twój pierwszy push na nowym branch (HEAD na capslocku):
git push -u origin HEAD

# pobiera zmiany z zdalnego repo (np. github)
# i aktualizuje aktualnie wybrany branch
git pull
```

## Inne przydatne komendy
```bash
# historia commitów na wybranym branchu
git log

# lista branchy
git branch

# lista wszystkich branchy (lokalne i zdalne)
git branch -a

# zaciąga zmiany z zdalnego repo, ale nie aktualizuje lokalnych branchy
git fetch

# usuwa branch (lokalny)
git branch -d <branch-name>

# przenosi trackowane zmiany do stash
git stash

# wyciąga ostatnie zmiany z stash 
git stash pop

# wyciąga ostatnie zmiany z stash (bez usuwania ich z stash)
git stash apply

# wyciąga konkretne zmiany z stash do stage (bez usuwania)
git stash apply stash@{<stash-number>}
## przykładowo przenosi zmiany z stash o numerze 0
git stash apply stash@{0}

# wyświetla listę zmian w stash
git stash list

# usuwa ostatnie zmiany z stash
git stash drop

# usuwa wszystkie zmiany w śledzonych plikach:
# UWAGA reset jest nieodwracalny!
# UWAGA --hard kasuje dane z dysku!
git reset --hard HEAD

# przechodzi na inny commit (analogicznie jak zmiana brancha)
# przydatne jak chcemy przeglądnąć stary kod albo wprowadzić niezależny bugfix
git checkout <commit-hash>
## przykładowo na commit o hashu "a1b2c3d4" (wystarczy pierwsze 8 znaków)
git checkout a1b2c3d4
## powrót na branch
git checkout <branch-name>

# przechodzi na poprzedni branch (lub commit) na którym byliśmy przed wykonaniem `git checkout`
git checkout -
```

## Plik .gitignore
Plik .gitignore pozwala na ignorowanie plików i folderów w repozytorium. Nie będą one śledzone 
przez co, nie zostaną dodane do commita. Uwaga, jeżeli plik był śledzony wcześniej, to nie zostanie
usunięty z repozytorium. Aby usunąć plik z repozytorium, należy użyć komendy `git rm <plik>`.
```gitignore
# ignorowanie pliku
path/do/pliku.txt

# ignoruje wszystkie pliki w folderze i podfolderach
path/do/folderu/

# śledzi plik, mimo że jest w ignorowanej ścieżce
!path/do/pliku.txt

# śledzi folder, mimo że jest w ignorowanej ścieżce
!path/do/folderu/
```




## Instalacja gita
- Pobrać Git: https://git-scm.com/
  - **linux**: zainstalować pakiet `git` (chyba na kazdej dystrybucji jest pod taką nazwą):
    - arch: `pacman -S git`
    - ubuntu: `apt install git`
    - fedora: `dnf install git` lub `yum install git`
  - **windows**:
    - pobrać installator
    - przeczytać licencję (to ważne)
    - w którymś z kolejnych okien instalatora **wybrać opcję "Add to PATH"** 
    - Warto również zaznaczyć "install git bash" (dodaje parę przydatnych komend i terminal z bash)
    - Zainstalować
- Wykonac podstawową konfigurację wpisując polecenia w terminalu (w dowolnym miejscu)
  - `git config --global user.name "Imie Nazwisko"` (wprowadź swoje imię i nazwisko)
  - `git config --global user.email "twoj.email@domena.com"` (wprowadź taki sam email jak na github)

### Dodatkowe rzeczy, które możesz zrobić

**Skonfigurować domyslny edytor tekstu** (na windows możesz to zrobić w oknie instalacji łatwiej). Przyda się jak zapomnisz `-m` przy `git commit` lub będziesz chciał napisać dłuższy commit message.

- Wybierz swój ulubiony edytor, np visual studio code (vscode)
- Upewnij się że twój edytor można uruchomić z terminala (wpisując w terminalu np. 
  `code jakis/plik.txt`)
- Upewnij się że twój edytor odpali się w sposób blokujący i zakończy proces jak zamkniesz otwarty
  w nim plik (np. `code --wait jakis/plik.txt`)
- Wpisz w terminalu `git config --global core.editor "TWOJ EDYTOR"`
  - vscode: `git config --global core.editor "code --wait"`
  - nano: `git config --global core.editor "nano"`
  - notepad++: `git config --global core.editor "'C:/Program Files (x86)/Notepad++/notepad++.exe' -multiInst -notabbar -nosession -noPlugin"`
    
<br>

**Skonfigurować połączenie ssh z kontem github**

Poradnik GitHub: [Connecting to GitHub with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh)
