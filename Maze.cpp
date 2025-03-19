#include <iostream>
#include <vector>
#include <queue>
#include <bits/stdc++.h>

void Search (vector<vector<int>>& Maze, int r, int c, int O_state, int N_state)
{
  //error check vector location and states
  if (r < 0 || // Ensures the row in the array is not negative (outside of maze)
      r > Maze.size() || // Ensures the row in the array is not larger than the size of the array (outside of maze)
      c < 0 || // Ensures the column in the array is not negative
      c >= Maze[0].size() || // Ensures the column is not smaller than than the array size
      Maze[r][c] != O_state) // Ensures non recursive dfs operations (changing values not relevant to the path)
      {

      }
  }


 auto main() -> int {

  // Declating global variables
 vector<vector<int>> Maze;

   auto I_image = 1;
   auto N_image = 1;

 };


