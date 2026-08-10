# Third-Party Notices

## Qt

HN Reader links dynamically against the Qt libraries supplied by reMarkable OS.

The optional AppLoad build compiles the unmodified `linuxfb` platform plugin sources from the
Qt 6.8.2 `qtbase` release. Those sources and the resulting plugin remain subject to Qt's licensing
terms, including the GNU Lesser General Public License version 3 where applicable. The build script
retrieves the source directly from the official [Qt repository](https://github.com/qt/qtbase) and
does not vendor it in this repository.

See [Qt licensing](https://www.qt.io/licensing/) and the license files in the downloaded Qt source
tree for the complete terms.

## Hacker News API

Story and comment data comes from the public
[Hacker News API](https://github.com/HackerNews/API). Hacker News and Y Combinator are not
affiliated with this project.

## Xovi and AppLoad

The optional device integration interoperates with
[Xovi](https://github.com/asivery/xovi) and [AppLoad](https://github.com/asivery/rm-appload).
They are separate projects distributed under their own licenses and are not included here.
