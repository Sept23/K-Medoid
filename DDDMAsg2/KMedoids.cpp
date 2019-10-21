//
//  main.cpp
//  DDDMAsg2
//
//  Created by Liu Qinghao on 2019/5/18.
//  Copyright © 2019 Liu Qinghao. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>
#include <math.h>
#include <iomanip>
#include <cmath>
using namespace std;

class packet{
public:
    string flow_key;
    int arrival_time;
    int packet_length;
};

class cluster{
public:
    int modiodIdx;
    vector<int> nesrestPoints;
    void addPoint(int pointIdx)
    {
        nesrestPoints.push_back(pointIdx);
    }
    void clearPoints()
    {
        nesrestPoints.clear();
    }
    void printPoints()
    {
    
        for (int i=0; i<nesrestPoints.size(); i++) {
            cout<<nesrestPoints[i]<<" ";
        }
        cout<<endl;
    }
    bool isBelong (int pointIdx)
    {
       
        for (int i=0; i<nesrestPoints.size(); i++) {
            if(pointIdx==nesrestPoints[i])
            {
                return true;
            }
        }
        return false;
    }
};

double Distance(vector<vector<double>> flowResult, int point1, int point2)
{
    return abs(flowResult[point1][0]-flowResult[point2][0])+abs(flowResult[point1][1]-flowResult[point2][1]);
}

//int findNearestModoid(int pointIdx,vector<vector<double>> flowResult, vector<int> modoidIdx)
//{
//    int minIdx;
//    double min_dis=9999999.0;
//    double dis;
//    for(int k=0;k<modoidIdx.size();k++)
//    {
//        dis=Distance(flowResult, pointIdx, modoidIdx[k]);
//        if (dis<min_dis)
//        {
//            min_dis=dis;
//            minIdx=modoidIdx[k];
//        }
//    }
//    return minIdx;
//}

int findNearestModoidMap(int pointIdx,vector<vector<double>> map, vector<int> modoidIdx)
{
    int minIdx;
    double min_dis=9999999.0;
    double dis;
    for(int k=0;k<modoidIdx.size();k++)
    {
        dis=map[pointIdx][modoidIdx[k]];
        if (dis<min_dis)
        {
            min_dis=dis;
            minIdx=modoidIdx[k];
        }
    }
    return minIdx;
}

bool isMedoid(vector<int> medoid_idx, int key)
{
    for (int i=0; i<medoid_idx.size(); i++) {
        if(key==medoid_idx[i])
        {
            return true;
        }
    }
    return false;
}

vector<vector<double>>  buildMap(vector<vector<double>> flowResult)
{
    vector<vector<double>> map;
    for (int x=0; x<flowResult.size(); x++) {
        vector<double> entity;
        for (int y=0; y<flowResult.size(); y++) {
            entity.push_back(Distance(flowResult, x, y));
        }
        map.push_back(entity);
    }
    return map;
}

void assignPoints(vector<vector<double>>& flowResult, vector<int>& modoidIdx, vector<cluster>& clusters)
{
    for (int l=0; l<clusters.size(); l++) {
        clusters[l].clearPoints();
        clusters[l].modiodIdx=modoidIdx[l];
    }
    
    for (int i=0; i<flowResult.size(); i++) {
        int minIdx;
        minIdx=findNearestModoidMap(i, flowResult, modoidIdx);
        if (minIdx!=-1)
        {
            for(int h=0;h<clusters.size();h++)
            {
                if(clusters[h].modiodIdx==minIdx)
                {
                    clusters[h].addPoint(i);
                    break;
                }
            }
        }
    }
}

vector<int> findFlow(string key,vector<packet> packets)
{
    vector<int> resultIdx;
    for (int i=0; i<packets.size(); i++) {
        if(packets[i].flow_key==key)
        {
            resultIdx.push_back(i);
        }
    }
    return resultIdx;
}

void removeFlow(vector<int> resultIdx, vector<packet>& packets)
{
    vector <packet>::iterator Iter;
    //remove from end to first
    for (int i=resultIdx.size()-1; i>=0; i--) {
        Iter = packets.begin()+resultIdx[i];
        packets.erase(Iter);
        Iter = packets.begin();
    }
}


double calculateTC(int previousModoidIdx, int potientalModoidIdx, vector<cluster>& clusters,  vector<vector<double>>& map, vector<int>& newModiodIdx,vector<int>& oldModiodIdx)
{
    //i previous medoid; h potiental replacement
    double TCih=0;
    for (int j=0; j<map.size(); j++) {
        bool belongPrevious =false;
        bool belongPotential=false;
        int newClusterIdx;
        int previousCluster;
        double cost=0;
        previousCluster=findNearestModoidMap(j, map, oldModiodIdx);
        if(previousCluster==previousModoidIdx)
        {
            belongPrevious = true;
        }
        newClusterIdx=findNearestModoidMap(j, map, newModiodIdx);
        if(newClusterIdx==potientalModoidIdx)
        {
            belongPotential=true;
        }
       
        if (belongPrevious==true&&belongPotential==true)
        {
            cost=map[j][potientalModoidIdx]-map[j][previousModoidIdx];
        }
        else if (belongPrevious==true&&belongPotential==false)
        {
            cost=map[j] [newClusterIdx]-map[j][previousModoidIdx];
        }
        else if (belongPrevious==false&&belongPotential==true)
        {
            cost=map[j][potientalModoidIdx]-map[j][previousCluster];
        }
        else
        {
            cost=0;
        }
        TCih+=cost;
    }
    return TCih;
}

double calculateError(vector<cluster>& clusters, vector<vector<double>>& map)
{
    double error=0;
    for (int i=0;i<clusters.size();i++)
    {
        for (int j=0; j<clusters[i].nesrestPoints.size(); j++) {
            error=error+map[clusters[i].modiodIdx][ clusters[i].nesrestPoints[j]];
        }
    }
    return error;
}

int main(int argc, const char * argv[]) {
    string network_packets="/Users/liuqinghao/Desktop/assignment2_sample1/network_packets.txt";
    string init_add="/Users/liuqinghao/Desktop/assignment2_sample1/initial_medoids.txt";
//    string network_packets=argv[1];
//    string init_add=argv[2];
    
    
    vector<packet> packets;
    vector<vector<double>> flow_result;
    vector<int> flow_idx;
    int medoid_num;
    vector<int> medoid_idx;
    ifstream infile;
    string data;
    infile.open(network_packets,ios::in);
    getline(infile,data);
    while (getline(infile, data)) {
        string tmp;
        vector<string> vec;
        stringstream ss(data);
        while (ss>>tmp) {
            vec.push_back(tmp);
        }
        packet pac;
        pac.flow_key=vec[0]+vec[1]+vec[2]+vec[3]+vec[4];
        pac.arrival_time=std::stoi(vec[5]);
        pac.packet_length=std::stoi(vec[6]);
        packets.push_back(pac);
    }
    infile.close();
    
    while (packets.size()!=0) {
        
        flow_idx=findFlow(packets[0].flow_key,packets);
        if (flow_idx.size()<=1) {
            removeFlow(flow_idx, packets);
        }
        else
        {
            double time=0;
            double length=0;
            vector<double> result;
            for (int j=flow_idx.size()-1; j>0; j--) {
                length=length+packets[flow_idx[j]].packet_length;
                time=time+packets[flow_idx[j]].arrival_time-packets[flow_idx[j-1]].arrival_time;
            }
            length=length+packets[0].packet_length;
            time=time/(flow_idx.size()-1);
            length=length/flow_idx.size();
            result.push_back(time);
            result.push_back(length);
            flow_result.push_back(result);
            removeFlow(flow_idx, packets);
            
        }
    }
    ofstream fout;
    fout.open ("Flow.txt", ios::out | ios::trunc);
    fout.flags(ios::fixed);
    for(int k=0;k<flow_result.size();k++)
    {
        fout<<k<<" ";
        fout<<setprecision(2)<<flow_result[k][0];
        fout<<" ";
        fout<<setprecision(2)<<flow_result[k][1]<<endl;
    }
    
    fout.close();
    
    ifstream infile1;
    
    infile1.open(init_add,ios::in);
    getline(infile1,data);
    medoid_num=stoi(data);
    while (getline(infile1, data)) {
        int num;
        stringstream ss(data);
        while (ss>>num) {
            medoid_idx.push_back(num);
        }
    }
    infile1.close();

    vector<cluster> clusters;
    for(int i=0; i<medoid_num; i++)
    {
        cluster clust;
        clusters.push_back(clust);
    }
    
    vector<vector<double>> map=buildMap(flow_result);
//    cout<<"done"<<endl;
    bool change=true;
    double error;
//    assignPoints(flow_result, medoid_idx, clusters);
    while(change)
    {
        assignPoints(map, medoid_idx, clusters);
        bool isChange=false;
        for (int m=0; m<flow_result.size(); m++) {
            if (isMedoid(medoid_idx, m)==false) {
                bool isExchange=false;
                for (int g=0; g<medoid_num; g++) {
//                 for (int g=medoid_num; g>0; g--) {
                    //calculate Total
                    vector<int> newModiodIdx(medoid_idx);
                    newModiodIdx[g]=m;
                    double TC;
                    TC=calculateTC(medoid_idx[g], newModiodIdx[g], clusters, map,newModiodIdx,medoid_idx);
                    if (TC<0)
                    {
                        isExchange=true;
                        medoid_idx=newModiodIdx;
                        isChange=true;
//                        assignPoints(flow_result, medoid_idx, clusters);
                        break;
                    }
                }
                if(isExchange)
                {
                    break;
                }
            }
            
        }
        if(isChange==true)
        {
            change=true;
        }
        else
        {
            change=false;
        }
    }
    error=calculateError(clusters, map);
    ofstream fout0;
    fout0.open ("KMedoidsClusters.txt",ios::out | ios::trunc);
    fout0.flags(ios::fixed);
    fout0<<setprecision(2)<<error<<endl;
    for(int i=0;i<medoid_idx.size();i++)
    {
        fout0<<setprecision(2)<<medoid_idx[i]<<" ";
    }
    fout0<<endl;
    for (int i=0; i<clusters.size(); i++) {
        for (int j=0; j<clusters[i].nesrestPoints.size(); j++) {
            fout0<<setprecision(2)<<clusters[i].nesrestPoints[j]<<" ";
        }
        fout0<<endl;
    }
    
    fout0.close();
    
    return 0;
}
