/*
 * Copyright (c) 2010, NanChang BangTeng Inc
 * All Rights Reserved.
 *
 * You may use the Software for free for non-commercial use
 * under the License Restrictions.
 *
 * You may modify the source code(if being provieded) or interface
 * of the Software under the License Restrictions.
 *
 * You may use the Software for commercial use after purchasing the
 * commercial license.Moreover, according to the license you purchased
 * you may get specified term, manner and content of technical
 * support from NanChang BangTeng Inc
 *
 * See COPYING file for detail.
 */
#include <iostream>
#include "KConfigParser.h"
#include "KXml.h"
#include "KAccess.h"
#include "KAcserverManager.h"
#include "KWriteBackManager.h"
#include "malloc_debug.h"
#include "KThreadPool.h"
#include "KRequestQueue.h"
#include "lib.h"
#include "md5.h"
#include "do_config.h"
#include "KListenConfigParser.h"
#include "KSelectorManager.h"
#include "KHttpFilterDsoManage.h"
using namespace std;
KConfigParser::KConfigParser() {

}

KConfigParser::~KConfigParser() {
}
bool KConfigParser::startElement(std::string &context, std::string &qName,
		std::map<std::string, std::string> &attribute) {
#ifndef _WIN32
	if (qName == "run") {
		cconf->run_user = attribute["user"];
		cconf->run_group = attribute["group"];
	}
#endif
	if (qName == "admin") {
		cconf->admin_user = attribute["user"];
		cconf->passwd_crypt = parseCryptType(attribute["crypt"].c_str());
		cconf->auth_type = KHttpAuth::parseType(attribute["auth_type"].c_str());
		cconf->admin_passwd = attribute["password"];
		explode(attribute["admin_ips"].c_str(), '|', cconf->admin_ips);
		change_admin_password_crypt_type();
		return true;
	}
	if (qName == "gzip") {
		cconf->only_gzip_cache = atoi(attribute["only_gzip_cache"].c_str());
		cconf->min_gzip_length = atoi(attribute["min_gzip_length"].c_str());
		cconf->gzip_level = atoi(attribute["gzip_level"].c_str());
		return true;
	}
	if(qName == "autoupdate"){
		if(attribute["type"]=="off"){
			cconf->autoupdate = AUTOUPDATE_OFF;
		}else if(attribute["type"] == "down"){
			cconf->autoupdate = AUTOUPDATE_DOWN;
		}else{
			cconf->autoupdate = AUTOUPDATE_ON;
		}
		if(attribute["install"].size()>0){
			conf.set_autoupdate_time(atoi(attribute["install"].c_str()));
		}
	}
	if (qName == "cache") {
		cconf->default_cache = atoi(attribute["default"].c_str());
		cconf->max_cache_size = (unsigned)get_size(attribute["max_cache_size"].c_str());
		/////////[191]
		if (attribute["memory"].size()>0) {
			cconf->mem_cache = get_size(attribute["memory"].c_str());
		}
		if (attribute["refresh_time"].size()>0) {
			cconf->refresh_time = atoi(attribute["refresh_time"].c_str());
		}
#ifdef ENABLE_DISK_CACHE
		if (attribute["disk"].size()>0) {
			cconf->disk_cache = get_size(attribute["disk"].c_str());
		}
		if (attribute["disk_dir"].size()>0) {
			SAFE_STRCPY(cconf->disk_cache_dir2 ,attribute["disk_dir"].c_str());	
		}
		if (attribute["disk_work_time"].size()>0) {
			SAFE_STRCPY(cconf->disk_work_time ,attribute["disk_work_time"].c_str());	
		}	
#endif
		return true;
	}
	if (qName == "connect") {
		cconf->max_per_ip = atoi(attribute["max_per_ip"].c_str());
		cconf->max = atoi(attribute["max"].c_str());
		if (cconf->max < 0) {
			cconf->max = 1024;
		}
		cconf->per_ip_deny = atoi(attribute["per_ip_deny"].c_str());
		return true;
	}
#ifdef ENABLE_REQUEST_QUEUE
	if(qName == "request_queue"){
		globalRequestQueue.set(
			atoi(attribute["max_worker"].c_str()),
			atoi(attribute["max_queue"].c_str())
			);
		return true;
	}
#endif
/////////[192]
	if (qName == "log") {
		if (attribute["level"].size()>0) {
			cconf->log_level = atoi(attribute["level"].c_str());
		}
		SAFE_STRCPY(cconf->log_rotate , attribute["rotate_time"].c_str());
		if (attribute["rotate_size"].size()>0) {
			cconf->log_rotate_size = get_size(attribute["rotate_size"].c_str());
		}
		cconf->logs_day = atoi(attribute["logs_day"].c_str());
		if (attribute["logs_size"].size()>0) {
			cconf->logs_size = get_size(attribute["logs_size"].c_str());
		}
		if (attribute["error_rotate_size"].size()>0) {
			cconf->error_rotate_size = get_size(attribute["error_rotate_size"].c_str());
		}
		cconf->log_handle = attribute["log_handle"]=="1";
		cconf->log_sub_request = attribute["log_sub_request"]=="1";
		return true;
	}
	return false;

}
bool KConfigParser::startElement(KXmlContext *context, std::map<std::string,
		std::string> &attribute) {
	if (context->path == "config") {
#ifdef ENABLE_KSAPI_FILTER
		if (context->qName == "http_filter") {
			if (conf.hfdm==NULL) {
				conf.hfdm = new KHttpFilterDsoManage;
			}
			conf.hfdm->add(attribute);
		}
#endif
	}
	if (context->path == "config/connect" && context->qName == "per_ip") {
		KPerIpConnect *per_ip = new KPerIpConnect;
		memset(per_ip,0,sizeof(KPerIpConnect));
		if (attribute["max"]=="deny") {
			per_ip->deny = true;
		} else {
			per_ip->deny = false;
			per_ip->max = atoi(attribute["max"].c_str());
		}
		KIpAclBase::addIpModel(attribute["src"].c_str(),per_ip->src);
		KConfig *c = (KConfig *)context->getData();
		assert(c);
		if (c->per_ip_last) {
			c->per_ip_last->next = per_ip;
		} else {
			c->per_ip_head = per_ip;
		}
		c->per_ip_last = per_ip;
		return true;
	}
	return false;
}
bool KConfigParser::startCharacter(std::string &context, std::string &qName,
		char *character, int len) {

	if (context == "config/admin") {
		if (qName == "allowip") {
			cconf->admin_ips.push_back(character);
			return true;
		}
	}
	if (context == "config/cache") {
		if (qName == "memory") {
			cconf->mem_cache = get_size(character);
			return true;
		}
		if (qName == "refresh_time") {
			cconf->refresh_time = atoi(character);
			return true;
		}
#ifdef ENABLE_DISK_CACHE
		if(qName=="disk"){
			cconf->disk_cache = get_size(character);
		}
		if (qName=="disk_dir") {
			SAFE_STRCPY(cconf->disk_cache_dir2 ,character);			
		}
		if (qName == "disk_work_time") {
			SAFE_STRCPY(cconf->disk_work_time,character);
		}
#endif
	}
	if (context == "config") {
#ifdef MALLOCDEBUG
		if(qName == "mallocdebug"){
			if (atoi(character)==0) {
				cconf->mallocdebug = false;
			} else {
				cconf->mallocdebug = true;
			}
		}
#endif
		if(qName == "lang"){
			SAFE_STRCPY(cconf->lang , character);
			return true;
		}
		if (qName == "worker_thread") {
			cconf->select_count = atoi(character);
			return true;
		}
		if (qName == "keep_alive") {
			cconf->keep_alive = atoi(character);
			return true;
		}
		if (qName == "timeout") {
			cconf->set_time_out(atoi(character));
			return true;
		}
		if (qName == "connect_timeout") {
			cconf->set_connect_time_out(atoi(character));
			return true;
		}
#ifdef KSOCKET_UNIX	
		if(qName == "unix_socket"){
			if(strcmp(character,"1")==0 || strcmp(character,"on")==0){
                                cconf->unix_socket=true;
                        }else{
                                cconf->unix_socket=false;
                        }
		}
#endif
		if (qName == "error_try_count") {
			cconf->setErrorTryCount(atoi(character));
		}
		if(qName == "path_info"){
			if(strcmp(character,"1")==0 || strcmp(character,"on")==0){
				cconf->path_info=true;
			}else{
				cconf->path_info=false;
			}
		}
		if (qName == "error_ip") {
			cconf->error_ip = (atoi(character)==1);
		}
		if(qName == "remove_accept_encoding"){
                        if(strcmp(character,"1")==0 || strcmp(character,"on")==0){
                                cconf->removeAcceptEncoding=true;
                        }else{
                                cconf->removeAcceptEncoding=false;
                        }
                }
		if(qName == "min_free_thread"){
			cconf->min_free_thread = atoi(character);
		}
/////////[193]
		if(qName == "access_log"){
			SAFE_STRCPY(cconf->access_log ,character);
		}
		if(qName == "access_log_handle"){
			SAFE_STRCPY(cconf->logHandle, character);
		}
		if(qName == "log_handle_concurrent"){
			cconf->maxLogHandle = atoi(character);
		}
		/////////[194]
		if(qName == "server_software"){
			SAFE_STRCPY(cconf->server_software ,character);
		}
		if (qName == "hostname") {
			cconf->setHostname(character);
		}
#ifdef ENABLE_TF_EXCHANGE
#if 0
		if (qName == "tempfile") {
			cconf->tmpfile = atoi(character);
			if (cconf->tmpfile<0 || cconf->tmpfile>2) {
				cconf->tmpfile = 1;
			}
		}
#endif
		if (qName == "max_post_size") {
			cconf->max_post_size = get_size(character);
		}
#endif
		/////////[195]
		if (qName == "async_io") {
			cconf->async_io = (atoi(character)==1);
			return true;
		}
		if (qName == "worker_io") {
			cconf->worker_io = atoi(character);
			return true;
		}
		if (qName == "worker_dns") {
			cconf->worker_dns = atoi(character);
			return true;
		}

	}
	return false;

}
bool KConfigParser::startCharacter(KXmlContext *context, char *character,
		int len) {

	return false;
}
bool KConfigParser::endElement(std::string &context, std::string &qName) {
	
	return false;
}
bool KConfigParser::endElement(KXmlContext *context) {

	return false;
}
void KConfigParser::startXml(const std::string &encoding) {
	
}
void KConfigParser::endXml(bool result) {
}


