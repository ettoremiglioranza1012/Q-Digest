# Dataset generator

The python script contained in this directory is meant to be run from the CLI and will produce
a set of random integers that can be fed as input to the Q-Digest to test how it handles different
problem sizes and perform an unbiased benchmarking procedure against different implementations.

## Setup

```bash
# create a virtual environment
python -m venv .venv
# activate the created environment (Linux/MacOS)
source .venv/bin/activate
# install dependencies
pip install -r requirements.txt
```

## Example usage

The following commands assume that the current working directory is set to this directory (i.e., the one
where the script is located.)

```bash
# generate a dataset containing 1000000 integers, set the random seed to 101 (replicability of results)
# and save the results to the `datasets` directory (default value)
python ./generate_dataset.py --seed 101 --n 1000000 --save

# it is also possible to sample from specific, known probability distributions by specifying the
# --dist argument. The example below samples from a Gaussian distribution.
python ./generate_dataset.py --seed 101 --n 1000000 --save --dist gaussian

# it is also possible to compute the quantiles of the distribution and print those
# the example below computes the 10th, 20th, 70th, and 90th quantiles along with
# the median (0.5) and the maximum of the distribution (1.0)
python ./generate_dataset.py --seed 101 --n 1000000 --quantiles 0.1 0.2 0.5 0.7 0.9 1.0

# to know all the details and supported distributions open the help menu by running:
python ./generate_dataset.py --help
```
