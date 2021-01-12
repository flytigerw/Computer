#include <unistd.h>
#include "student.h"
#include "personal.h"
#include "database.h"

template<class T>
Database<T>::Database(){

}

template<class T>
void Database<T>::add(T& d){
  database.open(fname,ios::in|ios::out|ios::binary);
  database.seekp(0,ios::end);
  d.writeToFile(database);
  database.close();
}

template<class T>
void Database<T>::modify(const T& d){
  T tmp;
  database.open(fname,ios::in|ios::out|ios::binary);
  while(!database.eof()){
    tmp.readFromFile(database);
    if(tmp == d){
      cin >> tmp;
      database.seekp(-d.size(),ios::cur);
      tmp.writeToFile(database);
      database.close();
      return;
    }
  }
  database.close();
  cout << "The record to be modifed is not in the database\n";
}

template<class T>
bool Database<T>::find(const T& d){
  T tmp;
  database.open(fname,ios::in|ios::binary);
  while(!(database.eof())){
    tmp.readFromFile(database);
    if(tmp == d){
      database.close();
      return true;
    }
  }
  database.close();
  return false;
}

template<class T>
ostream& Database<T>::print(ostream& out){
  T tmp;
  database.open(fname,ios::in|ios::out|ios::binary);
  while(true){
    tmp.readFromFile(database);
    if(database.eof())
      break;
    sleep(1);
    out << tmp << endl;
  }
  database.close();
  return out;
}

template<class T>
void Database<T>::run(){
  cout << "File name: ";
  cin >> fname;
  char option[5];
  T rec;
  cout <<"1. Add 2.Find 3.Modify a record 4.Exit\n";
  cout << "Enter an optin:";
  cin.getline(option,4);  //get "\n"
  while(cin.getline(option,4)){
    switch(*option){
      case '1':
        cin>>rec;
        add(rec);
        break;
      case '2':
        rec.readKey();
        cout << "The record is";
        if(find(rec) == false)
          cout << "not";
        cout <<"in the database\n";
        break;
      case '3':
        rec.readKey();
        modify(rec);
        case '4':
        return;
        default:
        cout << "Wrong option\n";
    }
    cout << *this;
    cout << "Enter an option";
  }
}

int main(){
  Database<Student> database;
  database.run();
  return 0;
}
