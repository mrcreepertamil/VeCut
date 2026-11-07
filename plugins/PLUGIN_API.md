# VeCut Plugin API — Draft v1

This document describes the proposed first version of the JavaScript plugin API.

## Entry point

VeCut loads the plugin entry file in an isolated JavaScript context and provides a host-controlled `veCut` API object. The entry file must define a `plugin` object with an `activate(veCut)` function. An optional `deactivate()` function is called when the plugin is disabled.

## Commands

```js
veCut.commands.register({
  id: "my-plugin.example-command",
  title: "Example command",
  run() {
    veCut.ui.showNotification({
      title: "Plugin",
      message: "Command executed."
    });
  }
});
```

Command identifiers must be unique. VeCut owns the command registry and should remove a plugin's registered commands when that plugin is disabled.

## Notifications

```js
veCut.ui.showNotification({
  title: "Finished",
  message: "The plugin completed its task."
});
```

## Security requirements for the future runtime

- Run each plugin in an isolated JavaScript context.
- Do not expose Node.js, Dart internals, arbitrary platform channels, or unrestricted filesystem access.
- Do not grant network access by default.
- Validate every requested permission and every host API call.
- Provide a way to disable and remove a plugin.
- Handle plugin exceptions so one plugin cannot crash the editor.
- Apply execution limits so a plugin cannot freeze the editor indefinitely.

This API is a draft contract, not a working runtime. The actual implementation must be completed and tested before third-party plugins can run.
