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

        files: [
            "src/cavern.c",
            "src/filename.c",
            "src/filename.h",
            "src/commands.c",
            "src/commands.h",
            "src/netartic.h",
            "src/netartic.c",
            "src/netbits.c",
            "src/netbits.h",
            "src/listpos.h",
            "src/netskel.c",
            "src/netskel.h",
            "src/network.c",
            "src/network.h",
            "src/listpos.c",
            "src/message.c",
            "src/message.h",
            "src/matrix.c",
            "src/matrix.h",
            "src/datain.c",
            "src/datain.h",
            "src/str.c",
            "src/str.h",
            "src/readval.h",
            "src/readval.c",
            "src/namecmp.c",
            "src/namecmp.h",
            "src/date.c",
            "src/date.h",
            "src/img_hosted.c",
            "src/img_hosted.h",
            "src/hash.h",
            "src/hash.c",
            "src/osdepend.c",
            "src/osdepend.h",
            "src/cmdline.c",
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

        files: [
            "src/cmdline.c",
            "src/cmdline.h",
            "config.h",
            "src/debug.h",
            "src/filelist.h",
            "src/filename.c",
            "src/filename.h",
            "src/img.c",
            "src/img.h",
            "src/message.c",
            "src/message.h",
            "src/osalloc.h",
            "src/osdepend.c",
            "src/osdepend.h",
            "src/ostypes.h",
            "src/useful.c",
            "src/useful.h",
            "src/whichos.h"
        ]

    }

}
