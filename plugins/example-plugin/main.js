/*
 * Hello VeCut — example JavaScript plugin.
 *
 * This demonstrates the proposed plugin API. It will become executable
 * after VeCut's JavaScript runtime and plugin manager are implemented.
 */

const plugin = {
  activate(veCut) {
    veCut.commands.register({
      id: "hello-ve-cut.say-hello",
      title: "Say Hello to VeCut",
      run() {
        veCut.ui.showNotification({
          title: "Hello!",
          message: "Your first VeCut plugin is working."
        });
      }
    });
  },

  deactivate() {
    // VeCut will remove commands registered by this plugin when it is disabled.
  }
};
