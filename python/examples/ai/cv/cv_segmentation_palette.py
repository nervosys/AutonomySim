from autonomysim.utils import generate_color_palette


if __name__ == "__main__":
    numPixelsWide = 4
    outputFile = "seg_color_palette.png"
    generate_color_palette(numPixelsWide, outputFile)
