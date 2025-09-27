![Visiteurs du projet](https://visitor-badge.laobi.icu/badge?page_id=rapporttecnologia.genai-e-book-reader)
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt](https://img.shields.io/badge/Qt6-Widgets-brightgreen)
![CMake](https://img.shields.io/badge/CMake-%3E%3D3.16-informational)
[![Docs](https://img.shields.io/badge/docs-Doxygen-blueviolet)](docs/index.html)
[![Latest Release](https://img.shields.io/github/v/release/RapportTecnologia/GenAi-E-Book-Reader?label=version)](https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/latest)
[![Contributions Welcome](https://img.shields.io/badge/contributions-welcome-success.svg)](#contribuer)

<!-- Traductions -->
**Traductions :** 🇧🇷 [Português (original)](README.md) | 🇸🇦 [العربية](README.ar.md) | 🇬🇧 [English (UK)](README.en-GB.md) | 🇫🇷 Français

<figure style="text-align: center;">
    <img src="docs/imgs/logo-do-projeto.png" alt="GenAI E-Book Reader">
    <figcaption>GenAI E-Book Reader</figcaption>
    </figure>

## Soutien financier du projet

Si ce projet vous est utile et que vous souhaitez soutenir le développement, vous pouvez envoyer un PIX du montant de votre choix. Merci !

![PIX](docs/imgs/pix.png)

PIX : _consultoria@carlosdelfino.eti.br_ ou _(+55 85) 98520‑5490_

# GenAI E‑Book Reader

Lecteur d’e‑books moderne axé sur la productivité et l’étude, développé en C/C++ avec Qt6, avec des fonctionnalités prévues telles que les annotations, un dictionnaire, la synthèse vocale (TTS), des statistiques de lecture et l’appui de l’IA (RAG) pour les résumés et explications.

- Historique des changements : voir [CHANGELOG.md](CHANGELOG.md).
- Planification des versions : voir [ROADMAP.md](ROADMAP.md).
- Tutoriel pas à pas : voir [TUTORIAL.fr-FR.md](TUTORIAL.fr-FR.md).

## Comment obtenir l’application

Vous pouvez télécharger la dernière version stable ou compiler la version de développement pour accéder aux dernières nouveautés.

### Version stable (recommandée)

La dernière version stable est **v0.1.13**. Pour la plupart des utilisateurs, nous recommandons de télécharger l’exécutable prêt à l’emploi.

1. Rendez‑vous sur la [page des releases](https://github.com/RapportTecnologia/GenAi-E-Book-Reader/releases/latest).
2. Téléchargez `GenAI_EBook_Reader-v0.1.13-x86_64.AppImage`.
3. Rendez le fichier exécutable :
    ```bash
    chmod +x GenAI_EBook_Reader-v0.1.13-x86_64.AppImage
    ```
4. Lancez l’application :
    ```bash
    ./GenAI_EBook_Reader-v0.1.13-x86_64.AppImage
    ```

### Version de développement

Si vous souhaitez tester les dernières fonctionnalités destinées à la prochaine version, vous pouvez compiler le projet depuis les sources. Cette version inclut de nouvelles fonctions et des corrections de bogues, mais peut être instable.

## Tutoriels
- 🇧🇷 [Tutoriel en portugais](TUTORIAL.md)
- 🇸🇦 [الدليل بالعربية](TUTORIAL.ar.md)
- 🇬🇧 [Tutoriel (anglais Royaume‑Uni)](TUTORIAL.en-GB.md)
- 🇫🇷 [Tutoriel (français)](TUTORIAL.fr-FR.md)

## Tutoriels vidéo
* [Présentation du projet](https://www.youtube.com/watch?v=4wveYzO_Lko)
* [Présentation OpenRouter.ia](https://www.youtube.com/watch?v=dHggyhodAH4&t=4s)
* [Associer des fichiers e‑book au GER](https://www.youtube.com/watch?v=2a1KO5Vig0k)

* [Autres vidéos](https://www.youtube.com/@RapportTecnologia/videos)

## Fonctionnalités clés (MVP)
- Lecture de PDF avec navigation basique et thèmes clair/sombre. (Prise en charge d’EPUB/MOBI au programme.)
- Surlignages et annotations avec panneau latéral et export JSON.
- Dictionnaire au clic (au moins 1 langue).
- TTS pour des passages sélectionnés, avec contrôles de base.
- IA (OpenAI) pour résumer/expliquer des passages avec RAG.
- Statistiques : temps actif, progression, sessions.
- Restauration de session (rouvre les livres ouverts précédemment).
- Intégration avec Calibre pour ouvrir des livres depuis la bibliothèque.
- Panneau de sommaire (TOC) avec barre d’outils :
  - Basculer entre la liste des pages (« Pages ») et l’arborescence des chapitres (« Contenu »).
  - Les boutons « Précédent » et « Suivant » naviguent page par page (mode Pages) ou élément/chapitre par chapitre (mode Contenu).
  - Largeur par défaut du panneau ~10 % et zone de lecture ~90 % au premier lancement (ajustable via le séparateur).
- Barre supérieure avec bouton de titre centré affichant le nom du document en cours et proposant un menu contextuel (ouvrir le dossier, ajouter à Calibre avec migration des embeddings, renommer avec migration).
- Barre de recherche avec recherche de texte littéral et repli vers la recherche sémantique par phrases via embeddings ; inclut des options rapides (métrique, Top‑K et seuils).
- Prise en charge des fichiers OPF (métadonnées d’ebooks) avec lecture et affichage basiques des informations.

### Notes de version

Note (0.1.12 – développement) : focus sur l’amélioration des recherches (texte et sémantique) et sur l’interaction du chat avec prise en charge du Function Calling lorsque le modèle/fournisseur le propose. La fenêtre des paramètres LLM affiche désormais un indicateur en lecture seule indiquant si le modèle sélectionné déclare prendre en charge le Function Calling.

Note (0.1.11) : nouveaux fournisseurs LLM pris en charge (Ollama local, GenerAtiva et OpenRouter), améliorations de l’interface des paramètres LLM (liste des modèles et test du modèle), ajustements CI et CMake (cible de release locale), débogage élargi pour la sélection fournisseurs/modèles, suppression de la dépendance à PHPList et interaction personnalisée utilisateur/LLM. Une clé de courtoisie pour un usage initial d’OpenRouter est incluse (il est recommandé de configurer votre propre clé).

Note (0.1.10) : prise en charge des liens internes cliquables dans les PDF (index/résumé au sein du document) via QPdfLinkModel (Qt6), améliorations de la navigation via le TOC (synchronisation avec les clics et les boutons Précédent/Suivant), corrections de sélection de texte et petits ajustements d’UI. Documentation mise à jour.

Note (0.1.7) : ajout de la possibilité d’ouvrir des e‑books directement en ligne de commande et d’associer des fichiers dans le système. Démarrage du dictionnaire (utilisant actuellement un LLM) et amélioration du panneau d’informations de l’application.

Note (0.1.6) : optimisation du rendu du chat en déplaçant la conversion Markdown vers le back‑end (C++), correction de bogues d’instabilité avec MathJax.

Note (0.1.3) : affinement de l’UI — icône de l’app depuis `docs/imgs/logo-do-projeto.png`, écran de démarrage avec version/auteur et titre de la fenêtre affichant le nom du livre (métadonnées lorsque disponibles ; sinon, nom du fichier). Ajustement pour que la zone de lecture occupe 100 % de l’espace disponible. Panneau TOC repensé avec barre d’outils (basculer entre « Pages »/« Contenu » et boutons de navigation) et taille par défaut initiale du séparateur ~10 % (TOC) / ~90 % (visionneuse).

Note (0.1.2) : implémentation de « Enregistrer sous » (RF‑28) et petites améliorations de lecture.

Note (0.1.1) : ajout de la sélection de page via une liste déroulante et restauration du dernier fichier/dossier ouvert.

Remarque : pour les PDF, le sommaire (TOC) utilise les signets (chapitres/sous‑chapitres) lorsqu’ils sont disponibles ; à défaut, toutes les pages sont listées. La sélection par liste déroulante couvre toutes les pages du document. Le panneau TOC inclut une barre d’outils pour basculer entre « Pages » et « Contenu » et des boutons de navigation ; par défaut, le panneau occupe ~10 % de la largeur de la fenêtre au premier lancement. Actuellement, le binaire prend en charge la lecture de PDF ; les fichiers OPF sont acceptés pour la lecture des métadonnées.

## Sommaire (TOC) et navigation
- Basculer le mode du TOC :
  - « Pages » : liste plate des pages ; les boutons « Précédent/Suivant » changent la page en cours.
  - « Contenu » : chapitres (groupes de pages) ; les boutons parcourent l’élément précédent/suivant (chapitre ou page enfant).
- Astuces panneau : faites glisser le séparateur pour redimensionner ; la taille est conservée pour les prochaines sessions.

## Recherche dans le document (texte et sémantique)

- La barre de recherche en haut contient :
  - Champ de recherche, boutons « Rechercher », « Précédent » et « Suivant ».
  - Menu « Options » avec réglages rapides de similarité : métrique (Cosinus, Dot ou L2), Top‑K, seuil de similarité (pour cosinus/dot) et distance max (pour L2).
- Fonctionnement :
  - La recherche tente d’abord de trouver le texte littéral dans le PDF (par page).
  - Si rien n’est trouvé, l’application effectue une recherche sémantique par phrases à l’aide de l’index d’embeddings du document et se rend aux pages les plus pertinentes.
- Prérequis pour la recherche sémantique : le document doit avoir des embeddings indexés.
  - Pour recréer l’index, cliquez droit dans le PDF et choisissez « Recréer les embeddings du document… ».
  - Vous pouvez aussi ajuster le fournisseur/modèle et les paramètres dans `Paramètres > Embeddings`.

### Bouton de titre (barre supérieure)

- Affiche le titre du document (pour les PDF, utilise la métadonnée Title si disponible ; sinon, le nom de fichier).
- Cliquez pour voir le chemin complet du fichier et le copier dans le presse‑papiers.
- Menu contextuel (clic droit) :
  - « Ouvrir le dossier dans le gestionnaire »
  - « Ajouter à Calibre et migrer les embeddings… »
  - « Renommer le fichier et migrer les embeddings… »

## IA (LLM) : configuration et utilisation
L’application s’intègre à des fournisseurs compatibles avec l’API OpenAI pour le chat, les résumés et les synonymes.
Actuellement pris en charge :
- OpenAI (`https://api.openai.com`)
- GenerAtiva (`https://generativa.rapport.tec.br`)
- Ollama (local, `http://localhost:11434`)
- OpenRouter (`https://openrouter.ai`)

Comment configurer :
- Ouvrez la boîte de dialogue « Paramètres LLM » (menu Paramètres).
- Sélectionnez le fournisseur (OpenAI, GenerAtiva, Ollama ou OpenRouter) et le modèle.
- Listez et sélectionnez un modèle disponible (lorsque le fournisseur propose la liste des modèles) et utilisez le bouton « Tester le modèle » pour valider les identifiants et la connectivité.
- Saisissez la clé API du fournisseur choisi (pour OpenAI/GenerAtiva/OpenRouter). Pour Ollama local, aucune clé n’est nécessaire ; assurez‑vous simplement que le service tourne sur `http://localhost:11434`.
- Facultatif : remplissez « Base URL » pour pointer vers un endpoint compatible OpenAI lorsque c’est pertinent.
- Ajustez les prompts par défaut pour Synonymes, Résumés, Explications et Chat selon vos préférences.

### Indicateur de Function Calling (0.1.10)
- La fenêtre des paramètres LLM affiche désormais un champ en lecture seule indiquant si le fournisseur/modèle sélectionné prend en charge le Function Calling.
- Lorsque l’API du fournisseur expose cette capacité via la liste des modèles ou des métadonnées, la détection est automatique et l’indicateur affiche « Prend en charge le Function Calling ». Sinon, un message informatif s’affiche (ex. : « Capacité non indiquée par le fournisseur »).
- Remarque : la prise en charge effective du Function Calling dépend du fournisseur/modèle choisi et peut évoluer.

Utilisation dans le lecteur :
- Synonymes : sélectionnez un mot/une locution et déclenchez l’action IA pour les synonymes ; une confirmation est demandée avant l’envoi.
- Résumé : sélectionnez un passage et déclenchez l’action de résumé ; le résultat s’ouvre dans la boîte de dialogue dédiée.
- Chat : envoyez un passage au chat IA ou saisissez librement dans le panneau de chat.
  - Rendu avancé dans le panneau de chat (Markdown/HTML) :
    - Tableaux Markdown (GFM) avec bordures, en‑tête et défilement horizontal si nécessaire.
    - Coloration syntaxique pour les blocs de code (highlight.js, thème GitHub).
    - MathJax v3 pour les formules (en ligne et affichées), appliqué après l’analyse Markdown.
    - Défilement automatique vers le dernier message reçu/envoyé.
  - Sessions de chat :
    - Le bouton « Nouveau » démarre une nouvelle conversation (propose d’enregistrer la conversation actuelle dans l’historique, avec titre automatique).
    - Le bouton « Historique » liste et restaure les conversations enregistrées (par fichier ouvert), en conservant le contexte IA.
  - Contexte continu : les nouveaux envois incluent l’historique complet des messages (system/user/assistant) pour une meilleure continuité.

Persistance/Paramètres (QSettings) :
- `ai/provider` : `openai` | `generativa` | `ollama` | `openrouter` (par défaut : `openai`)
- `ai/base_url` : URL de base de substitution (facultatif)
- `ai/api_key` : jeton secret du fournisseur (non applicable à Ollama local)
- `ai/model` : nom du modèle (ex. : `gpt-4o-mini`, `llama3`, `gpt-4o-mini-transcribe`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Notes de confidentialité :
- Avant tout envoi de contenu à l’IA, l’application demande votre confirmation.
- Les jetons sont stockés dans les préférences utilisateur (`QSettings`).

Pour un guide pas à pas avec images et conseils, voir [TUTORIAL.fr-FR.md](TUTORIAL.fr-FR.md).

## RAG (expérimental)

ATTENTION : cette fonctionnalité est en test et peut surcharger les ressources sur certaines machines, entraînant des blocages ou l’arrêt prématuré de l’application/du terminal. Nous stabilisons le pipeline avec un traitement par étapes et des limites de consommation.

### Définition
- Indexation des PDF en vecteurs pour permettre la recherche sémantique et des réponses contextualisées.
- Pipelines 100 % C++ (pas de Python côté client), avec fournisseurs d’embeddings via HTTP :
  - `openai`, `generativa` (compatible OpenAI) et `ollama` (local).
- Stockage vectoriel local sur disque (format binaire simple + JSON d’ids/métadonnées) dans `~/.cache/br.tec.rapport.genai-reader/`.

### Utilisation
1. Ouvrez un PDF dans le lecteur.
2. Cliquez droit dans le PDF et choisissez « Recréer les embeddings du document… ».
3. Suivez la boîte de progression (étapes, pourcentages, métriques). Si configuré par étapes, relancez pour reprendre là où vous vous êtes arrêté.

### Configuration (Paramètres > Embeddings)
- Fournisseur : `OpenAI`, `GenerAtiva` (Base URL et clé API) ou `Ollama` local.
- Modèle d’embeddings (ex. : `text-embedding-3-small`, `nomic-embed-text:latest`).
- Banque (dossier du cache) : par défaut `~/.cache/br.tec.rapport.genai-reader`.
- Réglages fins :
  - Taille de chunk (par défaut 1000)
  - Chevauchement (par défaut 200)
  - Taille de lot (batch) (par défaut 16)
  - Pages par étape (facultatif ; traite N pages par exécution pour éviter la surcharge)
  - Pause entre lots (ms) (facultatif ; insère une pause entre lots)

Valeurs sûres suggérées :
- Pages par étape : 10–25
- Pause entre lots : 100–250 ms
- Lot : 8–16
- Chunk size : 800–1200
- Overlap : 100–250

### Dépendances d’extraction de texte (recommandées)
- Préférée : `poppler-utils` (fournit `pdftotext` et `pdftoppm`) — plus rapide et moins gourmand en mémoire.
- Secours : `tesseract-ocr` (OCR par page ; plus lourd et plus lent, à utiliser seulement si nécessaire).
- L’application détecte automatiquement leur absence et suggère l’installation.

### État actuel et limitations
- Expérimental : surcharge CPU/RAM et arrêt de l’application/du terminal possibles pour de gros documents.
- Atténuations implémentées :
  - Traitement par étapes (s’arrête après N pages ; relancer pour continuer).
  - Écriture incrémentale des vecteurs/ids/métadonnées (sans énormes tampons en mémoire).
  - Throttling entre lots (pause configurable) et respect de l’annulation.
  - Journaux des étapes, métriques et progression dans la fenêtre d’indexation.
- Améliorations à venir :
  - Diagnostic des dépendances (état de `pdftotext`, `pdftoppm`, `tesseract`) dans l’UI.
  - Autres solutions de repli et optimisations d’E/S des métadonnées.

## Versions à venir
- Planification continue dans `ROADMAP.md`.

#### Compilation depuis les sources (Linux)
Prérequis : CMake (>=3.16), compilateur C++17, Qt6 (Widgets, PdfWidgets, Network), Doxygen (facultatif).

```bash
cmake -S . -B build
cmake --build build -j
# Exécutable : build/bin/genai_reader

# Générer la documentation
cmake --build build --target docs
# ou
doxygen docs/Doxyfile
# Ouvrir : docs/index.html
```

## Comment créer une release GitHub

Prérequis :
- Dépôt distant configuré (origin) et droits de push.
- Facultatif : GitHub CLI (`gh`) authentifié : `gh auth login`.

Étapes suggérées :

```bash
# 1) Vérifier que CHANGELOG.md et README.md sont à jour (ex. : 0.1.9)
git add CHANGELOG.md README.md
git commit -m "docs: mise à jour du changelog et du readme pour v0.1.9"

# 2) Versionner dans le code si applicable (CMakeLists.txt, headers) et valider
# Exemple (si modification de version de build)
# git add CMakeLists.txt include/app/App.h
# git commit -m "chore(release): bump version to v0.1.9"

# 3) Créer un tag annoté et le pousser
git tag -a v0.1.9 -m "v0.1.9"
git push origin v0.1.9

# 4A) Créer la release via GitHub CLI (en joignant les notes du CHANGELOG)
gh release create v0.1.9 \
  --title "v0.1.9" \
  --notes "Voir CHANGELOG.md pour le détail de cette version."

# 4B) Alternative : créer la release via l’UI GitHub
# - Aller sur Releases > Draft a new release > Choisir le tag v0.1.9 > Renseigner titre/notes > Publish

# 5) (Optionnel) Joindre les binaires
# Si vous avez des artefacts dans dist/, joindre avec :
# gh release upload v0.1.9 dist/genai_reader-v0.1.9-linux-x86_64 dist/genai_reader-v0.1.9-linux-x86_64.tar.gz
```

Remarque : le projet exige Qt6 (Widgets, PdfWidgets, Network). Sans Qt6, l’application ne compile pas.

## Dépendances de build par plateforme

### Linux
- Général :
  - Compilateur C++17 (g++/clang), CMake (>= 3.16)
  - Qt6 Widgets + Qt6 PdfWidgets + Qt6 Network
  - Doxygen (facultatif) pour la documentation

- Debian/Ubuntu (24.04+ suggéré) :
  ```bash
  sudo apt update
  sudo apt install -y build-essential cmake ninja-build \
      qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
      qt6-pdf-dev doxygen graphviz
  # Alternative (Poppler au lieu de Qt PDF) :
  # sudo apt install -y libpoppler-qt6-dev
  ```

- Fedora
  ```bash
  sudo dnf install -y gcc-c++ cmake ninja-build \
      qt6-qtbase-devel qt6-qttools-devel qt6-qttools \
      qt6-qtpdf-devel doxygen graphviz
  # Alternative Poppler-Qt6 : sudo dnf install -y poppler-qt6-devel
  ```

- Arch/Manjaro
  ```bash
  sudo pacman -S --needed base-devel cmake ninja \
      qt6-base qt6-tools qt6-pdf doxygen graphviz
  # Alternative Poppler-Qt6 : sudo pacman -S poppler-qt6
  ```

Notes :
- Si votre distribution n’a pas de paquet Qt PDF, utilisez l’équivalent Poppler‑Qt6.
- Si l’include `QPdfBookmarkModel` n’est pas disponible, le TOC des PDF reviendra automatiquement à la liste des pages.

### Windows
- Option A : Visual Studio + Qt (recommandé)
  1) Installer Visual Studio (Community) ou « Build Tools for Visual Studio » avec MSVC C++.
  2) Installer Qt via le Qt Online Installer (même compilateur/version MSVC). Inclure Qt Widgets et Qt PDF.
  3) Installer CMake et Ninja (optionnel, améliore les builds) :
     - CMake : https://cmake.org/download/
     - Ninja : https://github.com/ninja-build/ninja/releases
  4) Définir `CMAKE_PREFIX_PATH` vers `.../Qt/<version>/<kit>/lib/cmake` (Qt6) :
     - Exemple : `set CMAKE_PREFIX_PATH=C:\\Qt\\6.7.0\\msvc2022_64\\lib\\cmake`
  5) Générer et compiler :
  ```bat
  cmake -S . -B build -G "Ninja"
  cmake --build build -j
  ```

- Option B : MSYS2/MinGW
  1) Installer MSYS2 et mettre à jour.
  2) Installer toolchain et Qt :
  ```bash
  pacman -S --needed mingw-w64-ucrt-x86_64-toolchain \
      mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
      mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools \
      mingw-w64-ucrt-x86_64-qt6-pdf doxygen graphviz
  ```
  3) Ouvrir le shell UCRT64/MinGW correspondant et compiler avec CMake.

## Profil lecteur (optionnel)
- L’application propose une boîte « Données du lecteur » (`Fichier > Lecteur > Données du lecteur...`) pour enregistrer les préférences utilisateur.
- Tous les champs sont optionnels et peuvent servir à personnaliser les interactions :
  - Nom
  - E‑mail
  - WhatsApp
  - Surnom (utilisé par les LLM pour s’adresser à vous, si approprié)

Remarques :
- Aucune donnée n’est envoyée vers l’extérieur par l’application. Les données sont stockées localement via `QSettings`.
- Si un « Surnom » est indiqué, il sera pris en compte dans les instructions système envoyées aux LLM pour personnaliser l’adresse.

## Contribuer
Les contributions sont les bienvenues ! Vous pouvez aider de plusieurs façons :
- Signaler des bogues et ouvrir des issues avec des suggestions d’amélioration.
- Implémenter des fonctionnalités du MVP selon `REQUISITOS.md`.
- Proposer des améliorations d’UX, de documentation ou d’exemples.

Étapes suggérées :
1. Forkez le dépôt.
2. Créez une branche pour votre fonctionnalité/correction : `git checkout -b feat/nom-de-fonctionnalite`.
3. Des commits petits et ciblés avec des messages clairs.
4. Ouvrez une Pull Request décrivant le problème/la solution et référant les RF/RNF/CA affectés.

## Code de conduite
Soyez respectueux et collaboratif. Les comportements abusifs ne seront pas tolérés. Utilisez un langage courtois et empathique dans les interactions.

## Licence
Ce projet est sous licence Creative Commons Attribution 4.0 International (CC BY 4.0). Voir le fichier [`LICENSE`](LICENSE).

Vous êtes libre de :
- Partager — copier et redistribuer le matériel sur tout support ou format.
- Adapter — remixer, transformer et créer à partir du matériel pour tout usage, même commercial.

À condition de créditer de manière appropriée et d’indiquer les modifications éventuelles. Voir les termes complets :
- https://creativecommons.org/licenses/by/4.0/

## Remerciements
- Communauté Qt et bibliothèques de lecture de documents (Poppler/Qt PDF, etc.).
- Projet Calibre pour l’inspiration de l’intégration.
- Contributeurs et testeurs qui aident à améliorer le projet.
