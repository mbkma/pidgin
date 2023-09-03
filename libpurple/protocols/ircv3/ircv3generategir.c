#include <glib.h>

#include <gplugin-introspection.h>

int
main(int argc, char *argv[]) {
    return gplugin_introspection_introspect_plugin(&argc, &argv,
                                                   PLUGIN_FILENAME);
}

