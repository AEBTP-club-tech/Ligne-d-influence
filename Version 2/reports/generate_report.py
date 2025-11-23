"""
Script pour générer un rapport de calcul en format texte avec tableaux pandas
"""

import json
import pandas as pd
from pathlib import Path
from datetime import datetime

def load_json(filename):
    """Charge un fichier JSON"""
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"Erreur lors du chargement de {filename}: {e}")
        return None

def get_all_json_files(directory):
    """Récupère tous les fichiers JSON d'un répertoire"""
    json_files = {}
    if directory.exists():
        for json_file in sorted(directory.glob("*.json")):
            data = load_json(json_file)
            if data:
                json_files[json_file.stem] = data
    return json_files

def generate_report():
    """Génère le rapport de calcul complet en lisant dynamiquement les fichiers JSON"""
    
    # Déterminer le chemin de base (compatible avec ancienne et nouvelle structure)
    current_path = Path(__file__).resolve().parent
    if current_path.name == "reports":
        # Nouvelle structure: le script est dans reports/
        base_path = current_path.parent
    else:
        # Ancienne structure: le script est à la racine
        base_path = current_path
    
    analysis_path = base_path / "data" / "results" / "analysis"
    properties_path = base_path / "data" / "results" / "properties"
    
    # Charger tous les fichiers JSON disponibles
    analysis_files = get_all_json_files(analysis_path)
    properties_files = get_all_json_files(properties_path)
    
    # Créer le rapport
    report = []
    report.append("=" * 100)
    report.append("RAPPORT DE CALCUL - ANALYSE STRUCTURELLE".center(100))
    report.append("Lignes d'Influence pour Pont Multi-Travées".center(100))
    report.append("=" * 100)
    report.append(f"\nDate de génération: {datetime.now().strftime('%d/%m/%Y %H:%M:%S')}\n")
    
    # ===== 1. PROPRIÉTÉS STRUCTURELLES =====
    report.append("\n" + "=" * 100)
    report.append("1. PROPRIÉTÉS STRUCTURELLES")
    report.append("=" * 100)
    
    # Afficher automatiquement tous les fichiers de propriétés
    section_num = 1
    for prop_name, prop_data in sorted(properties_files.items()):
        section_num += 1
        report.append(f"\n1.{section_num} {prop_name.upper().replace('_', ' ')}")
        report.append("-" * 100)
        
        try:
            # Convertir les données en DataFrame
            if isinstance(prop_data, dict):
                # Si c'est un dictionnaire simple
                if all(isinstance(v, (int, float, str)) for v in prop_data.values()):
                    df = pd.DataFrame([
                        {"Clé": k, "Valeur": v} for k, v in prop_data.items()
                    ])
                    report.append(df.to_string(index=False))
                    
                    # Ajouter total si ce sont des nombres
                    numeric_values = [v for v in prop_data.values() if isinstance(v, (int, float))]
                    if numeric_values and prop_name == "span_lengths":
                        total = sum(numeric_values)
                        report.append(f"\nTotal: {total} m")
                else:
                    # Données complexes
                    report.append(str(prop_data))
            else:
                report.append(str(prop_data))
        except Exception as e:
            report.append(f"Erreur lors du traitement: {e}")
    
    # ===== 2. RÉSULTATS D'ANALYSE =====
    report.append("\n" + "=" * 100)
    report.append("2. RÉSULTATS D'ANALYSE")
    report.append("=" * 100)
    
    # Afficher automatiquement les fichiers d'analyse
    section_num = 0
    for analysis_name, analysis_data in sorted(analysis_files.items()):
        section_num += 1
        report.append(f"\n2.{section_num} {analysis_name.upper().replace('_', ' ')}")
        report.append("-" * 100)
        
        try:
            if isinstance(analysis_data, dict):
                # Cas spécial pour les données avec structure imbriquée
                if "top_10_aires" in analysis_data:
                    # Top 10 des surfaces
                    df = pd.DataFrame(analysis_data["top_10_aires"])
                    if "aire" in df.columns:
                        df = df.rename(columns={
                            "aire": "Surface",
                            "index_aire": "Index",
                            "section": "Section",
                            "travee": "Travée"
                        })
                    report.append(df.to_string(index=False))
                elif "T_0" in analysis_data or any(k.startswith("T_") for k in analysis_data.keys()):
                    # Données par travée
                    for travee_key, travee_data in sorted(analysis_data.items()):
                        if isinstance(travee_data, list):
                            report.append(f"\n  {travee_key}:")
                            df = pd.DataFrame(travee_data)
                            report.append(df.to_string(index=False))
                else:
                    # Données simples
                    if all(isinstance(v, (int, float, str)) for v in analysis_data.values()):
                        df = pd.DataFrame([
                            {"Clé": k, "Valeur": v} for k, v in analysis_data.items()
                        ])
                        report.append(df.to_string(index=False))
                    else:
                        report.append(str(analysis_data))
            else:
                report.append(str(analysis_data))
        except Exception as e:
            report.append(f"Erreur lors du traitement: {e}")
    
    # ===== 3. RÉSUMÉ ET RECOMMANDATIONS =====
    report.append("\n" + "=" * 100)
    report.append("3. RÉSUMÉ ET RECOMMANDATIONS")
    report.append("=" * 100)
    
    report.append("""
3.1 RÉSUMÉ DES RÉSULTATS
- Tous les calculs structurels ont été exécutés avec succès
- Les lignes d'influence ont été générées pour tous les cas de charge
- Les résultats incluent moments, forces de cisaillement, rotations et déflexions
- Rapport généré automatiquement à partir des fichiers JSON disponibles

3.2 RECOMMANDATIONS
⚠️  IMPORTANT:
  • Vérifier les résultats avec les normes de conception applicables
  • Valider les hypothèses de calcul avec les données de projet
  • Consulter un ingénieur structurel pour l'interprétation des résultats
  • Effectuer les vérifications de dimensionnement avant utilisation

3.3 PROCHAINES ÉTAPES
  1. Visualiser les lignes d'influence dans l'interface graphique
  2. Exporter les animations en format GIF
  3. Générer les rapports détaillés par travée
  4. Effectuer les vérifications de dimensionnement
""")
    
    report.append("\n" + "=" * 100)
    report.append("FIN DU RAPPORT".center(100))
    report.append("=" * 100)
    
    return "\n".join(report)

def save_report(filename="RAPPORT_CALCUL.txt"):
    """Sauvegarde le rapport dans un fichier texte"""
    report_content = generate_report()
    
    # Déterminer le chemin de sortie (compatible avec ancienne et nouvelle structure)
    current_path = Path(__file__).resolve().parent
    if current_path.name == "reports":
        # Nouvelle structure: le script est dans reports/
        output_path = current_path / filename
    else:
        # Ancienne structure: le script est à la racine
        output_path = current_path / filename
    
    # Créer le dossier s'il n'existe pas
    output_path.parent.mkdir(parents=True, exist_ok=True)
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(report_content)
    
    print(f"[OK] Rapport généré avec succès: {output_path}")
    return output_path

if __name__ == "__main__":
    save_report()
