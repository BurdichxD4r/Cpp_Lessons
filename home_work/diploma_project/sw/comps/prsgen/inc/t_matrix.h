#ifndef MATRIX_H
#define MATRIX_H

#include "t_bitstring.h"

typedef unsigned int uint ;

class Matrix
{
public:
    Matrix(){}
    Matrix(uint p_size);

public:

    int size = 0;
    int w_fill = 0;
    int h_fill = 0;

    std::vector<std::vector<uint>> field;
    std::vector<uint> block;

public:
    bool add_row(std::vector<uint> row, uint block_value = 0); //and triangulate
    bool add_row(BitString row, uint block_value = 0);    //and triangulate

    void set_column(BitString col, int num);

    void add_row_to_row(int row1, int row2);
    void swap_rows(int row1, int row2);

    bool linear_independence(int row);
    bool diagonalize();

    BitString compile_solution();

    void display();

    std::vector<uint> &operator[] (int num);
};

#endif // MATRIX_H
