# ChessKids

A remake of the classic ChessKids game built with Unreal Engine 5.7.

## Requirements

- Unreal Engine 5.7
- Python 3.x (optional — for asset generation scripts)

## Getting Started

1. Open `ChessKids.uproject` in Unreal Engine 5.7
2. The project will compile C++ sources and open automatically
3. Open `Content/Maps/L_Field.umap` for the main level

## Project Structure

- `Content/Maps/` — Game levels (`L_Field.umap` is the main field level)
- `Content/NaturePack/` — Vegetation and nature static meshes
- `Content/Materials/` — Landscape and sky materials
- `Content/GeneratedTextures/` — AI-generated textures (grass, dirt, rock, sky)
- `Source/ChessKids/` — C++ game logic
  - `ChessManager` — Chess game state and rules
  - `FieldSpawner` — Procedural foliage/vegetation spawner for L_Field

## Features

- Procedural field environment with sculpted landscape and foliage
- AI-generated textures via Stable Diffusion
- 3-layer auto-blend landscape material (grass/dirt/rock)
- C++ procedural vegetation spawner (`AFieldSpawner`)
- Chess board environment (in progress)
- Chess engine (in progress)
