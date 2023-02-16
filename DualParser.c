#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MaxLine 60

typedef enum {
    FALSE = 0,
    TRUE = 1
}boolean;

boolean isMinProblem(char *filename);
boolean isMaxProblem(char *filename);
void ReadPrimalProblem(char *filename);
void ReadDimensions(char *filename, int *n, int *m);
boolean IsGrammaticallyCorrect(char *filename);
boolean HasAllTheArithmeticOperatorsCorrect(char *filename, int n, int m);
boolean HasAllTheRelationalOperatorsCorrect(char *filename, int n, int m);
boolean HasAllTheRightHandValuesCorrect(char *filename, int n, int m);
void Calculate_C_Coefficients(char *filename, int n, int c[1][n]);
void Calculate_A_Coefficients(char *filename, int n, int m, int A[m][n]);
void Calculate_B_RightHandValues(char *filename, int m, int b[m][1]);
void Calculate_Eqin_Operators(char *filename, int m, int Eqin[m][1]);
void Calculate_MinMax(char *filename, int MinMax[1][1]);
void CalculatePrimalTablesAndNewConstraints(char *filename, int n, int m);
void CalculateAndPrintDual(char *filename, int n, int m, int A[m][n], int b[m][1], int c[1][n], int Eqin[m][1], int MinMax[1][1], int converted_constraints);


int main() {

    char filename[10], buffer[MaxLine];
    FILE *infile;
    int n, m;

    printf("This program gets as input from a file a \nlinear program and converts it to its dual form \n");

    //Reading the primal problem from the input file.
    ReadPrimalProblem(filename);
    //Reading primal's problem dimensions (e.g n = 3 and m = 3..Primal Problem dimensions = 3x3)
    ReadDimensions(filename,&n,&m);

    //Checking if there is a mistake in the syntax of the words "min"/"max" or "s.t"/"subject.to"
    if(!IsGrammaticallyCorrect(filename))
    {
        printf("There is no 'max' or 'min' word in the objective function\nor the words 's.t.', 's.t', 'subject to' are wrongly written\n");
        return;
    }
    //Checking if any of the relational operators(<,>,=) is missing..
    else if(!HasAllTheRelationalOperatorsCorrect(filename,n,m))
    {
        printf("There is a mistake in the linear problem. Maybe a '=' is missing from the linear constraints\n");
        return;
    }
    //Checking if any of the arithmetic operators (+,-) is missing..
    else if(!HasAllTheArithmeticOperatorsCorrect(filename,n,m))
    {
        printf("There is a mistake in the linear problem. Maybe one of symbols '+', '-' are missing\n");
        return;
    }
    //Checking if any of the values after the symbols >,<,= is missing.
    else if(!HasAllTheRightHandValuesCorrect(filename,n,m))
    {
        printf("There is a mistake in the linear problem. Maybe on of the values after the operators '<=', '>=', or '=' is missing\n");
        return;
    }
    else
    {
        //Calculating all the coefficients of the primal problem and puts them to matrices.
        CalculatePrimalTablesAndNewConstraints(filename,n,m);
    }

}

/*This function is used to read the primal problem which
  is inserted from a file*/
void ReadPrimalProblem(char *filename)
{
    char buffer[MaxLine];
    FILE *infile;

    //Checking if the input name of the file is correct.
    while(TRUE)
    {
        printf("Insert file name: ");
        scanf("%s", filename);
        infile = fopen(filename, "r" );
        if(infile != NULL)
            break;
        printf("%s does not exist. \n", filename);
    }

    printf("The linear problem that you inserted is: \n");
    while(fgets(buffer,MaxLine,infile) != NULL)
    {
        printf("%s", buffer);
    }
    printf("\n");
    fclose(infile);
}

/*This function is used to read the dimensions of the primal
  problem. For example if we have the problem
  max z =    22x1  -  34x2 - 3x3
  s.t.        1x1  -   1x2 +  x3  <= 10
              2x1  -    x2 -  x3  <= 2
              2x1  -   2x2 + 3x3  <= 6
               xj >= 0 (j=1,2,3,4)

  then it is a 3x3 primal problem. */
void ReadDimensions(char *filename, int *n, int *m)
{
    char buffer[MaxLine];
    int i;
    FILE *infile;

    infile = fopen(filename,"r");

    //Calculating m to find the number of linear constraints.
    *m = 0;
    while(fgets(buffer,MaxLine,infile) != NULL)
    {
        *m+=1;
    }
    //Abstracting 2 from m to remove the objective function and non-negative restrictions lines.
    *m = *m - 2;
    fclose(infile);
    infile = fopen(filename, "r");
    fgets(buffer,MaxLine,infile);
    //Calculating n to find the number of decision variables.
    *n = 0;
    for(i=0; buffer[i] != '\0'; i++)
    {
        /*Everytime we parse a 'x' letter we increase the n variable by 1 except the situation
          that before 'x' there is an 'a'. That means that the linear problem is maximization problem
          and we encounter the 'x' letter of the word "max" which is false.*/
        if(buffer[i] == 'x')
        {
            if(buffer[i-1] != 'a')
            {
                *n+=1;
            }
        }
    }
    fclose(infile);
}

/*This function checks if the syntax of the words "min/"max" or
  "s.t"/"subject.to" is correct*/
boolean IsGrammaticallyCorrect(char *filename)
{
    int i;
    char* maxFound, minFound, s_t1_, s_t2_, subject_to_;
    char buffer[MaxLine];
    char min[4] = "min";
    char max[4] = "max";
    char s_t1[4] = "s.t", s_t2[5] = "s.t.", subject_to[12] = "subject to";
    FILE *infile;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    minFound = strstr(buffer,min);
    maxFound = strstr(buffer,max);
    fgets(buffer,MaxLine,infile);
    s_t1_ = strstr(buffer,s_t1);
    s_t2_ = strstr(buffer,s_t2);
    subject_to_ = strstr(buffer,subject_to);

    return((minFound || maxFound) & (s_t1_ || s_t2_ || subject_to_));

    fclose(infile);
}

/*This function checks of all the relational operators are correct.
  For example if a (>,<,=) is missing from a constrain,there is an error*/
boolean HasAllTheRelationalOperatorsCorrect(char *filename, int n, int m)
{
    int i, j;
    char buffer[MaxLine];
    boolean correct_constraint;
    FILE *infile;

    infile = fopen(filename, "r");
    fgets(buffer,MaxLine,infile);
    fgets(buffer,MaxLine,infile);

    correct_constraint = FALSE;
    //Checking if the symbol "=" exists for every constraint.
    //The symbol "=" is necessary for every constrain because
    //the operators < and > will be written in the format >= and <=.
    for(j=0; j<m; j++)
    {
        for(i=0; buffer[i] != '\0'; i++)
        {
           if(buffer[i] == '=')
           {
               correct_constraint = TRUE;
           }
        }

        if(!correct_constraint)
        {
            return(FALSE);
        }
        correct_constraint = FALSE;
        fgets(buffer,MaxLine,infile);
    }
    fclose(infile);
    return(TRUE);
}

/*This function checks of there is a mistake in the symbols (+,-)
  inside the linear primal problem.*/
boolean HasAllTheArithmeticOperatorsCorrect(char *filename, int n, int m)
{
    int i, j;
    char buffer[MaxLine];
    int number_of_arithmetic_operators = 0, number_of_decision_variables = 0;
    FILE *infile;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    //For every linear constraint.
    for(j=0; j<m+1; j++)
    {
        number_of_arithmetic_operators = 0;
        number_of_decision_variables = 0;
        for(i=0; buffer[i] != '\0'; i++)
        {
            if(buffer[i] == '=')
            {
                continue;
            }
            if(buffer[i] == 'x')
            {
                number_of_decision_variables++;
            }
            if(buffer[i] == '+' || buffer[i] == '-')
            {
                number_of_arithmetic_operators++;
            }

        }
        //If j==0 (this is the objective function..)
        if(isMaxProblem(filename) && (j == 0))
        {
            //The number of arithmetic operators must be 2 less than the number of "x" letters
            //in the buffer. The "x" letter of the word "max" is also encountered.
            if(number_of_arithmetic_operators != (number_of_decision_variables - 2))
            {
                printf("Missing '+' or '-' in the objective function \n");
                return(FALSE);
            }
        }
        else
        {
            //Unlike the max problem, if the problem is min we don't have to count
            //for an extra "x" letter.
            if(number_of_arithmetic_operators != (number_of_decision_variables - 1))
            {
                printf("Missing '+' or '-' in the objective function or in the linear constraints \n");
                return(FALSE);
            }
        }
        fgets(buffer,MaxLine,infile);
    }
    fclose(infile);
    return(TRUE);
}

/*This function checks if all the coefficients after the relational
  symbols are there. For example if the linear problem is this
  max z =    22x1  -  34x2 - 3x3
  s.t.        1x1  -   1x2 +  x3  <= 10
              2x1  -    x2 -  x3  <=
              2x1  -   2x2 + 3x3  <=  6
               xj >= 0 (j=1,2,3,4)
  the coefficient in the 2nd constraint is missing.*/
boolean HasAllTheRightHandValuesCorrect(char *filename, int n, int m)
{
    int i, j;
    char buffer[MaxLine];
    boolean value_exists;
    FILE *infile;

    infile = fopen(filename, "r");
    fgets(buffer,MaxLine,infile);
    fgets(buffer,MaxLine,infile);

    value_exists = FALSE;
    for(j=0; j<m; j++)
    {
        for(i=strlen(buffer); buffer[i] != '='; i--)
        {
           if(isdigit(buffer[i]))
           {
               value_exists = TRUE;
           }
        }

        if(!value_exists)
        {
            return(FALSE);
        }
        value_exists = FALSE;
        fgets(buffer,MaxLine,infile);
    }
    fclose(infile);
    return(TRUE);
}

/*This function calculates all the coefficients each decision variable
  has in the objective function.Every coefficient calculated is put in
  the c matrix*/
void Calculate_C_Coefficients(char *filename, int n, int c[1][n])
{
    char arithmetic_operators_c[1][n-1]; //All the arithmetic operators the objective function has.
    char buffer[MaxLine];
    char c_coefficient[5]; //A string which holds every coefficient for every decision variable.
    int i, j, k, p;
    FILE *infile;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    /*Calculating all the arithmetic operators ('+' or '-') that coefficients of objective function must have*/
    j = 0;
    //Creating the arithmetic operator table to know which operator ('+' or '-') each decision variable has.
    for(i=0; buffer[i] != '\0'; i++)
    {
        if(buffer[i] == '-')
        {
            arithmetic_operators_c[0][j] = '-';
            j++;
        }
        else if(buffer[i] == '+')
        {
            arithmetic_operators_c[0][j] = '+';
            j++;
        }
    }
    //Calculating the values of c table.
    /*The variable k is used to parse reversely the slots of the buffer before each decision variable
      The variable p is used to insert each coefficient of each decision variable into the c_coefficient matrix.
      The variable j is used to parse all the slots of the c matrix.
      The c_coefficient matrix is used to hold the coefficients of each decision variable, which will be converted
      later into 'int' type values so they can be stored to the c matrix.The c matrix(c[1][n]) stores all the coefficients
      values of all the decision variables. */
    for(k=0; k<5; k++)
    {
        c_coefficient[k] = '\0';
    }

    j = 0;
    k = 2;
    p = 1;
    for(i=0; buffer[i] != '\0'; i++)
    {
        if(buffer[i] == 'x')
        {
            //Checking if its a max problem.
            if(buffer[i-1] == 'a')
            {
                continue;
            }
            /*Checking if there is no coefficient before the decision variable. If this is true
              that means the coefficient of the specific decision variable is 1. */
            else if(buffer[i-1] == ' ')
            {
                c[0][j] = 1;
            }
            else
            {
                c_coefficient[0] = buffer[i-1];
                //While there are more arithmetic values as we parse the buffer reversely.
                while(isdigit(buffer[i-k]))
                {
                    c_coefficient[p] = buffer[i-k];
                    k++;
                    p++;
                }
                p = 1;
                k = 2;

                //Reverse the values of the string each time.
                strrev(c_coefficient);

                //Convert string to integer.
                c[0][j] = atoi(c_coefficient);

                //Clearing and initializing the char matrix.
                for(k=0; k<5; k++)
                {
                   c_coefficient[k] = '\0';
                }
            }
            k=2;
            j++;
         }
    }
    //Inserting the '-' operator in the c table whenever the operator before each decision variable is negative.
    for(i=0; i<n-1; i++)
    {
        if(arithmetic_operators_c[0][i] == '+')
        {
            continue;
        }
        else if(arithmetic_operators_c[0][i] == '-')
        {
            c[0][i+1] = -c[0][i+1];
        }
    }
    fclose(infile);
}

/*This function calculates all the coefficients each decision variable has
  in every linear constraint. All of these coefficients will be stored in the
  multidimensional A matrix. */
void Calculate_A_Coefficients(char *filename, int n, int m, int A[m][n])
{
    int i, j, k, p, h;
    char arithmetic_operators_a[m][n-1];
    char buffer[MaxLine];
    char a_coefficient[5];
    FILE *infile;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    /*Creating the arithmetic_operators_a table in which we will store all the '+' or '-' symbols
      of all the linear constraints */
    j = 0;
    /*Parsing all the linear constraints*/
    for(k=0; k<m; k++)
    {
        fgets(buffer,MaxLine,infile);
        /*Parsing every letter of the buffer till we find the symbol '=', which is the place where we should stop the scanning.*/
        for(i=0; buffer[i] != '='; i++)
        {
            if(buffer[i] == '-')
            {
                arithmetic_operators_a[k][j] = '-';
                j++;
            }
            else if(buffer[i] == '+')
            {
                arithmetic_operators_a[k][j] = '+';
                j++;
            }
        }
        j=0;
    }

    //Refreshing the file.
    rewind(infile);

    //Initializing all the slots of the a_coefficient matrix which will be used to store every coefficient of every decision variable.
    for(k=0; k<5; k++)
    {
        a_coefficient[k] = '\0';
    }

    /*Similar to c table,we take all the coefficients from every decision variable in every linear constraint.
      All of these coefficients will be put in the A matrix. */

    fgets(buffer,MaxLine,infile);
    for(h=0; h<m; h++)
    {
        j = 0;
        k = 2;
        p = 1;
        fgets(buffer,MaxLine,infile);
        for(i=0; buffer[i] != '\0'; i++)
        {
            if(buffer[i] == 'x')
            {
                //Checking if its a max problem.
                if(buffer[i-1] == 'a')
                {
                    continue;
                }
                /*Checking if there is no coefficient before the decision variable. If this is true
                  that means the coefficient of the specific decision variable is 1. */
                else if(buffer[i-1] == ' ')
                {
                    A[h][j] = 1;
                }
                else
                {
                    a_coefficient[0] = buffer[i-1];
                    //While there are more arithmetic values as we parse the buffer reversely.
                    while(isdigit(buffer[i-k]))
                    {
                        a_coefficient[p] = buffer[i-k];
                        k++;
                        p++;
                    }
                    p = 1;
                    k = 2;

                    //Reverse the values of the string each time.
                    strrev(a_coefficient);

                    //Convert string to integer.
                    A[h][j] = atoi(a_coefficient);

                    //Clearing and initializing the char matrix.
                    for(k=0; k<5; k++)
                    {
                       a_coefficient[k] = '\0';
                    }
                }
                k=2;
                j++;
             }
        }
        j = 0;
        k = 2;
        p = 1;
    }

    //Inserting the '-' operator in the a table whenever the operator before each decision variable is negative.
    for(i=0; i<m; i++)
    {
        for(j=0; j<n-1; j++)
        {
            if(arithmetic_operators_a[i][j] == '+')
            {
                continue;
            }
            else if(arithmetic_operators_a[i][j] == '-')
            {
                A[i][j+1] = -A[i][j+1];
            }
        }
    }
    fclose(infile);
}

/*This function calculates all the values that exist after the relational
  operators (<=, >=, =) of every linear constraint. These values are being
  stored in the b matrix.*/
void Calculate_B_RightHandValues(char *filename, int m, int b[m][1])
{
    int i, k, p, h;
    char buffer[MaxLine];
    boolean b_value_is_negative = FALSE;
    char b_coefficient[5];
    FILE *infile;

    //Calculating the b Table.
    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    //Initializing all positions of b_coefficient matrix.
    for(k=0; k<5; k++)
    {
        b_coefficient[k] = '\0';
    }

    for(h=0; h<m; h++)
    {
        p=0;
        fgets(buffer,MaxLine,infile);
        //Parsing the buffer reversely because b values are
        //in the end of the buffer.
        for(i=strlen(buffer); buffer[i] != '='; i--)
        {
            //Continue till parsing a number..
            if(buffer[i] == ' ')
            {
                continue;
            }
            if(isdigit(buffer[i]))
            {
                b_coefficient[p] = buffer[i];
                p++;
            }
            if(buffer[i] == '-')
            {
                b_value_is_negative = TRUE;
            }

        }
        //Checking if the b value is negative.
        if(b_value_is_negative)
        {
            strrev(b_coefficient);
            //Making the b negative.
            b[h][0] = -atoi(b_coefficient);

        }
        else
        {
            strrev(b_coefficient);
            b[h][0] = atoi(b_coefficient);
        }
        //Initializing again the b_coefficient matrix for the next constraint.
        for(k=0; k<5; k++)
        {
            b_coefficient[k] = '\0';
        }
        p=0;
        b_value_is_negative = FALSE;

    }
    fclose(infile);
}

/*This function calculates the relational operators for every constrain
  and stores the value -1, 1 or 0, if the relational operators are <= ,
  >= or = respectively. */
void Calculate_Eqin_Operators(char *filename, int m, int Eqin[m][1])
{
    int i, h;
    char buffer[MaxLine];
    FILE *infile;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    //Creating the Eqin table.
    /*Inserting in each position of Eqin table the values: -1 (if the comparison is less than '<=')
      1 (if the comparison is greater than '>=') and 0 (if the comparison is equal '=').*/
    for(h=0; h<m; h++)
    {
        fgets(buffer,MaxLine,infile);
        for(i=0; buffer[i] != '\0'; i++)
        {
            if(buffer[i] == '=')
            {
                if(buffer[i-1] == '<')
                {
                    Eqin[h][0] = -1;
                }
                else if(buffer[i-1] == '>')
                {
                    Eqin[h][0] = 1;
                }
                else
                {
                    Eqin[h][0] = 0;
                }
            }
        }
    }
    fclose(infile);
}

/*This function calculates if the primal problem
  is a maximization or minimization one. */
void Calculate_MinMax(char *filename, int MinMax[1][1])
{
    if(isMaxProblem(filename))
    {
        MinMax[0][0] = 1;
    }
    else
    {
        MinMax[0][0] = -1;
    }
}


/*This function calculates all the converted constraints which will be used
  to calculate the dual problem*/
void CalculatePrimalTablesAndNewConstraints(char *filename, int n, int m)
{
    int A[m][n], b[m][1], c[1][n], Eqin[m][1], MinMax[1][1];
    char arithmetic_operators_c[1][n-1];
    char arithmetic_operators_a[m][n-1];
    int i, j, k, p, l, h;
    char buffer[MaxLine];
    boolean b_value_is_negative = FALSE;
    char c_value[5], a_value[5], b_value[5];
    int converted_constraints;
    FILE *infile;

    Calculate_C_Coefficients(filename,n,c);
    Calculate_A_Coefficients(filename,n,m,A);
    Calculate_B_RightHandValues(filename,m,b);
    Calculate_Eqin_Operators(filename,m,Eqin);
    Calculate_MinMax(filename,MinMax);

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    //Converted_constraints value is being used to identify the number of constraints that the matrix A will have after the convertion.
    converted_constraints = 0;

    for(i=0; i<m; i++)
    {
        //We will need to add constraints to the dual problem
        //equal to the number of primal constraints which have
        //the "=" symbol as relational operator.
        if(Eqin[i][0] == 0)
        {
            converted_constraints++;
        }
    }
    //Converting the primal linear problem to dual.
    CalculateAndPrintDual(filename,n,m,A,b,c,Eqin,MinMax,converted_constraints);
}

/*This function calculates and prints the dual problem.
  It stores the new converted coefficients to dual matrices
  Dual A, Dual c Dual b, Dual Eqin, Dual MinMax. */
void CalculateAndPrintDual(char *filename, int n, int m, int A[m][n], int b[m][1], int c[1][n], int Eqin[m][1], int MinMax[1][1], int converted_constraints)
{
    char buffer[MaxLine];
    int i,j, k;
    int Dual_A[n][m+converted_constraints], Dual_b[n][1], Dual_c[1][m+converted_constraints], Dual_Eqin[n][1], Dual_MinMax[1][1]; //The dual problem matrices.
    //Converted matrices are being used to modify the primal problem where the linear constraints
    //contain only the symbol "=" as relational operator.
    int Converted_A[m+converted_constraints][n];
    int Converted_b[m+converted_constraints][1];
    int Converted_Eqin[m+converted_constraints][1];
    FILE *infile, *outfile;

    infile = fopen(filename,"r");
    converted_constraints = converted_constraints + m;
    //Doing the essential changes in the primal tables and inserting the right values to dual tables.
    if(MinMax[0][0] == -1)
    {
        Dual_MinMax[0][0] = 1;
    }
    else
    {
        Dual_MinMax[0][0] = -1;
    }

    //If its a max problem we want all the constraints to be '<=' so we change all the operators of the specific constraint.
    //If its a min problem we do exactly the opposite. We wont '>=' constraints.
    //If one of the constraints has only the symbol '=' we create 2 more constraints, one with the operators of the constraint and one with the exact opposite operators.
    k=0;
    for(i=0; i<m; i++)
    {
        if(Eqin[i][0] == 0)
        {
            for(j=0; j<n; j++)
            {
                 Converted_A[k][j] = A[i][j];
                 Converted_A[k+1][j] = -A[i][j];
            }
            Converted_b[k][0] = b[i][0];
            Converted_b[k+1][0] = -b[i][0];
            if(MinMax[0][0] == - 1)
            {
                Converted_Eqin[k][0] = 1;
                Converted_Eqin[k+1][0] = 1;
            }
            else
            {
                Converted_Eqin[k][0] = -1;
                Converted_Eqin[k+1][0] = -1;
            }
            k+=2;
        }
        else if((MinMax[0][0] == -1 && Eqin[i][0] == -1) || (MinMax[0][0] == 1 && Eqin[i][0] == 1))
        {
            for(j=0; j<n; j++)
            {
                Converted_A[k][j] = -A[i][j];
            }
            Converted_b[k][0] = -b[i][0];
            if(MinMax[0][0] == -1)
            {
                Converted_Eqin[k][0] = 1;
            }
            else
            {
                Converted_Eqin[k][0] = -1;
            }
            k++;
        }
        else
        {
            for(j=0; j<n; j++)
            {
                Converted_A[k][j] = A[i][j];
            }
            Converted_b[k][0] = b[i][0];
            Converted_Eqin[k][0] = Eqin[i][0];
            k++;
        }
    }

    //Creating the dual c matrix.
    for(i=0; i<converted_constraints; i++)
    {
        Dual_c[0][i] = Converted_b[i][0];
    }

    //Creating the dual b matrix.

    for(i=0; i<n; i++)
    {
        Dual_b[i][0] = c[0][i];
    }

    //Creating the dual A matrix.
    for(i=0; i<n; i++)
    {
        for(j=0; j<converted_constraints; j++)
        {
            Dual_A[i][j] = Converted_A[j][i];
        }
    }

    for(i=0; i<n; i++)
    {
        Dual_Eqin[i][0] = -Converted_Eqin[i][0];
    }

    outfile = fopen("DualProblem","w");

    if(Dual_MinMax[0][0] == -1)
    {
        fprintf(outfile,"min z =");
    }
    else
    {
        fprintf(outfile,"max z =");
    }

    //Factorization and printing.
    k=0;
    for(i=0; i<m; i++)
    {
        if(abs(Dual_c[0][i]) == abs(Dual_c[0][i+1]) && (Eqin[i][0] == 0))
        {
            if((Dual_c[0][k] > 0 && i>0))
            {
                fprintf(outfile," +%dw%d ", Dual_c[0][k], i+1);
                k+=2;
            }
            else
            {
                fprintf(outfile,"  %dw%d ", Dual_c[0][k], i+1);
                k+=2;
            }
        }
        else
        {
            if((Dual_c[0][k] > 0 && i>0))
            {
                fprintf(outfile," +%dw%d ", Dual_c[0][k], i+1);
                k++;
            }
            else
            {
                fprintf(outfile,"  %dw%d ", Dual_c[0][k], i+1);
                k++;
            }
        }
    }
    fprintf(outfile,"\ns.t.");

    k=0;
    for(i=0; i<n; i++)
    {
        for(j=0; j<m; j++)
        {
            if(abs(Dual_A[i][j]) == abs(Dual_A[i][j+1]) && j>m)
            {
                if(Dual_A[i][k] > 0 && j>0)
                {
                    fprintf(outfile,"  +%dw%d ", Dual_A[i][k], j+1);
                    k+=2;
                }
                else
                {
                    fprintf(outfile,"   %dw%d ", Dual_A[i][k], j+1);
                    k+=2;
                }

            }
            else
            {
                if(Dual_A[i][k] > 0 && j>0)
                {
                    fprintf(outfile,"  +%dw%d ", Dual_A[i][k], j+1);
                    k++;
                }
                else
                {
                    fprintf(outfile,"   %dw%d ", Dual_A[i][k], j+1);
                    k++;
                }
            }
        }
        if(Dual_Eqin[i][0] == -1)
        {
            fprintf(outfile," <= ");
        }
        else if(Dual_Eqin[i][0] == 1)
        {
            fprintf(outfile," >= ");
        }
        else
        {
            fprintf(outfile," = ");
        }
        fprintf(outfile,"%d", Dual_b[i][0]);
        fprintf(outfile,"\n    ");
        k=0;
    }

    fprintf(outfile,"       wj >= 0 (j=1");

    for(i=1; i<m; i++)
    {
        fprintf(outfile,",%d", i+1);
    }
    fprintf(outfile,")\n");
    printf("\nThe Dual Problem has been printed in the file: DualProblem.txt\n");
    fclose(infile);
    fclose(outfile);

}

/*This function is used to clarify if the linear problem
  needs minimization */
boolean isMinProblem(char *filename)
{
    FILE *infile;
    char buffer[MaxLine], min[4] = "min";
    char* isMin;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    isMin = strstr(buffer,min);

    if(isMin)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
    fclose(infile);
}
/*This function is used to clarify if the linear problem
  needs maximization */
boolean isMaxProblem(char *filename)
{
    FILE *infile;
    char buffer[MaxLine], max[4] = "max";
    char* isMax;

    infile = fopen(filename,"r");
    fgets(buffer,MaxLine,infile);
    isMax = strstr(buffer,max);

    if(isMax)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
    fclose(infile);
}


