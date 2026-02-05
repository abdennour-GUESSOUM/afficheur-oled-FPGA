# Afficheur OLED VHDL FPGA

Projet de contrôle d'afficheur OLED sur FPGA Zynq-7000 (ZedBoard).

## Structure du projet

```
.
├── Vivado/          # Projet Vivado pour la conception matérielle
│   ├── OLED.xpr     # Fichier projet Vivado
│   └── OLED.srcs/   # Sources VHDL et diagrammes de blocs
└── Vitis/           # Projet Vitis pour l'application embarquée
    └── OLED/        # Application de contrôle OLED
        └── src/     # Code source C
```

## Fichiers importants

- **[Vivado/OLED.xpr](Vivado/OLED.xpr)** - Projet Vivado principal
- **[Vivado/design_1_wrapper.xsa](Vivado/design_1_wrapper.xsa)** - Export matériel pour Vitis
- **[Vitis/OLED/src/oled_driver.c](Vitis/OLED/src/oled_driver.c)** - Driver OLED personnalisé
- **[Vitis/OLED/src/oled_driver.h](Vitis/OLED/src/oled_driver.h)** - Interface du driver

## Prérequis

- Xilinx Vivado 2023.2
- Xilinx Vitis 2023.2
- ZedBoard (Zynq-7000 xc7z020clg484-1)

## Workflow de développement

### 1. Vivado (Conception matérielle)

```bash
cd Vivado
vivado OLED.xpr
```

Après modification du design:
1. Générer le bitstream
2. Exporter le hardware : `File > Export > Export Hardware`
3. Le fichier `.xsa` sera mis à jour

### 2. Vitis (Application embarquée)

```bash
cd Vitis
vitis -workspace .
```

Configuration de la plateforme:
1. Importer le fichier `design_1_wrapper.xsa` depuis Vivado
2. La plateforme sera automatiquement régénérée dans `platform/`

## Fichiers exclus du dépôt

Les fichiers suivants ne sont **pas inclus** dans Git (108 MB):

- Artefacts de build Vivado (~39 MB)
- Artefacts de build Vitis (~69 MB)
- BSP de la plateforme (à régénérer depuis le `.xsa`)
- Fichiers de cache et logs

Pour reconstruire le projet, il suffit d'ouvrir le projet Vivado et Vitis, puis de lancer les builds.

## Notes

- Le projet utilise le processeur ARM Cortex-A9 du Zynq
- Le BSP standalone est configuré pour ps7_cortexa9_0
- Les fichiers générés sont automatiquement exclus par `.gitignore`
