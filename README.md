# DualParser

### Explanation
A Project that parses a primal linear problem written in a text file and converts it to its dual form.

Features:
  * Reads a primal problem from the inputed file.
  * Checks for any syntax mistakes that might exist.
  * Stores its coefficients to matrices.
       
         c matrix is used to store the coefficients of objective function. 
         A matrix is used to store the coefficients of linear constraints. 
         b matrix is used to store the values of linear constraints after the symbols (<=,>=,=) 
         Eqin matrix is used to store the relational operators of linear constraints. 
             -1 is used for less than (<=) 
              1 is used for greater than (>=) 
         MinMax is used to store the value -1 if its a minimization problem or 1 otherwise. 
        
       
         For example if the primal problem is:
         max z =    22x1  -  34x2 - 3x3
         s.t.       1x1   -   1x2 +  x3  <= 10
                    2x1   -    x2 -  x3  <= 2
                    2x1   -   2x2 + 3x3  <= 6
                        xj >= 0 (j=1,2,3,4)
       
         then the primal matrices will be:
       
         c = [22 -34 -3]
       
         A = [1 -1 1,
              2 -1 -1,
              2 -2 3]
       
         b = [10 2 6]
       
         Eqin = [-1 -1 -1]
       
         MinMax = [1]
       
  * Uses these matrices to calculate the dual problem.
  * Prints the dual problem in a text file called DualProblem.
