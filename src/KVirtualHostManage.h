/*
 * Kconf.gvm->h
 *
 *  Created on: 2010-4-19
 *      Author: keengo
 */
#ifndef KVIRTUALHOSTMANAGE_H_
#define KVIRTUALHOSTMANAGE_H_
#include <map>
#include <vector>
#include "global.h"
#define VH_CONFIG_FILE	"/etc/vh.xml"
#include "KXmlSupport.h"
#include "KVirtualHost.h"
#include "KNsVirtualHost.h"
#include "KMutex.h"
#include "KIpVirtualHost.h"
class KGTempleteVirtualHost;

class KVirtualHostManage {
public:
	KVirtualHostManage();
	virtual ~KVirtualHostManage();
	void copy(KVirtualHostManage *vm);
	void getMenuHtml(std::stringstream &s,KVirtualHost *vh,std::stringstream &url,int t);
	void getHtml(std::stringstream &s,std::string vh_name,int id, KUrlValue &attribute);
	void getListenHtml(std::stringstream &s);
	void build(std::stringstream &s);
	bool saveConfig(std::string &errMsg);
	bool vhAction(KVirtualHost *ov,KTempleteVirtualHost *tm,KUrlValue &attribute, std::string &errMsg);
	bool vhBaseAction(KUrlValue &attribute,std::string &errMsg);

	int getNextInstanceId();
	void getAutoName(std::string &name);
	void inheritVirtualHost(KVirtualHost *vh,bool clearFlag);
	void addAllVirtualHost();
	void startStaticListen(std::vector<KListenHost *> &services,bool start);
	bool startService(KListenHost *service, bool start);
	int getCount();
public:
	/*
	 * ������������
	 */
	bool updateVirtualHost(KVirtualHost *vh);
	bool updateVirtualHost(KVirtualHost *vh,KVirtualHost *ov);
	/*
	 * ������������
	 */
	bool addVirtualHost(KVirtualHost *vh);
	/*
	 * ɾ����������
	 */
	bool removeVirtualHost(KVirtualHost *vh);
	/*
	 * ������������������rq�ϡ�
	 */
	void bindVirtualHost(KServer *server);
	query_vh_result queryVirtualHost(KHttpRequest *rq,const char *site);
	void getAllVh(std::list<std::string> &vhs,bool status,bool onlydb);
	bool getAllTempleteVh(const char *templeteGroup,std::list<std::string> &vhs);
	void getAllGroupTemplete(std::list<std::string> &vhs);
	KVirtualHost *refsVirtualHostByName(std::string name);

	KTempleteVirtualHost *refsTempleteVirtualHost(std::string name);
	bool updateTempleteVirtualHost(KTempleteVirtualHost *tvh);
	bool removeTempleteVirtualHost(std::string name);
	/////////[232]
	/*
	�����е�virtualHost��updatedFlag=false,׼������vh.xml�ļ���
	*/
	//void updateAllVirtualHost();
	/*
	�Ѿ���������vh.xml�ļ���������updatedFlag=false��ɾ��,��ɱ����Ӧ�Ľ��̡�
	*/
	//void checkAllVirtualHost();
	KBaseVirtualHost globalVh;
public:
	friend class KHttpServerParser;
	friend class KHttpManage;
private:
	void getVhIndex(std::stringstream &s,KVirtualHost *vh,int id,int t,u_short default_http_port);
	void inheriteAll();
	bool internalAddVirtualHost(KVirtualHost *vh,KVirtualHost *ov);
	bool internalRemoveVirtualHost(KVirtualHost *vh,bool removeIndex=true);
	//KNsVirtualHost *getNsVirtualHost(std::string name);
	void getAllVhHtml(std::stringstream &s,int tvh);
	void getVhDetail(std::stringstream &s, KVirtualHost *vh,bool edit,int t);
	void flushListen(KVirtualHost *vh);
	/*
	 * �������������б�
	 */
	std::map<std::string, KVirtualHost *> avh;
	/*
	 * ����ģ����������
	 */
	std::map<std::string, KGTempleteVirtualHost *> gtvhs;
	/*
	* �����е�vh��������
	*/
	void internalBindAllVirtualHost();
	/*
	* �󶨵�������
	*/
	void internalBindVirtualHost(KVirtualHost *vh);
private:
	KMutex lock;
	//KMutex envLock;
	KMutex instancelock;
	int curInstanceId;
};
#endif /* KVIRTUALHOSTMANAGE_H_ */