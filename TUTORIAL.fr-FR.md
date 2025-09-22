# GenAI E‑Book Reader — Tutoriel

Ce guide explique comment utiliser les principales fonctionnalités de l’application, de la lecture de base aux capacités avancées d’IA comme la recherche sémantique (RAG) et le chat avec un LLM.

## 1. Premiers pas : ouverture et navigation

1. **Ouvrir un livre** : allez dans `Fichier > Ouvrir...` pour sélectionner un fichier PDF.
2. **Navigation** :
   * Utilisez la molette de la souris pour faire défiler les pages.
   * Utilisez `Ctrl + molette de la souris` pour zoomer/dézoomer.
3. **Sommaire (TOC)** : le panneau de gauche permet de naviguer dans le contenu. Utilisez les boutons en haut du panneau pour basculer entre les vues **Pages** et **Contenu** (chapitres).

## 2. Lecture interactive : sélection, dictionnaire et recherche

### Sélectionner du texte et des images

- **Sélection de texte** : dans le menu `Édition`, choisissez `Sélectionner le texte` et marquez le passage souhaité.
- **Sélection d’image** : choisissez `Sélectionner un rectangle (image)` pour capturer une zone du document en image.
- **Actions** : avec du texte ou une image sélectionné(e), vous pouvez copier, enregistrer dans un fichier, ou envoyer au chat IA.

### Consulter le dictionnaire

1. **Configurer** : allez dans `Paramètres > Dictionnaire`. Par défaut, le dictionnaire utilise le LLM configuré pour fournir des définitions.
2. **Utiliser** : sélectionnez un mot dans le texte et faites un clic droit. Dans le menu contextuel, choisissez l’option de consultation du dictionnaire. La définition s’affiche dans le panneau de chat.

### Rechercher dans le document

La barre de recherche en haut propose deux types de recherche :

- **Recherche textuelle** : saisissez un mot ou une phrase et cliquez sur « Rechercher ». L’application localise les correspondances exactes dans le texte.
- **Recherche sémantique (RAG)** : si la recherche textuelle ne renvoie rien, l’application effectue automatiquement une recherche sémantique, qui comprend le sens de votre requête. Pour fonctionner, le document doit d’abord être indexé (voir la section RAG ci‑dessous).

## 3. Recherche sémantique avec RAG (Retrieval‑Augmented Generation)

RAG vous permet de poser des questions en langage naturel sur le contenu du livre. Pour l’utiliser, vous devez d’abord créer un « index d’embeddings » pour votre document.

### Étape 1 : configurer les embeddings

1. Allez dans `Paramètres > Embeddings`.
2. **Fournisseur** : choisissez un fournisseur pour générer les embeddings. `Ollama` est une excellente option locale.
3. **Modèle** : sélectionnez un modèle d’embeddings (ex. : `nomic-embed-text`).
4. **Paramètres de découpage** : ajustez la `taille de chunk` et le `chevauchement` pour contrôler le découpage du texte avant traitement. Les valeurs par défaut conviennent généralement.

### Étape 2 : indexer le document

1. Livre ouvert, faites un clic droit dans la visionneuse et choisissez `Recréer les embeddings du document…`.
2. Patientez jusqu’à la fin du processus. Une fenêtre affiche l’avancement.

### Étape 3 : effectuer une recherche sémantique

- Utilisez maintenant la barre de recherche pour poser des questions comme « Quelle est l’idée principale du chapitre 2 ? » ou « Expliquez le concept X ». L’application utilise l’index RAG pour trouver les passages les plus pertinents et répondre.

## 4. Intégration IA (LLM)

Vous pouvez connecter l’application à un service LLM pour utiliser le chat, demander des résumés, des synonymes, et plus encore.

### Configurer l’accès au LLM

1. Allez dans `Paramètres > Paramètres LLM`.
2. **Fournisseur** : choisissez `OpenAI`, `GenerAtiva`, `Ollama` (local) ou `OpenRouter`.
3. **Lister/Tester le modèle** : utilisez « Lister les modèles » (si disponible) et le bouton « Tester le modèle » pour valider identifiants et connectivité.
4. **Clé API** : renseignez votre clé API (pour les services cloud). Pour `Ollama`, aucun jeton n’est nécessaire ; assurez‑vous que le service tourne sur `http://localhost:11434`.
5. **Modèle** : sélectionnez le modèle à utiliser (ex. : `gpt-4o-mini`, `llama3`).
6. **Prompts** : personnalisez les prompts utilisés par l’IA pour les résumés, synonymes, etc.

Les paramètres sont stockés dans `QSettings` avec les clés :
- `ai/provider` : `openai` | `generativa` | `ollama` | `openrouter`
- `ai/base_url` : URL de base (facultatif)
- `ai/api_key` : jeton secret (non applicable pour `ollama`)
- `ai/model` : nom du modèle (ex. : `gpt-4o-mini`, `llama3`)
- `ai/prompts/synonyms`, `ai/prompts/summaries`, `ai/prompts/explanations`, `ai/prompts/chat`

Confidentialité :
- L’application demande toujours confirmation avant d’envoyer un passage à l’IA.
- Les jetons sont stockés dans les préférences utilisateur.

### Utiliser les fonctions IA

- **Chat** : ouvrez le panneau de chat pour converser avec l’IA. Posez des questions, demandez des explications, ou envoyez des passages et images du livre pour analyse.
- **Résumés et synonymes** : sélectionnez un texte, clic droit, puis choisissez résumer ou chercher des synonymes.
- **Dictionnaire** : sélectionnez un mot et utilisez la fonction dictionnaire pour obtenir sa définition dans le panneau de chat.

Remarques :
- Les messages utilisent le modèle défini dans les Paramètres LLM.
- Les prompts de dialogue sont personnalisables dans les Paramètres LLM.
- En 0.1.9, compatibilité ajoutée avec `OpenRouter`. Une clé de courtoisie peut être fournie pour des tests ; nous recommandons de configurer votre propre clé pour un usage continu.

## 5. Conseils et ressources supplémentaires

- **Bouton de titre** : le bouton au centre de la barre supérieure affiche le nom de votre livre. Clic droit pour des actions rapides, comme renommer le fichier ou l’ouvrir dans le gestionnaire de fichiers.
- **Dépannage** : si l’IA ne répond pas, vérifiez les paramètres LLM (clé API, URL de base) et votre connexion Internet.
- **Raccourcis** : utilisez `Ctrl+Entrée` pour envoyer des messages dans le panneau de chat.
