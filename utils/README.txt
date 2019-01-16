# ParAd: Parallel Adriana 1.1.2 - utilities:

## gen_clans - generate a (sparse) matrix with ideal clan structure

    >gen_clans k mc type d r 

 k - number of clans;
 mc - number of equations in a clan;
 type - output matrix format ("spm" - sparse or "mat" - conventional);
 d - density (0.0 - 1.0);
 r - ration for the number of clan's variables.

## spm_bas - sort spm basis and print a row in one line

    >spm_bas spm_matrix_file basis_file

basis_file contains lines of basis in form "index*value" sorted by indexes.

After spm_bas we sort each of two basises using "sort" to compare them using "diff".

Example: compare two basises b1 and b2

>spm_bas b1 | sort > b1.bas
>spm_bas b2 | sort > b2.bas
>diff b1.sb b2.bas

## toclans - decompose a sparse matrix with REAL elements into clans

    >toclans sparse_matrix_file_name [debug_level]

processes .mtx matrices in MatrixMarket format skipping header (lines beginning from %);
debug_levels greater than 0 give more detailed output and save files with clans.

## transpose - transposes a sparse matrix (.spm)

    >transpose matrixAfile matrixBfile

B=A^T

## mul2m - multiply sparse matrices (.spm) using GraphBLAS

    >grb_mul2a matrAfile matrBfile matrCfile

C=A*B

# 
