import time
from argparse import ArgumentParser

from autonomysim.utils import SetupPath
from python.autonomysim.ai.vision.benchmarks import ImageBenchmarker, cameraTypeMap


def main(args):
    SetupPath()

    image_benchmarker = ImageBenchmarker(
        img_benchmark_type=args.img_benchmark_type,
        viz_image_cv2=args.viz_image_cv2,
        save_images=args.save_images,
        img_type=args.img_type,
    )

    image_benchmarker.start_img_benchmark_thread()
    time.sleep(args.time)
    image_benchmarker.stop_img_benchmark_thread()


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument(
        "--img_benchmark_type",
        type=str,
        choices=["simGetImage", "simGetImages"],
        default="simGetImages",
    )
    parser.add_argument(
        "--enable_viz_image_cv2",
        dest="viz_image_cv2",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "--save_images", dest="save_images", action="store_true", default=False
    )
    parser.add_argument(
        "--img_type", type=str, choices=cameraTypeMap.keys(), default="scene"
    )
    parser.add_argument(
        "--time", help="Time in secs to run the benchmark for", type=int, default=30
    )

    args = parser.parse_args()
    main(args)
