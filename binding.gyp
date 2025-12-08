{
	"targets": [
		{
			"target_name": "xmljs",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 },
      },
			"cflags!": ["-fno-exceptions"],
			"cflags_cc!": ["-fno-exceptions"],
			"product_extension": "node",
			"type": "shared_library",
	"include_dirs": [
        'vendor/libxml2-config',
        'vendor/libxml2/include',
        'vendor/libxml2/include/libxml',
		  "<!(node -p \"require('node-addon-api').include_dir\")",
        "<!(node -p \"require('node-addon-api').include\")",
	],
			"cflags": ["-Wall"],
			"xcode_settings": {
				"GCC_ENABLE_CPP_EXCEPTIONS": "YES",
				"CLANG_CXX_LIBRARY": "libc++",
				"MACOSX_DEPLOYMENT_TARGET": "10.7",
				"OTHER_CFLAGS": ["-Wall"]
			},
			"win_delay_load_hook": "true",
			"defines": [
				"LIBXML_XINCLUDE_ENABLED",
				"LIBXML_SCHEMATRON_ENABLED",
        "LIBXML_RELAXNG_ENABLED",
				"BUILDING_NODE_EXTENSION",
				"NODE_ADDON_API_CPP_EXCEPTIONS"
			],
			"sources": [
				"src/libxmljs.cc",
				"src/xml_attribute.cc",
				"src/xml_document.cc",
				"src/xml_element.cc",
				"src/xml_comment.cc",
				"src/xml_namespace.cc",
				"src/xml_node.cc",
				"src/xml_sax_parser.cc",
				"src/xml_syntax_error.cc",
				"src/xml_textwriter.cc",
				"src/xml_text.cc",
				"src/xml_pi.cc",
				"src/xml_xpath_context.cc",
				"src/html_document.cc",
        "vendor/libxml2/HTMLparser.c",
        "vendor/libxml2/HTMLtree.c",
        "vendor/libxml2/SAX2.c",
        "vendor/libxml2/buf.c",
        "vendor/libxml2/c14n.c",
        "vendor/libxml2/catalog.c",
        "vendor/libxml2/chvalid.c",
        "vendor/libxml2/debugXML.c",
        "vendor/libxml2/dict.c",
        "vendor/libxml2/encoding.c",
        "vendor/libxml2/entities.c",
        "vendor/libxml2/error.c",
        "vendor/libxml2/globals.c",
        "vendor/libxml2/hash.c",
        "vendor/libxml2/list.c",
        "vendor/libxml2/nanohttp.c",
        "vendor/libxml2/parser.c",
        "vendor/libxml2/parserInternals.c",
        "vendor/libxml2/pattern.c",
        "vendor/libxml2/relaxng.c",
        "vendor/libxml2/schematron.c",
        "vendor/libxml2/shell.c",
        "vendor/libxml2/threads.c",
        "vendor/libxml2/tree.c",
        "vendor/libxml2/uri.c",
        "vendor/libxml2/valid.c",
        "vendor/libxml2/xinclude.c",
        "vendor/libxml2/xlink.c",
        "vendor/libxml2/xmlIO.c",
        "vendor/libxml2/xmlcatalog.c",
        "vendor/libxml2/xmllint.c",
        "vendor/libxml2/xmlmemory.c",
        "vendor/libxml2/xmlmodule.c",
        "vendor/libxml2/xmlreader.c",
        "vendor/libxml2/xmlregexp.c",
        "vendor/libxml2/xmlsave.c",
        "vendor/libxml2/xmlschemas.c",
        "vendor/libxml2/xmlschemastypes.c",
        "vendor/libxml2/xmlstring.c",
        "vendor/libxml2/xmlwriter.c",
        "vendor/libxml2/xpath.c",
        "vendor/libxml2/xpointer.c",
			],
			"conditions": [
				[
					"OS==\"mac\"",
					{
						#  node-gyp 2.x doesn't add this anymore
						#  https://github.com/TooTallNate/node-gyp/pull/612
						"xcode_settings": {
							"CLANG_CXX_LANGUAGE_STANDARD": "c++20",
							"OTHER_LDFLAGS": ["-undefined dynamic_lookup"]
						}
					}
				]
			]
		}
	]
}
