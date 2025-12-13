# GitHub Actions Workflow - Build BO3 Enhanced XS

Este workflow compila automaticamente as DLLs quando você faz push de código.

## 🔄 O que faz:

- **Compila automaticamente** em cada push para `master`, `main` ou `develop`
- **Testa builds** Release e Debug para x64
- **Faz upload dos artifacts** (DLLs compiladas) para download
- **Cria releases** automaticamente quando você faz tag (ex: `v1.0.0`)

## 📦 Artifacts

Após cada build bem-sucedido, os arquivos aparecem em:
- **Actions** → Clica no workflow → Download dos artifacts

Os artifacts ficam salvos por 30 dias por padrão.

## 🏷️ Criar Release Automática

Para criar uma release com as DLLs:

```bash
git tag v1.0.0
git push origin v1.0.0
```

Isso vai:
1. Compilar as DLLs
2. Criar uma Release no GitHub automaticamente
3. Anexar as DLLs compiladas

## 🔧 Trigger Automático

O workflow roda automaticamente quando:
- Você faz `git push` em branches `master`, `main` ou `develop`
- Detecta mudanças em arquivos `.cpp`, `.h`, `.vcxproj`
- Ou ao editar o próprio workflow

## ❌ Requisitos

- MSBuild instalado (já vem no Windows)
- Visual Studio 2022 (v143 toolset)
- NuGet para restaurar pacotes

Tudo isso é instalado automaticamente nos runners do GitHub Actions!
