IRCV3_SOURCES = [
	'purpleircv3capabilities.c',
	'purpleircv3connection.c',
	'purpleircv3core.c',
	'purpleircv3messagehandlers.c',
	'purpleircv3parser.c',
	'purpleircv3protocol.c',
	'purpleircv3protocolim.c',
	'purpleircv3sasl.c',
]

IRCV3_HEADERS = [
	'purpleircv3capabilities.h',
	'purpleircv3connection.h',
	'purpleircv3core.h',
	'purpleircv3messagehandlers.h',
	'purpleircv3parser.h',
	'purpleircv3protocol.h',
	'purpleircv3protocolim.h',
	'purpleircv3sasl.h',
]

if not DYNAMIC_IRCV3
	subdir_done()
endif

ircv3_filebase = f'purple-@purple_major_version@-ircv3'

ircv3_includes = include_directories('.')
ircv3_include_base = purple_include_base / 'protocols/ircv3'


ircv3_resources = gnome.compile_resources('ircv3resource',
	'resources/ircv3.gresource.xml',
	source_dir : 'resources',
	c_name : 'purple_ircv3')
IRCV3_SOURCES += ircv3_resources

ircv3_h_includes = []
foreach header : IRCV3_HEADERS
	ircv3_h_includes += f'#include <@header@>'
endforeach
ircv3_h_conf = configuration_data()
ircv3_h_conf.set('IRCV3_H_INCLUDES', '\n'.join(ircv3_h_includes))

ircv3_h = configure_file(input : 'purpleircv3.h.in',
                          output : 'purpleircv3.h',
                          configuration : ircv3_h_conf,
                          install : true,
                          install_dir : get_option('includedir') / ircv3_include_base)

install_headers(IRCV3_HEADERS,
	subdir : ircv3_include_base)

ircv3_prpl = shared_library('ircv3', IRCV3_SOURCES + IRCV3_HEADERS + [ircv3_h],
	c_args : ['-DPURPLE_IRCV3_COMPILATION', '-DG_LOG_USE_STRUCTURED', '-DG_LOG_DOMAIN="Purple-IRCv3"'],
	dependencies : [libpurple_dep, glib, gio, hasl, ws2_32],
	install : true,
	install_dir : PURPLE_PLUGINDIR)

ircv3_dep = declare_dependency(
	sources : [IRCV3_SOURCES, IRCV3_HEADERS],
	include_directories : ircv3_includes,
	dependencies : [libpurple_dep, glib, gio, hasl, ws2_32])

pkgconfig.generate(
	# we purposely don't put the library here because you should not be
	# linking to the plugin, everything will be resolved during runtime.
	name : 'ircv3',
	description : 'a purple3 protocol plugin for IRCv3',
	version : meson.project_version(),
	subdirs : [ircv3_include_base],
	filebase : ircv3_filebase,
	requires : [gio, glib, 'gplugin', 'hasl', libpurple])

meson.override_dependency(ircv3_filebase, ircv3_dep)

devenv.append('PURPLE_PLUGIN_PATH', meson.current_build_dir())

if get_option('introspection')
	GPLUGIN_INTROSPECTION = dependency('gplugin-introspection')

	plugin_filename = ircv3_prpl.full_path()

	ircv3_introspection_stub = executable('ircv3generategir',
		sources : 'ircv3generategir.c',
		dependencies : [ircv3_dep, libpurple_dep, glib, gio, hasl, ws2_32, GPLUGIN_INTROSPECTION],
		c_args : ['-DPURPLE_IRCV3_COMPILATION', f'-DPLUGIN_FILENAME="@plugin_filename@"'],
		install : false)

	ircv3_gir = gnome.generate_gir(
		ircv3_introspection_stub,
		sources : [IRCV3_SOURCES, IRCV3_HEADERS],
		includes : ['GLib-2.0', 'GObject-2.0', 'GPlugin-1.0', libpurple_gir[0]],
		namespace : 'PurpleIRCv3',
		symbol_prefix : 'purple_ircv3',
		nsversion : '1.0',
		install : true,
	    dependencies: [gplugin_dep],
		export_packages : ['ircv3'],
		extra_args : ['-DPURPLE_IRCV3_COMPILATION', '--verbose'])
endif

subdir('tests')
