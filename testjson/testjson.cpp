#include "json.hpp"
using json=nlohmann::json;
#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;
/*json序列化*/
//json 示例1：普通类型
string func1(){
    json js;
    //普通的key-value
    js["msg_type"]=2;
    js["msg"]="hello,what are you doing?";
    js["chat"]="shuai ge";

    string sendBuf=js.dump();/*json字符串转字符串*/
    //cout<<sendBuf.c_str()<<endl;/*string转char *在网络中发送 */
    return sendBuf;
}

//json 示例2：复杂类型
string func2(){
    json js;
    //数组
    js["id"]={1,2,3,4,5};
    //对象
    js["msg"]["zhang san"]="hello";
    js["msg"]["li si"]="world";
    //对象
    js["ege"]={{"muji",1},{"gongji",2}};
    //cout<<js<<endl;
    string sendBuf=js.dump();
    return sendBuf;
}

//json 示例3：直接将容器序列化json
void func3(){
    json js;
    vector<int> vec={1998,12,233};
    js["num"]=vec;
    /*用对象存的map*/
    map<string,int> mp={
        {"huangshan",1},
        {"songshan",2},
        {"taishan",3},
    };
    js["mountain"]=mp;
    /*用数组存的map*/
    map<int,string> test;
    test.insert({1,"chat"});
    test.insert({2,"web"});
    test.insert({3,"server"});
    js["name"]=test;
    cout<<js<<endl;
    /*{"mountain":{"huangshan":1,"songshan":2,"taishan":3},"name":[[1,"chat"],[2,"web"],[3,"server"]],"num":[1998,12,233]}*/
}
/*反序列化*/
int main(){
    string recvBuf=func2();
    json buf=json::parse(recvBuf);
    map<string,int> a=buf["ege"];
    for (auto &p:a){
        cout<<p.first<<" "<<p.second<<endl;
    }

    return 0;
}