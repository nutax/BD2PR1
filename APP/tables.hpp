#include "Types.hpp"
struct cliente{
INumber id;
Text<20> nombre;
Text<13> telf;
Text<30> pais;
INumber get_key(void)const{return id;}
};
std::ostream& operator<<(std::ostream& os, const cliente& data){
os<<data.id<<"\t"<<data.nombre<<"\t"<<data.telf<<"\t"<<data.pais<<"\t";
return os;
}
EH<cliente,INumber,1000>cliente_ ("cliente");
struct pastel{
INumber id;
Text<20> nombre;
INumber get_key(void)const{return id;}
};
std::ostream& operator<<(std::ostream& os, const pastel& data){
os<<data.id<<"\t"<<data.nombre<<"\t";
return os;
}
SF<pastel,INumber,1000>pastel_ ("pastel");
