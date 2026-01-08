#!/bin/bash

# --- Configuration ---
# Path to your MPI executable
EXECUTABLE="./mpi-implementation/bin/main"

# Directory to save results
BASE_RESULT_DIR="results"

# Simulation parameters
# Sizes in full integer form: 1M, 2M, 4M, 8M, 16M
SIZES=(1000000 2000000 4000000 8000000 16000000)

# CPU counts (adjust as needed for your cluster)
CPUS=(1 2 4 8 16 32 64)

# Number of iterations per configuration
ITERATIONS=10

# --- Main Logic ---

# Create base results directory if it doesn't exist
mkdir -p "$BASE_RESULT_DIR"

echo "Starting Benchmark Run..."
echo "--------------------------------"

for c in "${CPUS[@]}"; do
    for s in "${SIZES[@]}"; do
        
        # Define and create the specific sub-directory
        CURRENT_DIR="${BASE_RESULT_DIR}/cpus_${c}_size_${s}"
        mkdir -p "$CURRENT_DIR"
        
        echo "Processing: $c CPUs | Data Size: $s"

        for ((i=1; i<=ITERATIONS; i++)); do
            
            # 1. Create a temporary PBS submission script
            # We name it 'job.sh' so the output file becomes 'job.sh.o{ID}'
            cat <<EOF > job.sh
#!/bin/bash
#PBS -N benchmark_${c}_${s}
#PBS -l nodes=1:ppn=${c}
#PBS -l walltime=00:05:00
#PBS -j oe

# Move to the directory where the script was submitted
cd \$PBS_O_WORKDIR

# Run the MPI program
# Note: We pass the CPU count to -n and the size as an argument
mpirun -n ${c} ${EXECUTABLE} ${s}
EOF

            # 2. Submit the job and capture the Job ID
            # qsub returns the Job ID (e.g., 12345.cluster.local)
            FULL_JOB_ID=$(qsub job.sh)
            
            # Extract just the numeric part of the ID (e.g., 12345) for tracking
            JOB_NUM=$(echo "$FULL_JOB_ID" | grep -o '^[0-9]*')

            echo "  > Iteration $i/$ITERATIONS submitted. Job ID: $JOB_NUM"

            # 3. Wait loop: Check if the job is still running
            # We use qstat to check the status. If qstat returns exit code 0, the job exists.
            while qstat "$JOB_NUM" > /dev/null 2>&1; do
                sleep 5
            done

            # 4. Wait for the filesystem to sync (Crucial step)
            # Sometimes the .o file takes a second to appear after the job vanishes from qstat
            sleep 3

            # 5. Move the output file to the sub-directory
            # PBS typically names output: script_name.sh.o{JobNumber}
            OUTPUT_FILE="job.sh.o${JOB_NUM}"
            
            if [ -f "$OUTPUT_FILE" ]; then
                mv "$OUTPUT_FILE" "${CURRENT_DIR}/run_${i}_${OUTPUT_FILE}"
                echo "    Finished. Output saved to $CURRENT_DIR"
            else
                echo "    Warning: Output file $OUTPUT_FILE not found!"
            fi

        done
    done
done

# Clean up the temporary submission script
rm -f job.sh

echo "--------------------------------"
echo "All benchmarks completed."
