#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <received_trace.txt> <reference_trace.txt> [tolerance]\n", argv[0]);
        return 1;
    }

    const char *recv_path = argv[1];
    const char *ref_path = argv[2];
    double tol = (argc > 3) ? atof(argv[3]) : 1e-4;

    FILE *f_recv = fopen(recv_path, "r");
    if (!f_recv) {
        fprintf(stderr, "Error: cannot open received trace file: %s\n", recv_path);
        return 2;
    }

    FILE *f_ref = fopen(ref_path, "r");
    if (!f_ref) {
        fprintf(stderr, "Error: cannot open reference trace file: %s\n", ref_path);
        fclose(f_recv);
        return 2;
    }

    int count = 0;
    int mismatches = 0;
    double v_recv, v_ref;

    while (fscanf(f_recv, "%lf", &v_recv) == 1 && fscanf(f_ref, "%lf", &v_ref) == 1) {
        count++;
        double diff = fabs(v_recv - v_ref);
        if (diff > tol) {
            fprintf(stderr, "Mismatch at sample %d: received %f, expected %f (diff: %f > tol: %f)\n",
                    count, v_recv, v_ref, diff, tol);
            mismatches++;
        }
    }

    fclose(f_recv);
    fclose(f_ref);

    if (count == 0) {
        fprintf(stderr, "Error: no data points verified\n");
        return 3;
    }

    if (mismatches > 0) {
        fprintf(stderr, "FAIL: %d of %d samples mismatched\n", mismatches, count);
        return 4;
    }

    printf("PASS: Successfully verified %d samples against reference trace (tol = %e)\n", count, tol);
    return 0;
}
