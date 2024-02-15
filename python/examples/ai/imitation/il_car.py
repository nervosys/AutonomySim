from autonomysim.ai.imitation.agents import CarAgent
from autonomysim.ai.imitation.preprocessors import preprocess_data
from autonomysim.ai.imitation.trainers import train_drive_model


# point this to the directory containing the raw data
RAW_DATA_DIR = "./raw_data/"

# point this to the desired output directory for the cooked (.h5) data
COOKED_DATA_DIR = "./cooked_data/"

# choose The folders to search for data under RAW_DATA_DIR
COOK_ALL_DATA = True

# Trained model path
MODEL_PATH = "./models/example_model.h5"

# << The directory in which the model output will be placed >>
MODEL_OUTPUT_DIR = "./models/"


def main():
    # preprocess data
    preprocess_data(
        input_dir=RAW_DATA_DIR, output_dir=COOKED_DATA_DIR, all=COOK_ALL_DATA
    )

    # train model
    # NOTE: `DriveDataGenerator` performs data augmentation in `train_drive_model`
    train_drive_model(input_dir=COOKED_DATA_DIR, output_dir=MODEL_OUTPUT_DIR)

    # load and run the car agent model
    agent = CarAgent()
    agent.load(model_path=MODEL_PATH)
    agent.run()


if __name__ == "__main__":
    main()
