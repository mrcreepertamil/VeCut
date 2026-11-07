# VeCut JavaScript Plugins

VeCut is planned to support community plugins written in JavaScript. This folder contains the initial plugin format and an example plugin.

> **Development status:** This is the first draft of the plugin SDK contract. The JavaScript runtime, plugin manager, permission prompts, and editor integration are not implemented yet. Plugin files in this folder are examples and cannot be installed into VeCut yet.

## A plugin contains

- `manifest.json`: plugin identity, version, entry point, and requested permissions.
- `main.js`: JavaScript entry point.

## Quick start

Copy `example-plugin` as a starting point, then edit its manifest and JavaScript file.

## Initial API design

VeCut intends to provide a limited `veCut` API to each plugin. The first proposed API includes:

- `veCut.commands.register(...)`: add a command to the editor.
- `veCut.ui.showNotification(...)`: show a notification inside VeCut.
- `veCut.editor.getActiveProject()`: read basic information about the active project, when the plugin has the required permission.

Plugins will not receive unrestricted access to the app, device files, or operating system. VeCut will validate requested permissions and expose only supported APIs.

## Permissions

Plugins must declare the permissions they need in their manifest. VeCut should explain each permission before enabling a plugin. A plugin should request only the minimum permissions needed for its features.

The initial permission names are proposals and may change before the runtime is released:

- `ui.notifications`
- `editor.read`
- `editor.commands`

## Compatibility

The manifest includes an `apiVersion` field so VeCut can reject plugins that target an incompatible plugin API.
