import qbs 1.0

Project {

    Product {
        name: "cavern"
        type: "application"
        consoleApplication: true

        Depends { name: "cpp" }

        Group {
            fileTagsFilter: ["application"]
            qbs.install: true
            qbs.installDir: project.installDir
        }

        Group {
            name: "Shared Files"
            qbs.install: true
            qbs.installDir: project.installDir

            files: [
                "share"
            ]
        }

        Group {
            name: "src"
            fileTags: ["cpp"]
            files: [
                "src/cavern.c",
                "src/filename.c",
                "src/commands.c",
                "src/netartic.c",
                "src/netbits.c",
                "src/netskel.c",
                "src/network.c",
                "src/listpos.c",
                "src/message.c",
                "src/matrix.c",
                "src/datain.c",
                "src/str.c",
                "src/readval.c",
                "src/namecmp.c",
                "src/date.c",
                "src/img_hosted.c",
                "src/hash.c",
                "src/osdepend.c",
                "src/cmdline.c"
            ]
        }

        files: [
            "src/filename.h",
            "src/commands.h",
            "src/netartic.h",
            "src/netbits.h",
            "src/listpos.h",
            "src/netskel.h",
            "src/network.h",
            "src/message.h",
            "src/matrix.h",
            "src/datain.h",
            "src/str.h",
            "src/readval.h",
            "src/namecmp.h",
            "src/date.h",
            "src/img_hosted.h",
            "src/hash.h",
            "src/osdepend.h",
            "src/cmdline.h"
        ]

        cpp.includePaths: ["src", "."]
    }

    StaticLibrary {
        name: "cavern-3dReader"

        Depends { name: "cpp" }
        cpp.defines: ["IMG_API_VERSION=1", "IMG_HOSTED"]
        cpp.includePaths: ["src", "."]

        Export {
            Depends { name: "cpp" }
            cpp.defines: ["IMG_API_VERSION=1", "IMG_HOSTED"]
            cpp.includePaths: ["src", "."]
        }

        Group {
            name: "src"
            fileTags: ["cpp"]
            files: [
                "src/cmdline.c",
                "src/filename.c",
                "src/img.c",
                "src/message.c",
                "src/osdepend.c",
                "src/useful.c"
            ]
        }


        files: [
            "src/cmdline.h",
            "config.h",
            "src/debug.h",
            "src/filelist.h",
            "src/filename.h",
            "src/img.h",
            "src/message.h",
            "src/osalloc.h",
            "src/osdepend.h",
            "src/ostypes.h",
            "src/useful.h",
            "src/whichos.h"
        ]

    }

}
