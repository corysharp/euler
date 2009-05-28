import java.util.ArrayDeque;

public abstract class DancingLinks {

    protected static class Cell {
        Cell down;
        Cell up;
        Cell right;
        Cell left;
        Cell column;
        int size;
        String name;

        public Cell() {
            down = this;
            up = this;
            right = this;
            left = this;
            column = this;
        }

        public void addLeft( Cell c ) {
            c.left = left;
            c.right = this;
            left.right = c;
            left = c;
        }

        public void addRight( Cell c ) {
            c.right = right;
            c.left = this;
            right.left = c;
            right = c;
        }

        public void addUp( Cell c ) {
            c.up = up;
            c.down = this;
            up.down = c;
            up = c;
        }

        public void addDown( Cell c ) {
            c.down = down;
            c.up = this;
            down.up = c;
            down = c;
        }


        /** "this" must be an available cell to become a new row header for column;
         * to add a new row, call this first. */
        public void startNewRow( Cell column ) {
            this.left = this;
            this.right = this;
            this.column = column;
            column.addUp( this );
            column.size++;
        }

        /** "this" must be a row header, tailcolumn is the column, and tail the
         * new cell; to add more columns to a row, call this second. */
        public void appendToRow( Cell tailcolumn, Cell tail ) {
            tail.startNewRow( tailcolumn );
            append( tail );
        }

        public void append( Cell c ) {
            addLeft(c);
        }

        public String toString() {
            return name;
        }
    }

    protected Cell root;
    protected ArrayDeque<Cell> visit;
    protected int max_solutions;
    protected int num_solutions;

    abstract void foundSolution();

    protected DancingLinks() {
        root = new Cell();
        visit = new ArrayDeque<Cell>();
        max_solutions = Integer.MAX_VALUE;
    }

    void preset( Cell r ) {
        visit.add(r);
        cover(r.column);
        for (Cell j=r.right; j!=r; j=j.right)
            cover(j.column);
    }

    void clear() {
        Cell r;
        while ((r=visit.pollLast()) != null) {
            for (DancingLinks.Cell j=r.left; j!=r; j=j.left)
                uncover(j.column);
            uncover(r.column);
        }
        num_solutions = 0;
    }

    void cover( Cell c ) {
        c.right.left = c.left;
        c.left.right = c.right;
        for (Cell i=c.down; i!=c; i=i.down) {
            for (Cell j=i.right; j!=i; j=j.right) {
                j.down.up = j.up;
                j.up.down = j.down;
                j.column.size--;
            }
        }
    }

    void uncover( Cell c ) {
        for (Cell i=c.up; i!=c; i=i.up) {
            for (Cell j=i.left; j!=i; j=j.left) {
                j.down.up = j;
                j.up.down = j;
                j.column.size++;
            }
        }
        c.right.left = c;
        c.left.right = c;
    }

    void search() {
        Cell c = root.right;
        if (c == root) {
            num_solutions++;
            foundSolution();
        }
        else {
            for (Cell j=c.right; j!=root; j=j.right) {
                if (j.size < c.size)
                    c = j;
            }
            if (c.size > 0) {
                cover(c);
                for (Cell r=c.down; r!=c; r=r.down) {
                    visit.addLast(r);
                    for (Cell j=r.right; j!=r; j=j.right) {
                        cover(j.column);
                    }
                    search();
                    for (Cell j=r.left; j!=r; j=j.left) {
                        uncover(j.column);
                    }
                    visit.removeLast();

                    if (num_solutions >= max_solutions)
                        break;
                }
                uncover(c);
            }
        }
    }
}

