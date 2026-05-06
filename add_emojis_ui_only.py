#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Safe emoji addition script - ONLY modifies UI strings inside QStringLiteral()
Does NOT touch variable names, function names, or code logic
"""

import re
import sys

def add_emojis_to_ui_strings(content):
    """
    Add emojis only to user-visible strings inside QStringLiteral()
    Uses regex to match and replace specific patterns
    """
    
    # Define emoji mappings for UI strings only
    # Pattern: match QStringLiteral("text") and add emoji at the start
    replacements = [
        # Module titles and headers
        (r'QStringLiteral\("Produit([^"]*?)"\)', r'QStringLiteral("📦 Produit\1")'),
        (r'QStringLiteral\("Commande([^"]*?)"\)', r'QStringLiteral("📋 Commande\1")'),
        (r'QStringLiteral\("Employe([^"]*?)"\)', r'QStringLiteral("👤 Employe\1")'),
        (r'QStringLiteral\("Client([^"]*?)"\)', r'QStringLiteral("🤝 Client\1")'),
        (r'QStringLiteral\("Stock([^"]*?)"\)', r'QStringLiteral("📊 Stock\1")'),
        (r'QStringLiteral\("Depot([^"]*?)"\)', r'QStringLiteral("🏭 Depot\1")'),
        
        # Success/Error messages
        (r'alerteSucces\(QStringLiteral\("Succes"\)', r'alerteSucces(QStringLiteral("✅ Succes")'),
        (r'alerteErreur\(QStringLiteral\("Erreur"\)', r'alerteErreur(QStringLiteral("❌ Erreur")'),
        (r'alerteWarning\(QStringLiteral\("Attention"\)', r'alerteWarning(QStringLiteral("⚠️ Attention")'),
        (r'alerteInfo\(QStringLiteral\("Info"\)', r'alerteInfo(QStringLiteral("ℹ️ Info")'),
        
        # Specific UI labels
        (r'QStringLiteral\("Quantite"\)', r'QStringLiteral("📊 Quantite")'),
        (r'QStringLiteral\("Matiere"\)', r'QStringLiteral("🧵 Matiere")'),
        (r'QStringLiteral\("Etape"\)', r'QStringLiteral("⚙️ Etape")'),
        (r'QStringLiteral\("Fabrication"\)', r'QStringLiteral("🏭 Fabrication")'),
        (r'QStringLiteral\("Planification"\)', r'QStringLiteral("📅 Planification")'),
    ]
    
    modified = content
    for pattern, replacement in replacements:
        modified = re.sub(pattern, replacement, modified)
    
    return modified

def main():
    input_file = 'mainwindow.cpp'
    output_file = 'mainwindow.cpp'
    
    try:
        # Read the file with UTF-8 encoding
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        print(f"Original file size: {len(content)} characters")
        
        # Add emojis to UI strings only
        modified_content = add_emojis_to_ui_strings(content)
        
        print(f"Modified file size: {len(modified_content)} characters")
        
        # Write back with UTF-8 encoding
        with open(output_file, 'w', encoding='utf-8', newline='\n') as f:
            f.write(modified_content)
        
        print(f"✅ Successfully added emojis to UI strings in {output_file}")
        print("⚠️ IMPORTANT: Compile and test to ensure no errors!")
        
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
