# Copyright (c) 2020 Reese Danzer

# This software is released under the MIT License.
# https://opensource.org/licenses/MIT


from inquirer import list_input, text, confirm
from os import listdir, path
from subprocess import run, CalledProcessError
import stitch


rawPaths = []
mapPaths = []
outPath = ''
errstr = ('\nERROR:\nIn the time since this prompt was loaded,' +
          'you appear to have died. Or the file was moved. ' +
          'The seed folder should now be updated, look for a ' +
          '[DEAD] tag by the seed name.\n')


def getPathLists():
    global rawPaths
    global mapPaths
    rawPaths = listdir('.\\raws')
    if 'desktop.ini' in rawPaths:
        rawPaths.remove('desktop.ini')
    if 'README.md' in rawPaths:
        rawPaths.remove('README.md')
    rawPaths = ['.\\raws\\' + i for i in rawPaths]
    rawPaths.sort(key=path.getctime, reverse=True)
    rawPaths = [i.lstrip('.\\raws\\') for i in rawPaths]

    mapPaths = listdir('.\\maps')
    if 'desktop.ini' in mapPaths:
        mapPaths.remove('desktop.ini')
    if 'README.md' in mapPaths:
        mapPaths.remove('README.md')
    mapPaths = ['.\\maps\\' + i for i in mapPaths]
    mapPaths.sort(key=path.getctime, reverse=True)
    mapPaths = [i.lstrip('.\\maps\\') for i in mapPaths]


def stcWrap(pth):
    try:
        return stitch.stitch2(pth)
    except FileNotFoundError:
        print(errstr)
        print(f'Bad Path: {pth}\n')
        return 0


def aStcWrap(pth):
    """Advanced mode is pretty trash
    """
    print('\nAdvanced Stitch\n---------------')
    print('Leaving fields blank will result in defaults being used')
    print('WARNING: The entry fields are finicky, and capture ALL input. ' +
          'If you try to use backspace, the value will be rejected.')
    print(f'Drawing from {pth}\n')

    cF = text('Desired Crop Factor (num 0.01 - 1, def 0.75)')
    if not cF:
        cF = 0.75
    try:
        cF = float(cF)
    except ValueError:
        print('Value entered is not a number, using default')
        cF = 0.75
    if cF > 1 or cF < 0.01:
        print('Value entered is outside acceptable range, using default')
        cF = 0.75
    print(f'Crop Factor is {cF}\n')

    fF = text('Desired Feather Radius (num 0.01 - 1, def 0.15)')
    if not fF:
        fF = 0.15
    try:
        fF = float(fF)
    except ValueError:
        print('Value entered is not a number, using default')
        fF = 0.15
    if fF > 1 or fF < 0.01:
        print('Value entered is outside acceptable range, using default')
        fF = 0.15
    print(f'Feather Radius is {fF}\n')

    icon = confirm('Mark player position?', default=True)
    print(f'Mark Player = {icon}\n')

    try:
        return stitch.stitch2(pth,
                              '',
                              cF,
                              fF,
                              icon)
    except FileNotFoundError:
        print(errstr)
        print(f'Bad Path: {pth}\n')
        return 0


def accWrap(pth):
    try:
        run(['start', pth], shell=True, check=True)
        return 1
    except CalledProcessError:
        print(errstr)
        print(f'Bad Path: {pth}\n')
        return 0


def parametrized(dec):
    """src:
    stackoverflow.com/questions/5929107/decorators-with-parameters
    """
    def layer(*args, **kwargs):
        def repl(f):
            return dec(f, *args, **kwargs)
        return repl
    return layer


@parametrized
def extopt(func, mode):
    def wrapper(*args):
        mapOpList = ['Open different map', 'Draw different map']
        menuOpList = ['Main Menu', 'Exit']
        bList = mapOpList + menuOpList

        if mode == 'map':
            args += (mapOpList,)
        elif mode == 'menu':
            args += (menuOpList,)
        elif mode == 'both':
            args += (bList,)

        choice = func(*args)

        if choice == 'Open different map':
            omap()
        elif choice == 'Draw different map':
            draw()
        if choice == 'Main Menu':
            menu2()
        elif choice == 'Exit':
            return

    return wrapper


@extopt('both')
def now(pth, exOps):
    global outPath
    print('\r')
    getPathLists()

    if pth == 0:
        menu2()
        return

    nw = list_input(
        'What next?',
        choices=['Redraw this map',
                 'Reopen this map',
                 'Redraw this map in Advanced Mode'
                 ]+exOps
                    )

    if nw == 'Redraw this map':
        outPath = stcWrap(pth)
        if not outPath:
            draw()
        else:
            naxt(pth)
    elif nw == 'Reopen this map':
        if not accWrap(outPath):
            omap()
        else:
            now(pth)
    elif nw == 'Redraw this map in Advanced Mode':
        outPath = aStcWrap(pth)
        if not outPath:
            draw()
        else:
            naxt(pth)
    else:
        return nw


@extopt('both')
def naxt(pth, exOps):
    global outPath
    getPathLists()
    print('\r')

    if pth == 0:
        menu2()
        return

    nxt = list_input(
        'What next?',
        choices=['Open this map',
                 'Redraw this map',
                 'Redraw this map in Advanced Mode'
                 ]+exOps
                     )

    if nxt == 'Open this map':
        if not accWrap(outPath):
            omap()
        else:
            now(pth)
    elif nxt == 'Redraw this map':
        outPath = stcWrap(pth)
        if not outPath:
            draw()
        else:
            naxt(pth)
    elif nxt == 'Redraw this map in Advanced Mode':
        outPath = aStcWrap(pth)
        if not outPath:
            draw()
        else:
            naxt(pth)
    else:
        return nxt


@extopt('menu')
def omap(mOps):
    getPathLists()

    targ = list_input(
        'Select which seed you want to view',
        choices=mapPaths+mOps)
    if targ == 'Exit' or targ == 'Main Menu':
        return targ
    else:
        omap2(targ)


@extopt('menu')
def omap2(seed, mOps):
    global outPath
    maps = []
    try:
        maps = listdir('.\\maps\\' + seed)
    except FileNotFoundError:
        print(errstr)
        omap()
    if maps == []:
        return
    if 'desktop.ini' in maps:
        maps.remove('desktop.ini')
    maps.sort(reverse=True)

    mtarg = list_input(
        'Select which version you want to view',
        choices=maps+['Redraw Map']+mOps)
    if mtarg == 'Exit' or mtarg == 'Main Menu':
        return mtarg
    elif mtarg == 'Redraw Map (simple mode only)':
        outPath = stcWrap('.\\raws\\' + seed)
        if not outPath:
            draw()
        else:
            naxt('.\\raws\\' + seed)
    elif not accWrap('.\\maps\\' + seed + '\\' + mtarg):
        omap()
    else:
        now('.\\raws\\' + seed)


@extopt('menu')
def draw(mOps):
    global outPath
    getPathLists()

    targ = list_input(
        'Select which seed you want to map',
        choices=rawPaths+mOps)
    if targ == 'Exit' or targ == 'Main Menu':
        return targ
    pth = '.\\raws\\' + targ
    adv = confirm('Use Advanced Mode?', default=False)
    if adv:
        outPath = aStcWrap(pth)
        if not outPath:
            draw()
        else:
            naxt(pth)
    else:
        outPath = stcWrap(pth)
        if not outPath:
            draw()
        else:
            naxt(pth)


def menu2():
    getPathLists()

    whatdo = list_input(
            'MAIN',
            choices=['Draw a map', 'Open a map', 'Exit'])
    if whatdo == 'Exit':
        return
    elif whatdo == 'Draw a map':
        draw()
    elif whatdo == 'Open a map':
        omap()


def menu():
    getPathLists()

    whatdo = list_input(
            'What would you like to do?',
            choices=['Draw a map', 'Open a map', 'Exit'])
    if whatdo == 'Exit':
        return
    elif whatdo == 'Draw a map':
        draw()
    elif whatdo == 'Open a map':
        omap()


def main():
    print('------------------------------\n',
          'ADVANCED CARTOGRAPHY: STITCHER\n',
          '------------------------------',
          sep='')
    print('Use ARROW KEYS to make selection, ENTER to confirm')
    print('Newest seeds/maps are always first\n\n')

    menu()


if __name__ == '__main__':
    main()
