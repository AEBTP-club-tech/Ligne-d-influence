"""
Module contenant le contenu HTML pour l'application d'analyse structurelle.
"""

ABOUT_CONTENT_HTML = """
<html>
<head>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.8; text-align: center; background-color: #2b2b2b; color: #ffffff; }
        h1 { color: #5dade2; margin-bottom: 20px; }
        h2 { color: #85c1e2; margin-top: 25px; border-bottom: 2px solid #3498db; padding-bottom: 5px; }
        p { margin: 10px 0; text-align: justify; color: #e0e0e0; }
        .version { font-size: 1.2em; color: #3498db; font-weight: bold; margin: 20px 0; }
        .description { background-color: #353535; padding: 15px; border-radius: 5px; margin: 15px 0; border: 1px solid #555555; }
        .tech { background-color: #353535; padding: 10px; border-radius: 5px; margin: 10px 0; border: 1px solid #555555; }
        ul { text-align: left; display: inline-block; color: #e0e0e0; }
        li { margin: 5px 0; }
        strong { color: #ffffff; }
    </style>
</head>
<body>
    <h1>🏗️ Analyse Structurelle</h1>
    <h2>Interface de Visualisation</h2>
    
    <div class="version">Version 1.0</div>
    
    <div class="description">
        <p><strong>Logiciel de visualisation des lignes d'influence</strong> pour l'analyse structurelle des tabliers de ponts.</p>
        <p>Cette application permet de visualiser, analyser et exporter les résultats de calculs structurels, 
        notamment les moments, forces de cisaillement, rotations et réactions d'appui.</p>
    </div>
    
    <h2>🔧 Technologies Utilisées</h2>
    <div class="tech">
        <ul>
            <li><strong>PyQt6</strong> : Interface graphique</li>
            <li><strong>Matplotlib</strong> : Visualisation et animation des graphiques</li>
            <li><strong>NumPy</strong> : Calculs numériques</li>
            <li><strong>Python</strong> : Langage de programmation</li>
        </ul>
    </div>
    
    <h2>📋 Fonctionnalités</h2>
    <ul>
        <li>Visualisation des lignes d'influence</li>
        <li>Animation des courbes</li>
        <li>Exportation en format GIF</li>
        <li>Configuration personnalisable</li>
        <li>Intégration avec calculs C++</li>
    </ul>
    
    <p style="margin-top: 30px; color: #7f8c8d; font-size: 0.9em;">
        Développé pour l'analyse structurelle des ponts
    </p>
</body>
</html>
"""

HELP_CONTENT_HTML = """
<html>
<head>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.6; background-color: #2b2b2b; color: #ffffff; }
        h1 { color: #5dade2; border-bottom: 3px solid #3498db; padding-bottom: 10px; }
        h2 { color: #85c1e2; margin-top: 25px; border-left: 4px solid #3498db; padding-left: 10px; }
        h3 { color: #aed6f1; margin-top: 20px; }
        p { margin: 10px 0; text-align: justify; color: #e0e0e0; }
        ul, ol { margin: 10px 0; padding-left: 30px; color: #e0e0e0; }
        li { margin: 5px 0; }
        code { background-color: #404040; color: #f39c12; padding: 2px 6px; border-radius: 3px; font-family: 'Courier New', monospace; }
        .section { margin: 20px 0; padding: 15px; background-color: #353535; border-radius: 5px; border: 1px solid #555555; }
        .icon { font-size: 1.2em; }
        .shortcut { background-color: #404040; color: #f39c12; padding: 2px 8px; border-radius: 3px; font-weight: bold; font-family: 'Courier New', monospace; }
        .warning { background-color: #4a3a00; color: #ffd700; padding: 10px; border-left: 4px solid #ffc107; margin: 10px 0; border-radius: 3px; }
        .info { background-color: #003d4a; color: #5dade2; padding: 10px; border-left: 4px solid #17a2b8; margin: 10px 0; border-radius: 3px; }
        strong { color: #ffffff; }
        em { color: #aed6f1; }
    </style>
</head>
<body>
    <h1>📚 Guide d'Utilisation - Analyse Structurelle</h1>
    
    <div class="section">
        <h2>🎯 Vue d'Ensemble</h2>
        <p>Cette application permet de visualiser et d'analyser les lignes d'influence pour l'analyse structurelle des tabliers de ponts. 
        Elle offre des fonctionnalités de tracé, d'animation et d'exportation des résultats de calcul.</p>
    </div>
    
    <div class="section">
        <h2>🚀 Démarrage Rapide</h2>
        <ol>
            <li><strong>Lancer le calcul C++</strong> : Cliquez sur l'icône <span class="icon">▶️</span> dans la barre d'outils ou utilisez le menu <em>Fichier → Lancer le Calcul C++</em></li>
            <li><strong>Sélectionner le type de courbe</strong> : Utilisez l'icône <span class="icon">📊</span> ou le menu <em>Configuration</em></li>
            <li><strong>Choisir la travée et la section</strong> : Utilisez les contrôles dans le panneau de configuration</li>
            <li><strong>Tracer la courbe</strong> : Cliquez sur l'icône <span class="icon">📈</span> ou appuyez sur <span class="shortcut">Ctrl+P</span></li>
            <li><strong>Animer</strong> : Utilisez l'icône <span class="icon">🎬</span> pour animer la courbe sélectionnée</li>
        </ol>
    </div>
    
    <div class="section">
        <h2>📋 Fonctionnalités Principales</h2>
        
        <h3>1. Tracé de Courbes</h3>
        <p>Visualisez les lignes d'influence pour différents types de courbes :</p>
        <ul>
            <li><strong>Moments de Travée</strong> (<code>span_moments</code>) : Moments dans les travées</li>
            <li><strong>Forces de Cisaillement</strong> (<code>span_shear_forces</code>) : Forces de cisaillement</li>
            <li><strong>Rotations</strong> (<code>span_rotations</code>) : Rotations aux nœuds</li>
            <li><strong>Moments d'Appui</strong> (<code>support_moments</code>) : Moments aux appuis</li>
            <li><strong>Réactions d'Appui</strong> (<code>support_reactions</code>) : Réactions aux appuis</li>
        </ul>
        
        <h3>2. Animation</h3>
        <p>Deux modes d'animation sont disponibles :</p>
        <ul>
            <li><strong>Animation Courbe</strong> : Anime uniquement la courbe sélectionnée</li>
            <li><strong>Animation Complète</strong> : Anime toutes les courbes avec les éléments structurels</li>
        </ul>
        <div class="info">
            <strong>💡 Astuce</strong> : Utilisez le slider dans le panneau de configuration pour ajuster la vitesse d'animation.
        </div>
        
        <h3>3. Affichage du Maximum</h3>
        <p>L'icône <span class="icon">📊</span> permet d'afficher automatiquement la courbe avec la plus grande aire de moment, 
        ce qui correspond généralement au cas le plus défavorable.</p>
        
        <h3>4. Exportation</h3>
        <p>Exportez vos visualisations :</p>
        <ul>
            <li><strong>Export Animation</strong> : Exporte l'animation en format GIF</li>
            <li><strong>Sauvegarde Configuration</strong> : Sauvegarde vos paramètres dans <code>Configuration.json</code></li>
        </ul>
    </div>
    
    <div class="section">
        <h2>⚙️ Configuration</h2>
        
        <h3>Options d'Affichage</h3>
        <ul>
            <li><strong>Grille</strong> : Affiche/masque la grille du graphique</li>
            <li><strong>Travées</strong> : Affiche les limites des travées sur le graphique</li>
            <li><strong>Nœuds</strong> : Affiche les positions des nœuds</li>
            <li><strong>Légende</strong> : Affiche/masque la légende</li>
            <li><strong>Inverser l'Axe Y</strong> : Inverse l'orientation de l'axe vertical</li>
        </ul>
        
        <h3>Édition de input.txt</h3>
        <p>L'icône <span class="icon">📝</span> permet d'éditer directement le fichier <code>input.txt</code> depuis l'interface. 
        Les modifications seront prises en compte lors du prochain calcul.</p>
        <div class="warning">
            <strong>⚠️ Attention</strong> : Assurez-vous de sauvegarder les modifications avant de lancer un nouveau calcul.
        </div>
    </div>
    
    <div class="section">
        <h2>🎨 Interface Utilisateur</h2>
        
        <h3>Barre d'Outils</h3>
        <p>La barre d'outils contient les actions principales :</p>
        <ul>
            <li><span class="icon">📈</span> <strong>Tracer</strong> : Trace la courbe sélectionnée</li>
            <li><span class="icon">🎬</span> <strong>Animer</strong> : Lance l'animation</li>
            <li><span class="icon">📊</span> <strong>Maximum</strong> : Affiche le maximum</li>
            <li><span class="icon">💾</span> <strong>Sauvegarder</strong> : Sauvegarde la configuration</li>
            <li><span class="icon">📤</span> <strong>Exporter</strong> : Exporte l'animation</li>
            <li><span class="icon">🔲</span> <strong>Plein Écran</strong> : Active le mode plein écran</li>
            <li><span class="icon">⚙️</span> <strong>Configuration</strong> : Ouvre le dialogue de configuration</li>
            <li><span class="icon">📝</span> <strong>Éditer input.txt</strong> : Édite le fichier d'entrée</li>
            <li><span class="icon">▶️</span> <strong>Lancer Calcul</strong> : Lance le calcul C++</li>
            <li><span class="icon">📊</span> <strong>Type de Courbe</strong> : Sélectionne le type de courbe</li>
            <li><span class="icon">🎯</span> <strong>Sélection Travée/Section</strong> : Ouvre le sélecteur</li>
            <li><span class="icon">👁️</span> <strong>Masquer/Afficher Panneau</strong> : Bascule la visibilité du panneau</li>
        </ul>
        
        <h3>Panneau de Configuration</h3>
        <p>Le panneau de gauche peut être :</p>
        <ul>
            <li><strong>Redimensionné</strong> : Glissez la bordure pour ajuster la taille</li>
            <li><strong>Masqué</strong> : Utilisez l'icône <span class="icon">👁️</span> ou le menu <em>Vue</em></li>
        </ul>
    </div>
    
    <div class="section">
        <h2>⌨️ Raccourcis Clavier</h2>
        <ul>
            <li><span class="shortcut">F1</span> : Afficher l'aide</li>
            <li><span class="shortcut">F11</span> : Basculer le mode plein écran</li>
            <li><span class="shortcut">Ctrl+P</span> : Tracer la courbe</li>
            <li><span class="shortcut">Ctrl+A</span> : Animer la courbe</li>
            <li><span class="shortcut">Ctrl+M</span> : Afficher le maximum</li>
            <li><span class="shortcut">Ctrl+S</span> : Sauvegarder la configuration</li>
            <li><span class="shortcut">Ctrl+E</span> : Exporter l'animation</li>
            <li><span class="shortcut">Ctrl+Q</span> : Quitter l'application</li>
        </ul>
    </div>
    
    <div class="section">
        <h2>📁 Structure des Fichiers</h2>
        <p>L'application utilise les fichiers suivants :</p>
        <ul>
            <li><code>input.txt</code> : Fichier d'entrée pour le calcul C++</li>
            <li><code>Configuration.json</code> : Fichier de configuration de l'interface</li>
            <li><code>data/results/</code> : Dossier contenant les résultats du calcul</li>
            <li><code>Ligne d'influence.exe</code> : Exécutable du calcul C++</li>
        </ul>
    </div>
    
    <div class="section">
        <h2>🔧 Dépannage</h2>
        
        <h3>Le calcul ne se lance pas</h3>
        <ul>
            <li>Vérifiez que <code>Ligne d'influence.exe</code> existe dans le répertoire</li>
            <li>Assurez-vous que <code>input.txt</code> est présent et valide</li>
            <li>Vérifiez les permissions d'exécution</li>
        </ul>
        
        <h3>Les données ne se chargent pas</h3>
        <ul>
            <li>Vérifiez que le calcul C++ s'est terminé avec succès</li>
            <li>Vérifiez que le dossier <code>data/results/</code> contient les fichiers JSON</li>
            <li>Relancez le calcul si nécessaire</li>
        </ul>
        
        <h3>L'animation ne fonctionne pas</h3>
        <ul>
            <li>Assurez-vous qu'une courbe est sélectionnée</li>
            <li>Vérifiez que les données sont chargées</li>
            <li>Essayez de tracer d'abord la courbe avant d'animer</li>
        </ul>
    </div>
    
    <div class="section">
        <h2>📞 Support</h2>
        <p>Pour toute question ou problème, consultez :</p>
        <ul>
            <li>Le menu <em>Aide → Raccourcis Clavier</em> pour la liste complète des raccourcis</li>
            <li>Le menu <em>Aide → À propos</em> pour les informations sur la version</li>
        </ul>
    </div>
    
    <div class="info">
        <strong>💡 Note</strong> : Cette application nécessite que le calcul C++ soit exécuté au préalable pour générer les données de visualisation.
    </div>
</body>
</html>
"""
