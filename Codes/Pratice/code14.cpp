

#include <iostream>

using namespace std;


//用动态规划切绳子
int cutRoles(int n,int m){
  //参数检查...
  
  int f[n+1][m+1];
  
  for(int i=1;i<=n;i++){
    //一行一行填充
    int end = min(i,m);
    for(int j=1;j<=end;j++){
        if(j==1)
          f[i][j] = i;
        else if(i == j)
          f[i][j] = 1;
        else {
          int maxValue = -1;
          //f(i,k)*f(n-i,j-k)
          for(int k=1; k<=j/2; k++){
            for(int x = 1; x<=j && k<= i+k-j; ++x)
              if(f[x][k]*f[i-x][j-k] > maxValue)
                maxValue = f[x][k]*f[i-x][j-k];
          f[i][j] = maxValue;
         }
       }
    }
  }


  for(int i=1;i<=n;i++){
    for(int j=1;j<=m;j++)
      cout << f[i][j] << ' ';
    cout << endl;
  }
  return  0;
}


int main(){
  cutRoles(8,4);
  return 0;
}
