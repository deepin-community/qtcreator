import qbs 1.0

QtcPlugin {
    name: "Coco"

    Depends { name: "Core" }
    Depends { name: "LanguageClient" }
    Depends { name: "TextEditor" }

    Depends { name: "Qt"; submodules: ["widgets"] }

    files: [
        "cocoplugin.cpp",
        "cocolanguageclient.cpp",
        "cocolanguageclient.h",
    ]
}

