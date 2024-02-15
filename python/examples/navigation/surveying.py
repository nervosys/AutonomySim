import sys
import argparse

from autonomysim.utils import SetupPath
from autonomysim.navigation.surveying import SurveyNavigator


def main():
    SetupPath()

    args = sys.argv
    args.pop(0)
    arg_parser = argparse.ArgumentParser("Usage: survey boxsize stripewidth altitude")
    arg_parser.add_argument(
        "--size", type=float, help="size of the box to survey", default=50
    )
    arg_parser.add_argument(
        "--stripewidth",
        type=float,
        help="stripe width of survey (in meters)",
        default=10,
    )
    arg_parser.add_argument(
        "--altitude",
        type=float,
        help="altitude of survey (in positive meters)",
        default=30,
    )
    arg_parser.add_argument(
        "--speed", type=float, help="speed of survey (in meters/second)", default=5
    )
    args = arg_parser.parse_args(args)
    nav = SurveyNavigator(args)
    nav.start()


if __name__ == "__main__":
    main()
