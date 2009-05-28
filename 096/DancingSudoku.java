import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayDeque;
import java.util.Arrays;

public class DancingSudoku extends DancingLinks {

    private boolean print_solution = false;

    protected class Cell extends DancingLinks.Cell {
        byte row;
        byte col;
        byte box;
        byte val;

        Cell( int col, String name ) {
            this.col = (byte)col;
            this.name = name;
        }

        Cell( int row, int col, int box, int val, String pre ) {
            this.row = (byte)row;
            this.col = (byte)col;
            this.box = (byte)box;
            this.val = (byte)val;
            //this.name = pre+":r"+row+",c"+col+",b"+box+",v"+val;
        }

        Cell( Cell cell, String pre ) {
            this( cell.row, cell.col, cell.box, cell.val, pre );
        }
    }

    Cell[] cells = new Cell[9*9*9];
    int[] solution = new int[81];
    int nsolved = 0;
    int sum = 0;

    public DancingSudoku() {

        max_solutions = 1;
        visit = new ArrayDeque<DancingLinks.Cell>(81);

        Cell[] cols = new Cell[4*9*9];
        String[] colpre = { "rc", "rv", "cv", "bv" };

        for (int i=0; i<cols.length; i++) {
            Cell c = new Cell( i, colpre[(i/81)]+":r"+((i%81)/9)+",c"+(i%9) );
            cols[i] = c;
            root.append( c );
        }

        for (int row=0; row<9; row++) {
            for (int col=0; col<9; col++) {
                for (int val=0; val<9; val++) {
                    int box = 3*(row/3) + (col/3);
                    Cell r = new Cell( row, col, box, val+1, colpre[0] );
                    cells[81*row+9*col+val] = r;
                    r.startNewRow( cols[81*0+9*row+col] );
                    r.appendToRow( cols[81*1+9*row+val], new Cell( r, colpre[1] ) );
                    r.appendToRow( cols[81*2+9*col+val], new Cell( r, colpre[2] ) );
                    r.appendToRow( cols[81*3+9*box+val], new Cell( r, colpre[3] ) );
                }
            }
        }
    }

    void preset( String puzzle ) {
        int n = 0;
        for (char c : puzzle.toCharArray()) {
            if (c != '0')
                preset( cells[9*n+c-'1'] );
            n++;
        }
    }

    public void setPrintSolution( boolean b ) {
        print_solution = b;
    }

    public void solve( String puzzle ) {
        preset(puzzle);
        search();
        clear();
    }

    public void foundSolution() {
        nsolved++;

        if (true) {
            for (DancingLinks.Cell cell : visit) {
                Cell c = (Cell)cell;
                solution[9*c.row+c.col] = c.val;
            }
            sum += solution[0]*100 + solution[1]*10 + solution[2];
        }

        if (print_solution) {
            String s = "";
            for (int i=0; i<9; i++) {
                for (int j=0; j<9; j++) {
                    s += solution[9*i+j];
                    if ((j%3)==2)
                        s += " ";
                }
                s += "\n";
                if ((i%3)==2)
                    s += "\n";
            }
            System.out.println(s);
        }
    }

    public static void main(String[] args) throws Exception {

        boolean print_solution = false;
        String puzzle_filename = null;

        for (int n=0; n<args.length; n++) {
            String opt = args[n];
            if (opt.startsWith("-")) {
                if (opt.equals("-p")) {
                    print_solution = true;
                }
                else {
                    System.err.println("unknown command line option \""+opt+"\"");
                    System.exit(1);
                }
            }
            else if (puzzle_filename == null) {
                puzzle_filename = opt;
            }
            else {
                System.err.println("only one puzzle.txt argument may be specified but \""+opt+"\" given");
                System.exit(1);
            }
        }

        if (puzzle_filename == null) {
            System.out.println("usage: DancingSudoku [-p] [puzzles.txt]");
            System.out.println();
            System.out.println("Options:");
            System.out.println("    -p  print solutions");
            System.out.println();
            System.out.println("puzzles.txt must either be in the format given in at Project Euler or must");
            System.out.println("contain one puzzle per line, ordered row-first.  Zeros indicate an empty");
            System.out.println("square.  For example, this is a valid description of a puzzle:");
            System.out.println();
            System.out.println("003020600900305001001806400008102900700000008006708200002609500800203009005010300");
            System.exit(0);
        }

        BufferedReader in = new BufferedReader( new FileReader( puzzle_filename ) );
        String line;
        int nloaded = 0;

        DancingSudoku sudoku = new DancingSudoku();
        sudoku.setPrintSolution( print_solution );

        long t0 = System.nanoTime();

        while((line=in.readLine()) != null) {
            line = line.trim();
            String puzzle = null;
            if (line.startsWith("Grid")) {
                puzzle = "";
                for (int i=0; i<9; i++)
                    puzzle += in.readLine().trim();
            }
            else if(line.length() == 81) {
                puzzle = line;
            }

            if (puzzle != null) {
                nloaded++;
                sudoku.solve( puzzle );
                if ((nloaded & 1023) == 0)
                    System.out.println("progress: "+nloaded+" puzzles");
            }
        }

        long t1 = System.nanoTime();
        System.out.println( sudoku.nsolved+" out of "+nloaded+" puzzles solved in "
                +((t1-t0)/1e9)+" seconds" );
        System.out.println("sum "+sudoku.sum);
    }
}

