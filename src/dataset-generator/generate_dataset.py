# NOTE: keep the sample size to less than 1000000000 to avoid the program from getting killed by the OS

import argparse
import numpy as np
from datetime import datetime
import os
import logging
from pathlib import Path
from typing import List

# initialize logger
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# instantiate the argument parser
parser = argparse.ArgumentParser()

# determine which distributions should be allowed
valid_dists = {'gaussian', 'exponential', 'poisson', 'geometric'}
# declare optional arguments
parser.add_argument("--seed", 
                    help = "the seed to be used for the random data generation.",
                    type = int)
parser.add_argument("--dist", 
                    help = f"the probability distribution to be used for the random data generation.\nOne of {valid_dists}.",
                    type = str,
                    choices = valid_dists,
                    metavar='dist') # prevents overwriting of choices over name when calling --help
parser.add_argument("--n", 
                    help = "the size of the dataset generated.", type = int)
parser.add_argument("--save", 
                    action='store_true', # treats this argument as a flag setting it to true
                    help = "use this argument if to write results to a file.")
parser.add_argument("--quantiles",
                    help = "the quantiles to compute. A set of floats between 0, and 1 (inclusive)",
                    nargs='+',
                    type=float)

args = parser.parse_args()
# handle cases when the arguments are not set
size: int = args.n if args.n else 1000
# logger.info("Setting size to:", size)
seed = 121 if not args.seed else args.seed
# logger.info("Setting seed to:", seed)
dist = None if not args.dist else args.dist
# logger.info("Setting distribution to:", dist)
# default to quartiles if no quantile is passed
quantiles: List[float] = [0.0, 0.25, 0.5, 0.75, 1.0] if not args.quantiles else args.quantiles
result = np.array([])
# instantiate the generator object
generator = np.random.default_rng(seed=seed)

max_uint32 = np.iinfo(np.uint32).max
# case when dist is not set from CLI
if not args.dist:
    logger.info(f"Generating {size} random integers from uniform distribution")
    result = generator.integers(low=0, high=max_uint32, size=size, dtype=np.uint32)
# case when dist is set
else:
    # generate based on chosen distribution, no need to check for default case
    # (already checked by argparse)
    match args.dist:
        case 'gaussian':
            mean = 10000
            sd = 1000
            logger.info(f"Generating {size} random integers from normal distribution with:\nmean={mean}\nsd={sd}")
            result = generator.normal(mean, sd, size=size).astype(np.uint32)
            result = np.where(result < 0, 0, result) # filters the dataset to exclude negative-valued integers
        case 'exponential':
            logger.info(f"Generating {size} random integers from exponential distribution")
            result = generator.exponential(size=size).astype(np.uint32)
        case 'poisson':
            logger.info(f"Generating {size} random integers from poisson distribution")
            result = generator.poisson(size=size).astype(np.uint32)
        case 'geometric':
            logger.info(f"Generating {size} random integers from geometric distribution")
            result = generator.geometric(p=0.1, size=size).astype(np.uint32)

# save to file + error handling in case of missing directory
if args.save:
    file_path = Path(__file__)
    dir_path = file_path.parent
    dest_dir_name = "datasets"
    dataset_name = f"{datetime.now()}+size{size}.txt".replace(' ', '_').replace(':', '.')
    dest_dir_path = dir_path.joinpath(dest_dir_name)
    if not dest_dir_path.exists():
        logger.warning(f"Directory {dest_dir_path} does not exist, creating it now...")
        os.makedirs(dest_dir_path)
        logger.info(f"Directory {dest_dir_path} created successfully.")
    full_path = dest_dir_path.joinpath(dataset_name)
    np.savetxt(full_path, result.reshape(1,-1), delimiter=',', fmt='%d') # need to reshape into array with 1 row and size cols
else:
    logger.info(f"Generated numbers: {result}")

distribution_quantiles = np.quantile(result, quantiles).astype(np.uint32)
for i in range(len(quantiles)):
    logger.info(f"{quantiles[i]*100}th quantile: {distribution_quantiles[i]}") 

# logger.debug(f"Check min of distribution: {result.min()}")
# logger.debug(f"Check max of distribution: {result.max()}")
