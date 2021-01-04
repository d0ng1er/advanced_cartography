# Copyright (c) 2020 Reese Danzer

# This software is released under the MIT License.
# https://opensource.org/licenses/MIT


from os import listdir, path
from time import sleep, strftime
from PIL import Image, ImageDraw
from threading import Thread
from math import floor


class pic:
    def __init__(self, fpath, cropFactor, featherFactor):
        self.name = path.basename(fpath)
        self.fpath = fpath
        self.im = Image.open(fpath)
        self.size = self.im.size
        self.coords = self.name.lstrip('0123456789_')
        self.coords = self.coords.lstrip('(')
        self.coords = self.coords.rstrip(').jpg')
        self.coords = self.coords.split(',')
        self.coords = [float(i) for i in self.coords]
        self.mtime = path.getmtime(fpath)
        self.featherFactor = featherFactor
        self.cTuple = (round((self.size[0]-self.size[0]*cropFactor)/2),  # LEFT
                       round((self.size[1]-self.size[1]*cropFactor)/2),  # TOP
                       round((self.size[0]+self.size[0]*cropFactor)/2),  # RITE
                       round((self.size[1]+self.size[1]*cropFactor)/2))  # BOTM
        self.cSize = (self.cTuple[2]-self.cTuple[0],
                      self.cTuple[3]-self.cTuple[1])
        self.cim = self.im.crop(self.cTuple)

    def getFMask(self):
        """ Returns an edge feather mask to be used in paste function.
        """
        if self.featherFactor < 0.01:
            mask = Image.new('L', self.cSize, color=255)
            return mask
        mask = Image.new('L', self.cSize, color=255)
        draw = ImageDraw.Draw(mask)
        x0, y0 = 0, 0
        x1, y1 = self.cSize
        # print(f'Crop Tuple: {(x1, y1)}')
        feather = round(self.cSize[1] * self.featherFactor)
        # print(f'Feather Pixels: {feather}')
        for i in range(round(self.cSize[1]/2)):
            x1, y1 = x1-1, y1-1
            alpha = 255 if i > feather else round(255*(feather/(i+1))**(-1))
            draw.rectangle([x0, y0, x1, y1], outline=alpha)
            x0, y0 = x0+1, y0+1
        return mask

    def closePIL(self):
        """closes the im and cim PIL objects
        """
        self.im.close()
        self.cim.close()

    def getSkull(self):
        """adds a semi-transparent 8-bit image (skull) to cropped
        PIL image. For use on last pic in list when fPath is marked
        with [DEAD]
        """
        skull = Image.open('.\\files\\gfx\\skull.png')
        skull = skull.resize(self.cSize)
        self.cim.paste(skull, mask=skull)

    def getYAH(self):
        """adds a semi-transparent 8-bit image ("you are here" marker)
        to cropped PIL image. For use on last pic in list when fPath
        is NOT marked with [DEAD]
        """
        yah = Image.open('.\\files\\gfx\\yah.png')
        yah = yah.resize(self.cSize)
        self.cim.paste(yah, mask=yah)


def stitch2(rawPath,
            destPath='',
            cropFactor=0.75,
            featherFactor=0.15,
            marks=True):
    piclist = []
    batches = listdir(rawPath)
    cnt = 0
    tooBig = False
    dstPath = destPath
    if dstPath == '':
        dstPath = rawPath.replace('raws', 'maps', 1)
    fullPath = dstPath + '\\' + '[MAP]_' + strftime("%Y%m%d-%H%M%S") + '.png'
    print(f'Getting images from {rawPath}')

    for i in batches:
        names = listdir(rawPath + '\\' + i)
        paths = [rawPath + '\\' + i + '\\' + j for j in names]

        for k in range(len(names)):
            cnt += 1
            print(f'Images found: {cnt}', end='\r')
            piclist.append(pic(paths[k], cropFactor, featherFactor))
    piclist.sort(key=lambda i: i.mtime)

    if rawPath.find('DEAD') != -1 and marks:
        piclist[-1].getSkull()
    elif marks:
        piclist[-1].getYAH()

    # This next section calculates the bounds of the final map. It
    # may run into trouble in the future with image overwrites, as
    # currently I'm not doing anything to prevent them.
    xCoordMax = max(i.coords[0] for i in piclist)
    xCoordMin = min(i.coords[0] for i in piclist)
    yCoordMax = max(i.coords[1] for i in piclist)
    yCoordMin = min(i.coords[1] for i in piclist)

    xMaxPad = round(next(
        ((i.size[0]/2) for i in piclist if i.coords[0] == xCoordMax)
                    ))
    xMinPad = round(next(
        ((i.size[0]/2) for i in piclist if i.coords[0] == xCoordMin)
                    ))
    yMaxPad = round(next(
        ((i.size[1]/2) for i in piclist if i.coords[1] == yCoordMax)
                    ))
    yMinPad = round(next(
        ((i.size[1]/2) for i in piclist if i.coords[1] == yCoordMin)
                    ))

    mapWidth = round(xCoordMax - xCoordMin) + xMaxPad + xMinPad
    mapHeight = round(yCoordMax - yCoordMin) + yMaxPad + yMinPad
    print(f'\nYou have explored an area {mapWidth} pixels wide ' +
          f'and {mapHeight} pixels deep.')
    sleep(1)
    if mapWidth > 65535 or mapHeight > 65535:
        tooBig = True
        ratio = 65535 / max(mapWidth, mapHeight)
        mapWidth = floor(mapWidth*ratio-10)  # subtract 10 just to be sure
        mapHeight = floor(mapHeight*ratio-10)
        print(f"That's too many, downscaling by {round(ratio*100, 3)}%")
        print('(Limit is 65535px on either axis)')
        sleep(1)
        print(f'New output dimensions: {mapWidth} x {mapHeight}')
        sleep(1)

    print('Building canvas')
    bigMap = Image.new('RGB', (mapWidth, mapHeight), color=0)
    if tooBig:
        for i in piclist:
            print(f'Applying image {piclist.index(i)+1} of {len(piclist)}',
                  end='\r')
            nWidth = round(i.cSize[0]*ratio)
            nHeight = round(i.cSize[1]*ratio)
            resized = i.cim.resize((nWidth, nHeight))
            resizedMask = i.getFMask().resize((nWidth, nHeight))
            targ = (round((i.coords[0]-xCoordMin) * ratio),
                    round((i.coords[1]-yCoordMin) * ratio))
            bigMap.paste(resized, targ, resizedMask)
            i.closePIL()
    else:
        for i in piclist:
            print(f'Applying image {piclist.index(i)+1} of {len(piclist)}',
                  end='\r')
            targ = (round(i.coords[0] - xCoordMin),
                    round(i.coords[1] - yCoordMin))
            bigMap.paste(i.cim, targ, i.getFMask())
            i.closePIL()

    # Main thread to save map
    saver = Thread(
            target=bigMap.save,
            args=(fullPath,),
            kwargs={'subsampling': 0, 'quality': 100})

    def fEllipsis():
        """makes the ellipsis move while file is being saved, so user
        doesn't get bored.
        """
        print('\n', end='\r')
        while saver.is_alive():
            print('Drawing map   ', end='\r')
            sleep(0.3)
            print('Drawing map.  ', end='\r')
            sleep(0.3)
            print('Drawing map.. ', end='\r')
            sleep(0.3)
            print('Drawing map...', end='\r')
            sleep(0.3)

    ellipsis = Thread(target=fEllipsis)
    saver.start()
    ellipsis.start()
    saver.join()
    ellipsis.join()
    print('\nMap Complete')
    print(f'Path: {fullPath}')
    return(fullPath)
