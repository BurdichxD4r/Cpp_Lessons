#include "t_matrix.h"

using namespace std;

int LeftSide(vector<uint> a){
    for(int i = 0; i < int(a.size()); i++){
        if(a[i] == 1)
            return i;
    }
    return 128;
}

int RightSide(vector<uint> b){
    for(int i = int(b.size()) - 1; i >= 0; i--){
        if(b[i] == 1)
            return i;
    }
    return -1;
}

Matrix::Matrix(uint p_size)
{
    this->size = p_size;

    field = vector<vector<uint>>(size);

    for(int i = 0; i < size; i++){
        field[i] = vector<uint>(size);
    }

    block = vector<uint>(size);
}

bool Matrix::add_row(vector<uint> row, uint block_value)
{
    if(h_fill >= size)
        return false;

    if(int(row.size()) > size)
        block[h_fill] = row[size];
    else
        block[h_fill] = block_value;

    field[h_fill] = row;
    if(linear_independence(h_fill))
        h_fill++;
    return true;
}

bool Matrix::add_row(BitString row, uint block_value)
{
    if(h_fill >= size)
        return false;

    if(int(row.size()) >= size)
        block[h_fill] = row.test(size);
    else
        block[h_fill] = block_value;

    for(int i = 0; i < size; i++)
        field[h_fill][i] = row.test(size - 1 - i);

    if(linear_independence(h_fill))
        h_fill++;
    return true;
}

void Matrix::set_column(BitString col, int num)
{
    for(int i = 0; i < size; i++){
        field[i][num] = col.test(i);
    }
}

void Matrix::add_row_to_row(int row1, int row2)
{
    block[row1] ^= block[row2];

    for(int i = 0; i < size; i++){
        field[row1][i] ^= field[row2][i];
    }
}

void Matrix::swap_rows(int row1, int row2)
{
    int sparei = block[row1];
    block[row1] = block[row2];
    block[row2] = sparei;

    vector<uint> spare = field[row1];
    field[row1] = field[row2];
    field[row2] = spare;
}

bool Matrix::linear_independence(int row)
{
    for(int i = 0; i < row; i++){
        int dif = RightSide(field[row]) - RightSide(field[i]);

        if(dif > 0)
            swap_rows(i, row);
        else if(dif == 0)
            add_row_to_row(row, i);
        else
            continue;
    }
    if(field[row] == vector<uint>(size))
        return false;
    return true;
}

bool Matrix::diagonalize()
{
    for(int i = 0; i < h_fill; i++){                        // rearrange rows
        if(RightSide(field[i]) != size - 1 - i)
            for(int j = i; j < h_fill; j++){
                if(RightSide(field[j]) == size - 1 - i){
                    swap_rows(i, j);
                    break;
                }
            }

        for(int j = i + 1; j < h_fill; j++){                // clean the left part
            if(RightSide(field[i]) == RightSide(field[j]))
                add_row_to_row(j, i);
        }
    }
    for(int i = h_fill - 1; i > 0; i--){                    // clean the right part
        for(int j = i-1; j >= 0; j--){
            if(LeftSide(field[j]) == LeftSide(field[i])){
                add_row_to_row(j, i);
            }
        }
    }

    return h_fill >= 128? true : false;
}

BitString Matrix::compile_solution()
{
    BitString res;

    for(int i = 0; i < size; i++)
        res.setbit(size - i - 1, block[i]);  //Я убил на поиск этой ошибки приблизительно полтора дня. fuck

    return res;
}

void Matrix::display()
{
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            cout << field[i][j] ;
        }
        cout << "| " << block[i] << endl;
    }
}

vector<uint> &Matrix::operator[](int num)
{
    return field[num];
}











