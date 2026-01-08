#!/bin/bash

# --- Configuration ---
EXECUTABLE="./mpi-implementation/bin/main"
BASE_RESULT_DIR="results"
DATA_DIR="./src/dataset-generator/datasets/"

# Data sizes in full integer form
SIZES=(1000000 2000000 4000000 8000000 16000000)

# CPU counts
CPUS=(1 2 4 8 16 32 64)

ITERATIONS=10

# --- Main Logic ---

mkdir -p "$BASE_RESULT_DIR"

echo "Starting Benchmark Run..."
echo "--------------------------------"

for c in "${CPUS[@]}"; do
    for s in "${SIZES[@]}"; do
        
        INPUT_FILE="${DATA_DIR}size${s}.txt"

        # Safety Check
        if [ ! -f "$INPUT_FILE" ]; then
            echo "Error: Input file '$INPUT_FILE' not found! Skipping..."
            continue
        fi

        CURRENT_DIR="${BASE_RESULT_DIR}/cpus_${c}_size_${s}"
        mkdir -p "$CURRENT_DIR"
        
        # --- FIX 1: Define the Job Name variable here so we can reuse it ---
        JOB_NAME="benchmark_${c}_${s}"

        echo "Processing: $c CPUs | Input File: $INPUT_FILE"

        for ((i=1; i<=ITERATIONS; i++)); do
            
            cat <<EOF > job.sh
#!/bin/bash
#PBS -N ${JOB_NAME}
#PBS -l select=1:ncpus=${c}:mem=1gb -l place=pack:excl
#PBS -l walltime=00:01:30
#PBS -q short_HPC4DS
cd \$PBS_O_WORKDIR

module load mpich-3.2
# Run MPI with the input file name
mpirun.actual -n ${c} ${EXECUTABLE} ${INPUT_FILE}
EOF

            # Submit and grab ID
            FULL_JOB_ID=$(qsub job.sh)
            JOB_NUM=$(echo "$FULL_JOB_ID" | grep -o '^[0-9]*')

            echo "  > Iteration $i/$ITERATIONS submitted. Job ID: $JOB_NUM"

            # Wait for job completion
            while qstat "$JOB_NUM" > /dev/null 2>&1; do
                sleep 5
            done

            # Wait for filesystem sync
            sleep 3

            # --- FIX 2: Look for the file named after the Job Name ---
            OUTPUT_FILE="${JOB_NAME}.o${JOB_NUM}"
            
            if [ -f "$OUTPUT_FILE" ]; then
                mv "$OUTPUT_FILE" "${CURRENT_DIR}/run_${i}_${OUTPUT_FILE}"
                echo "    Finished."
            else
                echo "    Warning: Output file $OUTPUT_FILE missing."
            fi

        done
    done
done

rm -f job.sh

echo "--------------------------------"
echo "All benchmarks completed."
