
// TerraLib
#include <terralib/common.h>
#include <terralib/dataaccess.h>
#include <terralib/st.h>
#include <terralib/plugin.h>
#include <terralib/stmemory.h>
#include <terralib/datatype.h>


// STL
#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>

// Boost
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//Rcpp Para Tipos proprios
#include<RcppCommon.h>

//Meu Header
#include "ExemploDeAcesso.h"
using namespace std;
//Definir os membros da classe serão tratados
namespace Rcpp{
template<> std::map<std::string,std::string> as (SEXP dataset);
template<> te::da::DataSourceInfo         as (SEXP datasource);
template<> te::gm::Envelope                 as (SEXP envelope);
template<> te::dt::TimePeriod             as (SEXP timePeriod);
template<> te::st::Trajectory             as (SEXP trajectory);
template<> SEXP wrap            (const te::st::Trajectory &tj);
}

//Rcpp
#include <Rcpp.h>
namespace Rcpp{
template<> std::map<std::string,std::string> as (SEXP dataset){

  Rcpp::List dataSet = dataset;
  std::map<std::string,std::string> ds;

  ds.insert(std::pair<std::string,Rcpp::String>("tableName",dataSet["tableName"]));

  ds.insert(std::pair<std::string,Rcpp::String>("timeName",dataSet["phTimeName"]));
  ds.insert(std::pair<std::string,Rcpp::String>("geomName",dataSet["geomName"]));
  ds.insert(std::pair<std::string,Rcpp::String>("trajId",dataSet["trajId"]));
  ds.insert(std::pair<std::string,Rcpp::String>("trajName",dataSet["trajName"]));

  ds.insert(std::pair<std::string,Rcpp::String>("objId",dataSet["objId"]));

  return ds;
}
template<> te::da::DataSourceInfo as (SEXP datasource){
 // std::cout << std::endl << "começa?";
  Rcpp::List dataSource = datasource;
  te::da::DataSourceInfo ds;
  Rcpp::String typestring = dataSource["type"];

  std::string tstring = typestring;
 // std::cout << std::endl << "Tentando converter  datasource";
  if(tstring == "OGR"){

    std::map<std::string, std::string> connInfo;
    Rcpp::List cInfo = dataSource["connInfo"] ;
    connInfo.insert(std::pair<std::string,Rcpp::String>("URI",cInfo["URI"]));
    ds.setConnInfo(connInfo);
    ds.setType("OGR");

     }
  else if (tstring == "POSTGIS"){
    std::map<std::string, std::string> connInfo;
    Rcpp::List cInfo = dataSource["connInfo"] ;
   // std::cout<< "In ds 3";

    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_CLIENT_ENCODING",cInfo["PG_CLIENT_ENCODING"]));
    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_CONNECT_TIMEOUT",cInfo["PG_CONNECT_TIMEOUT"]));
    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_DB_NAME",cInfo["PG_DB_NAME"]));
    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_HOST",cInfo["PG_HOST"]));
    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_PORT",cInfo["PG_PORT"]));
    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_USER",cInfo["PG_USER"]));
    connInfo.insert(std::pair<std::string,Rcpp::String>("PG_PASSWORD",cInfo["PG_PASSWORD"]));
    //std::cout<< "In ds 4";

    ds.setConnInfo(connInfo);
    ds.setTitle(dataSource["title"]);
    ds.setAccessDriver(dataSource["accessDriver"]);
    ds.setType("POSTGIS");
  }



    return ds;
}
template<> te::gm::Envelope as (SEXP envelope){

  Rcpp::List mbr(envelope);
  Rcpp::List max = mbr["max"];
  Rcpp::List min = mbr["min"];
  double   	llx = min["x"];
  double   	lly = min["y"];
  double   	urx = max["x"];
  double   	ury = max["y"];
  te::gm::Envelope env = te::gm::Envelope(llx,lly,urx,ury);
  return env ;

}
template<> te::dt::TimePeriod as (SEXP timePeriod){
  Rcpp::List tp(timePeriod);
  Rcpp::String comeco = tp["begin"];
  Rcpp::String fim = tp["end"];
  boost::posix_time::ptime begin(boost::posix_time::time_from_string(comeco));
  boost::posix_time::ptime end(boost::posix_time::time_from_string(fim));
  te::dt::TimeInstant b(begin);
  te::dt::TimeInstant e(end);
  return te::dt::TimePeriod(b,e);

}
//ainda nao le obj_id(arrumar isso no futuro proximo)
template<> te::st::Trajectory as (SEXP trajectory){
  std::vector<double> x;
  std::vector<double> y;
  std::vector<std::string> tempo;
  std::vector<int> srid;
  std::vector<std::string> obj_id; //ainda não implementado
  std::vector<std::string> uni_traj_id; //ainda não implementado
  Rcpp::List tp(trajectory);
  x = tp["x"];
  y = tp["y"];
  tempo = tp["time"];
  srid = tp["srid"];
  obj_id = tp["obj_id"];
  te::st::Trajectory traj;
  for(int i=0; i<x.size();i++){
    te::gm::Point pt = te::gm::Point(x[i],y[i],srid[i]);
    boost::posix_time::ptime pti(boost::posix_time::time_from_string(tempo[i]));
    te::dt::TimeInstant ti(pti);
    traj.add(&ti,&pt);
  }

  return traj;

}

template<> SEXP wrap(const te::st::Trajectory &tj){
  std::vector<double> x;
  std::vector<double> y;
  std::vector<std::string> tempo;
  std::vector<int> srid;
  std::vector<std::string> obj_id;
  std::vector<std::string> uni_traj_id;

  Rcpp::List listaDePontos;

  te::st::TrajectoryObservationSet tos = tj.getObservations();
  te::st::TrajectoryObservationSet::const_iterator it = tos.begin();
  while(it != tos.end())
     {

     obj_id.push_back(tj.getObjId());
     uni_traj_id.push_back(tj.getTrajId());

     te::dt::DateTime* t = static_cast<te::dt::DateTime*>(it->first->clone());
     te::gm::Geometry* g = static_cast<te::gm::Geometry*>(it->second->clone());

     ++it;

     if(g->getGeometryType()=="Point"){

       te::gm::Point* coordenadas = (dynamic_cast<te::gm::Point*> (g));

       x.push_back(coordenadas->getX());
       y.push_back(coordenadas->getY());
       srid.push_back(coordenadas->getSRID());

     }
     else{
       return Rcpp::wrap("Not a Point");}
     tempo.push_back(t->toString());

    }


  listaDePontos.push_back(tempo,"time");
  listaDePontos.push_back(x,"x");
  listaDePontos.push_back(y,"y");
  listaDePontos.push_back(srid,"srid");
  listaDePontos.push_back(obj_id,"obj_id");
  listaDePontos.push_back(uni_traj_id,"uni_traj_id");
  return Rcpp::wrap(listaDePontos);
}
}


using namespace Rcpp;

//Conversion from number to string in order to avoid compiler problems
template <typename T>
string NumberToString ( T Number )
{
  ostringstream ss;
  ss << Number;
  return ss.str();
}

//Terralib initializer, terralib must be activated before accessing data
// [[Rcpp::export]]
void initializeTerralib(){

  TerraLib::getInstance().initialize();

  //Load Modules and Plugins
  LoadModules();

  //Initialize STDataLoader support
  te::st::STDataLoader::initialize();
}

/*This method is never used. When used the computer crashes
When the program is closed Terralib seems to close itself
Might be relevant for future work */
// [[Rcpp::export]]
void finalizeTerralib(){
  te::st::STDataLoader::finalize();

  te::plugin::PluginManager::getInstance().unloadAll();

  TerraLib::getInstance().finalize();
}

/*Metodo para buscar como ponteiro externo
trajetorias apartir de um datasource e um dataset */
// [[Rcpp::export]]
SEXP getTrajectoryByTerralibXPtr(SEXP datasource, SEXP dataset){

    try
  {
    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);

    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);

    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);
    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);

    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();
    dataset->moveBeforeFirst();



    Rcpp::XPtr<te::st::TrajectoryDataSet> dsPtr(dataset,true);
    return dsPtr;
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}

/*Metodo para buscar como vectorlist
 trajetorias a partir de um datasource e um dataset */
// [[Rcpp::export]]
SEXP getTrajectoryByTerralib(SEXP datasource, SEXP dataset){

  ////////////////////COMEÇO DO CODIGO
  try
  {

    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);
  //  std::cout << std::endl << "Leu o datasource info";
  std::cout << std::endl << "Linha 285";
    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager2(dsinfo);
    std::cout << std::endl << "Linha 288";
  //  std::cout << std::endl << "Criou o mangaer";
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);
//    std::cout << std::endl << "Leu o dataset info";
//te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);

    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["trajId"], dset["trajName"]);
//    std::cout << std::endl << "Criou o TjInfo";
std::cout << std::endl << "Linha 294";

    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();
    dataset->moveBeforeFirst();
    std::cout << std::endl << "Linha 297";
    ////////Codigo para mandar como Lista
    if(dataset == 0)
    {
      std::cout << "Trajectory Data Set is NULL!" << std::endl;
      return Rcpp::wrap("DataSet Vazio");
    }
    std::cout << std::endl << "Linha 304";
    std::vector<double> x;
    std::vector<double> y;
    std::vector<std::string> tempo;
    std::vector<int> srid;
    Rcpp::List listaDePontos;


    while(dataset->moveNext())
    {
      std::auto_ptr<te::dt::DateTime> time = dataset->getTime();
      std::auto_ptr<te::gm::Geometry> geom = dataset->getGeometry();


      if(geom->getGeometryType()=="Point"){

        te::gm::Point* coordenadas = (dynamic_cast<te::gm::Point*> (geom.get()));

        x.push_back(coordenadas->getX());
        y.push_back(coordenadas->getY());
        srid.push_back(coordenadas->getSRID());

      }

      tempo.push_back(time->toString());

    }

    listaDePontos.push_back(tempo,"time");
    listaDePontos.push_back(x,"x");
    listaDePontos.push_back(y,"y");
    listaDePontos.push_back(srid,"srid");

    return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

 return Rcpp::wrap("Fail");
}

/*Metodo para buscar como vectorlist de trajectories
 trajetorias a partir de um datasource e um dataset
pré determinado*/
// [[Rcpp::export]]
SEXP getTrajectoryByTerralibTraj(){

  ////////////////////COMEÇO DO CODIGO
  try
  {

    //Indicates the data source
    te::da::DataSourceInfo dsinfo;

    std::map<std::string, std::string> connInfo;
    connInfo["PG_HOST"] = "localhost";   // or "localhost";
    connInfo["PG_PORT"] = "5432";
    connInfo["PG_USER"] = "postgres";
    connInfo["PG_PASSWORD"] = "teste2ou3";
    connInfo["PG_DB_NAME"] = "vessel_trajectory";
    connInfo["PG_CONNECT_TIMEOUT"] = "4";
    connInfo["PG_CLIENT_ENCODING"] = "CP1252";     // "LATIN1"; //"WIN1252"

    dsinfo.setConnInfo(connInfo);
    dsinfo.setType("POSTGIS");
    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);




    std::string datasetname = "vessel_trajectories";
    std::string phTimeName = "datahora";
    std::string geomName = "ponto";
    std::string objIdName = "vessel_id";
    std::string tjIdName = "traj_id_unique"; //the object id. In this example, we just have object id and not trajectory id.

    //Indicates the data source
    //te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);

    //It creates a new Data Source and put it into the manager
    //CreateDataSourceAndUpdateManager(dsinfo);

    //std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);
    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, datasetname, phTimeName, geomName, objIdName, "", tjIdName, "");

    //te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["trajId"], dset["trajName"]);
    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);
    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();

    boost::ptr_vector<te::st::Trajectory> trajectories;
    dataset->getTrajectorySet(trajectories);
    ////////Codigo para mandar como Lista

    Rcpp::List listaDePontos;


    for (int i = 0; i < trajectories.size(); ++i)
    {

        std::string count = "trajetoria";
      count += NumberToString(i);
        listaDePontos.push_back(trajectories[i]/*,count*/ );

    }
    return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}

/*Metodo para buscar como vectorlist de trajectories
 trajetorias a partir de um datasource e um dataset
enviado pelo usuario*/
// [[Rcpp::export]]
SEXP getSubTrajectoryByTerralibTraj(SEXP datasource, SEXP dataset){

  ////////////////////COMEÇO DO CODIGO
  try
  {

    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);

     te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);


    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);
    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();

    if(dataset==0){
      printf("Lista vazia");
    }
    boost::ptr_vector<te::st::Trajectory> trajectories;
    dataset->getTrajectorySet(trajectories);
    ////////Codigo para mandar como Lista

    if(trajectories.size()==0){
      printf("Lista vazia");
    }
    Rcpp::List listaDePontos;


    for (int i = 0; i < trajectories.size(); ++i)
    {

      std::string count = "trajetoria";
      count += NumberToString(i);

      listaDePontos.push_back(trajectories[i]/*,count*/);

    }
    return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}

/*Metodo para buscar como ponteiro externo
 trajetorias a partir de um datasource e um dataset
e que interceptam os pontos espaço-temporais determinados*/
// [[Rcpp::export]]
SEXP getTrajectoryByTerralibStBoxXPtr(SEXP datasource, SEXP dataset, SEXP envelope, SEXP period){

  ////////////////////COMEÇO DO CODIGO
  try
  {
    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);

    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);

    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);

    te::gm::Envelope env = Rcpp::as<te::gm::Envelope>(envelope);
    te::dt::TimePeriod per = Rcpp::as<te::dt::TimePeriod>(period);
    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["trajId"], dset["trajName"]);
    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo,per,te::dt::OVERLAPS,env,te::gm::INTERSECTS,te::common::FORWARDONLY).release();
    dataset->moveBeforeFirst();



    Rcpp::XPtr<te::st::TrajectoryDataSet> dsPtr(dataset);
    return dsPtr;
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}


/*Metodo para buscar como ponteiro vectorlist
 trajetorias a partir de um datasource e um dataset
 e que interceptam os pontos espaço-temporais determinados*/
// [[Rcpp::export]]
SEXP getTrajectoryByTerralibStBox(SEXP datasource, SEXP dataset, SEXP envelope, SEXP period){

  ////////////////////COMEÇO DO CODIGO
  try
  {
    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);
   // te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["trajId"], dset["trajName"]);
    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);

    te::gm::Envelope env = Rcpp::as<te::gm::Envelope>(envelope);
    std::cout<<"linha 536";

    te::dt::TimePeriod per = Rcpp::as<te::dt::TimePeriod>(period);
    std::cout<<"linha 539";
    //return the entire trajectories and not only the trajectory pactches (false) --- now its selecting patches
    std::auto_ptr<te::st::TrajectoryDataSet> trajdataset = te::st::STDataLoader::getDataSet(tjinfo,per, te::dt::DURING, env, te::gm::INTERSECTS, te::common::FORWARDONLY, true);
    std::cout<<"linha 542";

    boost::ptr_vector<te::st::Trajectory> trajectories;
    trajdataset->getTrajectorySet(trajectories);
    ////////Codigo para mandar como Lista

    if(trajectories.size()==0){
      printf("Lista vazia");
    }
    Rcpp::List listaDePontos;


    for (int i = 0; i < trajectories.size(); ++i)
    {

      std::string count = "trajetoria";
      count += NumberToString(i);

      listaDePontos.push_back(trajectories[i]/*,count*/);

    }
    return (listaDePontos);    /////Codigo antigo
    /*dataset->moveBeforeFirst();

    ////////Codigo para mandar como Lista
    if(dataset == 0)
    {
      std::cout << "Trajectory Data Set is NULL!" << std::endl;
      return Rcpp::wrap("DataSet Vazio");
    }
    std::vector<double> x;
    std::vector<double> y;
    std::vector<std::string> tempo;
    std::vector<int> srid;
    std::vector<std::string> obj_id;

    Rcpp::List listaDePontos;


    while(dataset->moveNext())
    {
      std::auto_ptr<te::dt::DateTime> time = dataset->getTime();
      std::auto_ptr<te::gm::Geometry> geom = dataset->getGeometry();

      if(geom->getGeometryType()=="Point"){

        te::gm::Point* coordenadas = (dynamic_cast<te::gm::Point*> (geom.get()));

        x.push_back(coordenadas->getX());
        y.push_back(coordenadas->getY());
        srid.push_back(coordenadas->getSRID());

      }

      tempo.push_back(time->toString());

    }

    listaDePontos.push_back(tempo,"time");
    listaDePontos.push_back(x,"x");
    listaDePontos.push_back(y,"y");
    listaDePontos.push_back(srid,"srid");
    listaDePontos.push_back(obj_id,"obj_id");

    return (listaDePontos);*/
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}


// [[Rcpp::export]]
void LoadTrajectoryDataSetFromPostGIS()
{
  try
  {
    //Indicates the data source
    te::da::DataSourceInfo dsinfo;

    std::map<std::string, std::string> connInfo;
    connInfo["PG_HOST"] = "localhost";   // or "localhost";
    connInfo["PG_PORT"] = "5432";
    connInfo["PG_USER"] = "postgres";
    connInfo["PG_PASSWORD"] = "teste2ou3";
    connInfo["PG_DB_NAME"] = "vessel_trajectory";
    connInfo["PG_CONNECT_TIMEOUT"] = "4";
    connInfo["PG_CLIENT_ENCODING"] = "CP1252";     // "LATIN1"; //"WIN1252"

    dsinfo.setConnInfo(connInfo);
    dsinfo.setType("POSTGIS");

    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);



    std::string datasetname = "vessel_trajectories";
    std::string phTimeName = "datahora";
    std::string geomName = "ponto";
    std::string objIdName = "vessel_id";
    std::string tjIdName = "traj_id_unique"; //the object id. In this example, we just have object id and not trajectory id.

    //Use the STDataLoader to create a TrajectoryDataSet with all observations of all trajectories
    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, datasetname, phTimeName, geomName, objIdName, "", tjIdName, "");



    //Use the STDataLoader to get information about all distinct objects that exist in the dataset
    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);

    std::cout << std::endl << "Number of distinct objects and/or trajectories: " << output.size() << std::endl << std::endl;

    for (std::size_t i = 0; i < output.size(); ++i)
    {
      std::cout << std::endl << "------------------------------------- " << std::endl;
      std::cout << std::endl << "Object Id: " << output[i].getObjId() << std::endl;
      std::cout << std::endl << "Trajectory Id: " << output[i].getTrajId() << std::endl;

      //Get one trajectory of one object
      std::auto_ptr<te::st::TrajectoryDataSet> trajdataset = te::st::STDataLoader::getDataSet(output[i]);

      boost::ptr_vector<te::st::Trajectory> trajectories;

      trajdataset->getTrajectorySet(trajectories);

      std::cout << std::endl << "Number of trajectories: " << trajectories.size() << std::endl;

      for (std::size_t i = 0; i < trajectories.size(); ++i)
      {
        //PrintTrajectory(&trajectories[i]);

        //get trajectories whose st-extents interesect the i-th trajectory st-extent
        te::gm::Envelope spatialExt = trajectories[i].getSpatialExtent();
        std::auto_ptr<te::dt::DateTimePeriod> temporalExt = trajectories[i].getTemporalExtent();

        //return the entire trajectories and not only the trajectory pactches (false) --- noww is getting patches
        std::auto_ptr<te::st::TrajectoryDataSet> trajdataset2 = te::st::STDataLoader::getDataSet(tjinfo,
                                                                                                 *temporalExt.get(), te::dt::DURING, spatialExt, te::gm::INTERSECTS, te::common::FORWARDONLY, true);

        boost::ptr_vector<te::st::Trajectory> trajectories2;

        trajdataset2->getTrajectorySet(trajectories2);

        std::cout << std::endl << "Number of trajectories that intersects: " << trajectories2.size() << std::endl;

        //PrintTrajectory(&trajectories2[i]);
      }
    }
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurred in LoadTrajectoryDataSetFromPostGIS: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurred in LoadTrajectoryDataSetFromPostGIS!" << std::endl;
  }
}

// [[Rcpp::export]]
void LoadTrajectoryDataSetFromPostGIS2(SEXP datasource, SEXP dataset)
{
  try
  {

    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);

    //te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["trajId"], dset["trajName"]);
  //  te::st::TrajectoryDataSetInfo tjinfo(dsinfo, datasetname, phTimeName, geomName, objIdName, "", tjIdName, "");
    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);


    //Use the STDataLoader to get information about all distinct objects that exist in the dataset
    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);

    std::cout << std::endl << "Number of distinct objects and/or trajectories: " << output.size() << std::endl << std::endl;

    for (std::size_t i = 0; i < output.size(); ++i)
    {
      std::cout << std::endl << "------------------------------------- " << std::endl;
      std::cout << std::endl << "Object Id: " << output[i].getObjId() << std::endl;
      std::cout << std::endl << "Trajectory Id: " << output[i].getTrajId() << std::endl;

      //Get one trajectory of one object
      std::auto_ptr<te::st::TrajectoryDataSet> trajdataset = te::st::STDataLoader::getDataSet(output[i]);

      boost::ptr_vector<te::st::Trajectory> trajectories;

      trajdataset->getTrajectorySet(trajectories);

      std::cout << std::endl << "Number of trajectories: " << trajectories.size() << std::endl;

      for (std::size_t i = 0; i < trajectories.size(); ++i)
      {
        //PrintTrajectory(&trajectories[i]);

        //get trajectories whose st-extents interesect the i-th trajectory st-extent
        te::gm::Envelope spatialExt = trajectories[i].getSpatialExtent();
        std::auto_ptr<te::dt::DateTimePeriod> temporalExt = trajectories[i].getTemporalExtent();

        //return the entire trajectories and not only the trajectory pactches (false) --- now its getting patches
        std::auto_ptr<te::st::TrajectoryDataSet> trajdataset2 = te::st::STDataLoader::getDataSet(tjinfo,
                                                                                                 *temporalExt.get(), te::dt::DURING, spatialExt, te::gm::INTERSECTS, te::common::FORWARDONLY, true);

        boost::ptr_vector<te::st::Trajectory> trajectories2;

        trajdataset2->getTrajectorySet(trajectories2);

        std::cout << std::endl << "Number of trajectories that intersects: " << trajectories2.size() << std::endl;

        //PrintTrajectory(&trajectories2[i]);
      }
    }
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurred in LoadTrajectoryDataSetFromPostGIS: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurred in LoadTrajectoryDataSetFromPostGIS!" << std::endl;
  }
}

///Enviar trajetorias a partir de um ponteiro externo que retorna ao começo.
// [[Rcpp::export]]
SEXP receiveReturnXPTR(SEXP teste){
  SEXP teste2 = teste;
  boost::ptr_vector<te::st::Trajectory> trajectories;
  Rcpp::XPtr<te::st::TrajectoryDataSet> dsPtr(teste2);
  std::cout<<(dsPtr->getObjIdPropName())<<std::endl;
  Rcpp::XPtr<te::st::TrajectoryDataSet> data = dsPtr;

  data->moveBeforeFirst();
  data->getTrajectorySet(trajectories);

  Rcpp::List listaDePontos;


  for (int i = 0; i < trajectories.size(); ++i)
  {

    std::string count = "trajetoria";
    count += NumberToString(i);

    listaDePontos.push_back(trajectories[i]/*,count*/);

  }

  //te::da::DataSourceManager::getInstance().detach(dsinfo.getId());
trajectories.release();
 // te::st::TrajectoryDataSet(trajectories);
//teste = testeDataSet;
  return (listaDePontos);
}

///Enviar trajetorias em 2 partes a partir de um ponteiro externo que retorna ao começo.
// [[Rcpp::export]]
SEXP receiveReturnPartsXPTR(SEXP teste){
  SEXP teste2 = teste;
  boost::ptr_vector<te::st::Trajectory> trajectories;
  Rcpp::XPtr<te::st::TrajectoryDataSet> dsPtr(teste2);
  std::cout<<(dsPtr->getObjIdPropName())<<std::endl;
  Rcpp::XPtr<te::st::TrajectoryDataSet> data = dsPtr;

  //data->moveBeforeFirst();
  if(data->isBeforeBegin()){


  data->getTrajectorySet(trajectories);
  data->moveBeforeFirst();
    Rcpp::List listaDePontos;


    for (int i = 0; i < trajectories.size()/2; ++i)
    {
      data->moveNext();
      std::string count = "trajetoria";
      count += NumberToString(i);

      listaDePontos.push_back(trajectories[i]/*,count*/);
      std::cout<<sizeof listaDePontos << std::endl;

    }

    //te::da::DataSourceManager::getInstance().detach(dsinfo.getId());
    trajectories.release();
    // te::st::TrajectoryDataSet(trajectories);
    //teste = testeDataSet;
    return (listaDePontos);

  }
std::cout<<"Nao estou no começo"<<std::endl;

  int j=0;
  while(data->movePrevious()) {

    std::cout<<j++<<std::endl;
  }


    data->getTrajectorySet(trajectories);

  Rcpp::List listaDePontos;


  for (int i = j; i < trajectories.size(); ++i)
  {

    std::string count = "trajetoria";
    count += NumberToString(i);

    listaDePontos.push_back(trajectories[i]/*,count*/);

  }

  //te::da::DataSourceManager::getInstance().detach(dsinfo.getId());
  trajectories.release();
  // te::st::TrajectoryDataSet(trajectories);
  //teste = testeDataSet;
  return (listaDePontos);
}

///Enviar trajetorias em x partes a partir de um ponteiro externo que retorna ao começo.
// [[Rcpp::export]]
SEXP getPartsXPTR(SEXP teste, int division){
  SEXP teste2 = teste;
  boost::ptr_vector<te::st::Trajectory> trajectories;
  Rcpp::XPtr<te::st::TrajectoryDataSet> dsPtr(teste2);
  std::cout<<(dsPtr->getObjIdPropName())<<std::endl;
  Rcpp::XPtr<te::st::TrajectoryDataSet> data = dsPtr;
  int div = division ;

  int pos = 0;

  while(data->movePrevious()){
    pos++;
    std::cout<<pos<<std::endl;
  }

    data->moveBeforeFirst();
    data->getTrajectorySet(trajectories);
    data->moveBeforeFirst();

  int tempcount = 0;
  while (tempcount < pos){
    tempcount++;
   // std::cout<<tempcount<<std::endl;

    data->moveNext();
  }

  Rcpp::List listaDePontos;
  std::cout<<trajectories.size() <<std::endl;
  int tsize = 0;
  tsize = trajectories.size();
  int stop = (tsize/div);

    if(stop <= tsize){
      if((tsize-pos)<stop){
        stop = (tsize-pos);
      }
    for (int i = 0; i < stop; ++i)
    {

      if(data->isAfterEnd()){
        break;
      }
      data->moveNext();
      std::string count = "trajetoria";
      count += NumberToString(i);

      listaDePontos.push_back(trajectories[pos+i]);

     // std::cout<<sizeof listaDePontos << std::endl;

    }
    }
    //te::da::DataSourceManager::getInstance().detach(dsinfo.getId());
    trajectories.release();
    // te::st::TrajectoryDataSet(trajectories);
    //teste = testeDataSet;
    return (listaDePontos);

  }

///Enviar parte x de trajetorias  divididas em y partes a partir de um ponteiro
//externo que retorna ao começo.
// [[Rcpp::export]]
SEXP getSpecificPartsXPTR(SEXP teste,int part, int division){
  SEXP teste2 = teste;
  boost::ptr_vector<te::st::Trajectory> trajectories;
  Rcpp::XPtr<te::st::TrajectoryDataSet> dsPtr(teste2);
  std::cout<<(dsPtr->getObjIdPropName())<<std::endl;
  Rcpp::XPtr<te::st::TrajectoryDataSet> data = dsPtr;
  int div = division ;

  int pos = 0;

  data->moveBeforeFirst();
  data->getTrajectorySet(trajectories);
  data->moveBeforeFirst();

  Rcpp::List listaDePontos;
  std::cout<<trajectories.size() <<std::endl;
  int tsize = 0;
  tsize = trajectories.size();
  int start = (tsize/div)*(part-1);
  int stop = (tsize/div)*part;


    for (int i = start; i < stop; ++i)
    {

      std::string count = "trajetoria";
      count += NumberToString(i);

      listaDePontos.push_back(trajectories[i]);

      // std::cout<<sizeof listaDePontos << std::endl;

    }

  //te::da::DataSourceManager::getInstance().detach(dsinfo.getId());
  trajectories.release();
  // te::st::TrajectoryDataSet(trajectories);
  //teste = testeDataSet;
  return (listaDePontos);

}

//Gets trajectories and returns the one with the specified Trajectory ID
// [[Rcpp::export]]
SEXP getTrajectoryByTrajID(SEXP datasource, SEXP dataset, std::string id){

  ////////////////////COMEÇO DO CODIGO
  try
  {

    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);

    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);


    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);
    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();

    if(dataset==0){
      printf("Lista vazia");
    }
    boost::ptr_vector<te::st::Trajectory> trajectories;
    dataset->getTrajectorySet(trajectories);
    ////////Codigo para mandar como Lista

    if(trajectories.size()==0){
      printf("Lista vazia");
    }
    Rcpp::List listaDePontos;


    for (int i = 0; i < trajectories.size(); ++i)
    {

      std::string count = "trajetoria";
      count += NumberToString(i);
      if(trajectories[i].getTrajId() == id){
      listaDePontos.push_back(trajectories[i]/*,count*/);
      }
    }
    return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}

//Gets trajectories and returns the ones with the specified obj ID
// [[Rcpp::export]]
SEXP getTrajectoryByObjID(SEXP datasource, SEXP dataset, std::string id){

  ////////////////////COMEÇO DO CODIGO
  try
  {

    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);

    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);


    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);
    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();

    if(dataset==0){
      printf("Lista vazia");
    }
    boost::ptr_vector<te::st::Trajectory> trajectories;
    dataset->getTrajectorySet(trajectories);
    ////////Codigo para mandar como Lista

    if(trajectories.size()==0){
      printf("Lista vazia");
    }
    Rcpp::List listaDePontos;


    for (int i = 0; i < trajectories.size(); ++i)
    {

      std::string count = "trajetoria";
      count += NumberToString(i);
      if(trajectories[i].getObjId() == id){
        listaDePontos.push_back(trajectories[i]/*,count*/);
      }
    }
    return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}

///Enviar parte x de trajetorias  divididas em y partes a partir do db
// [[Rcpp::export]]
SEXP getSpecificPartsDB(SEXP datasource, SEXP dataset,int part, int division){
  try{
  //Indicates the data source
  te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


  //It creates a new Data Source and put it into the manager
  CreateDataSourceAndUpdateManager(dsinfo);
  std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);

  te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"], "", dset["trajId"], dset["trajName"]);


  std::vector<te::st::TrajectoryDataSetInfo> output;
  te::st::STDataLoader::getInfo(tjinfo, output);
                                                                                                                                                      te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();

  int div = division ;

  int pos = 0;

  boost::ptr_vector<te::st::Trajectory> trajectories;

  dataset->moveBeforeFirst();
  dataset->getTrajectorySet(trajectories);

  Rcpp::List listaDePontos;
  std::cout<<trajectories.size() <<std::endl;
  int tsize = 0;
  tsize = trajectories.size();
  int start = (tsize/div)*(part-1);
  int stop = (tsize/div)*part;


  for (int i = start; i < stop; ++i)
  {

    std::string count = "trajetoria";
    count += NumberToString(i);

    listaDePontos.push_back(trajectories[i]);

    // std::cout<<sizeof listaDePontos << std::endl;

  }

  //te::da::DataSourceManager::getInstance().detach(dsinfo.getId());
  trajectories.release();
  // te::st::TrajectoryDataSet(trajectories);
  //teste = testeDataSet;
  return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}

//Gets trajectories and returns the ones with the specified obj ID
// [[Rcpp::export]]
SEXP getTrajectoryByObjIDList(SEXP datasource, SEXP dataset, SEXP idlist){

  ////////////////////COMEÇO DO CODIGO
  try
  {
    Rcpp::List ids = idlist;
    Rcpp::List listaDePontos;
    //Indicates the data source
    te::da::DataSourceInfo dsinfo = Rcpp::as<te::da::DataSourceInfo>(datasource);


    //It creates a new Data Source and put it into the manager
    CreateDataSourceAndUpdateManager(dsinfo);
    std::map<std::string,std::string> dset = Rcpp::as<std::map<std::string,std::string> >(dataset);
    std::cout<<"Antes do for"<<std::endl;

    for(int j = 0 ; j<ids.size();j++){

      Rcpp::String idrcpp =  ids[j];

     std::string id = idrcpp;
     std::cout<<j<<" of "<<ids.size()<<std::endl;

    te::st::TrajectoryDataSetInfo tjinfo(dsinfo, dset["tableName"], dset["timeName"], dset["geomName"], dset["objId"],id, dset["trajId"], dset["trajName"]);


    std::vector<te::st::TrajectoryDataSetInfo> output;
    te::st::STDataLoader::getInfo(tjinfo, output);
    te::st::TrajectoryDataSet* dataset = te::st::STDataLoader::getDataSet(tjinfo).release();

    if(dataset==0){
      printf("Lista vazia");
    }
    boost::ptr_vector<te::st::Trajectory> trajectories;
    dataset->getTrajectorySet(trajectories);
    ////////Codigo para mandar como Lista

    if(trajectories.size()==0){
      printf("Lista vazia");
    }



    for (int i = 0; i < trajectories.size(); ++i)
    {

      std::string count = "trajetoria";
      count += NumberToString(i);

        listaDePontos.push_back(trajectories[i]/*,count*/);

    }
    }
    return (listaDePontos);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurried in TrajectoryExamplesFromKML: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurried in TrajectoryExamplesFromKML!" << std::endl;
  }
  /////////////////////FIM

  return Rcpp::wrap("Fail");
}


/////////////////////FIM
