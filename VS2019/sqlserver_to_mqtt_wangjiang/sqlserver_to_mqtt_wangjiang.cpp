/**
 * @file sqlserver_to_mqtt_wangjiang.cpp
 * @author Tam (821239820@qq.com)
 * @brief 望江隧道数据推送，从SQL Server推送至MQTT
 * @version 0.1
 * @date 2023-03-24
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <iostream>
#include <mqtt/client.h>
#include <co/all.h>
#include <nanodbc/nanodbc.h>
#define TEST

Tasked taskQueue;
co::WaitGroup waitGroup;

static std::string GBKToUTF8(const char *strGBK)
{
   int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
   wchar_t *wstr = new wchar_t[len + 1];
   memset(wstr, 0, len + 1);
   MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
   len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
   char *str = new char[len + 1];
   memset(str, 0, len + 1);
   WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
   std::string strTemp = str;
   if (wstr)
      delete[] wstr;
   if (str)
      delete[] str;
   return strTemp;
}

void sendMQTT(std::string _str)
{
   std::string clientID = "advcenter_mqtt_" + std::to_string(co::thread_id()) + "_" + std::to_string(co::coroutine_id());
   std::cout << "clientid is " << clientID << "\n";
#ifdef TEST
   mqtt::client client("tcp://127.0.0.1:1883", clientID);
#else
   mqtt::client client("tcp://200.1.11.245:30056", clientID);
#endif // TEST

   mqtt::connect_options connOpts;
   connOpts.set_keep_alive_interval(20);
   connOpts.set_clean_session(true);
#ifndef TEST
   connOpts.set_user_name("admin");
   connOpts.set_password("public");
#endif // TEST
   try
   {
      client.connect(connOpts);
      if (client.is_connected())
      {
         auto pubmsg = mqtt::make_message("/HZSD/WJSD", GBKToUTF8(_str.c_str()).c_str());
         pubmsg->set_qos(2);
         client.publish(pubmsg);
         LOG << "mqtt发送成功";
         client.disconnect();
      }
      else
      {
         //go([_str]
         //   { sendMQTT(_str); });
         ELOG << "MQTT服务器断开连接";
      }
   }
   catch (const std::exception &e)
   {
      //go([_str]
      //   { sendMQTT(_str); });
      ELOG << "连接MQTT服务器失败  " << e.what();
   }
}

void sqlQuery(std::string _table)
{

#ifdef TEST
   auto const connection_string("DRIVER={SQL SERVER};SERVER=172.20.61.29;DATABASE=TEST;UID=sa;PWD=Mima123321;");
#else
   auto const connection_string("DRIVER={SQL SERVER};SERVER=33.94.101.35;DATABASE=pims;UID=sa;PWD=P@ssw0rd;");
#endif // TEST

   try
   {
      nanodbc::connection conn(connection_string);
      conn.connected();
#ifdef TEST
      std::string dsn = "select n_code,t_rec_time from dbo." + _table;
#else
      std::string dsn = "select point_name,point_value from dbo." + _table;
#endif // TEST

      auto res = nanodbc::execute(conn, dsn);
      if (res.affected_rows() == -1)
      {
         json::Json json;
         json::Json data;
         json::Json value;

         while (res.next())
         {
            json::Json item;
#ifdef TEST
            auto const n_code = res.get<std::string>("n_code");
            auto const t_rec_time = res.get<std::string>("t_rec_time");
             item.add_member("pointUuid", n_code);
             item.add_member("pointValue", t_rec_time);
             value.push_back(item);
            //std::cout << n_code << " : " << t_rec_time << std::endl;
#else
            auto point_name = res.get<std::string>("point_name");
            auto point_value = res.get<std::string>("point_value");
            if (point_value.size() == 0)
               point_value = "0";
            float value_f = str::to_double(point_value);
            item.add_member("pointUuid", point_name);
            if (value_f == 0 && point_value.size() != 1)
            {
               item.add_member("pointValue", point_value);
            }
            else
            {
               item.add_member("pointValue", value_f);
            }
            value.push_back(item);
            // LOG << point_name << " : " << point_value;
            // std::cout << point_name << " : " << point_value << std::endl;
#endif // TEST
         }
         data.push_back(json::Json().add_member("time", epoch::ms()).add_member("value", value));
         json.add_member("data", data);
         std::string str_json = json.str().c_str();
         go([str_json]
            { sendMQTT(str_json); });

         // LOG << json.str();
      }
      conn.disconnect();
   }
   catch (std::exception const &e)
   {
      ELOG << "SQL Server 连接错误!" << _table << e.what();
   }
   //co::sleep(1000 * 10);
   //go([_table]
   //   { sqlQuery(_table); });
}

int main()
{
   flag::set_value("cout", "true");
#ifdef TEST

   // go([]
   //     { sendMQTT("hello"); });
   taskQueue.run_every(std::bind(sqlQuery, "cur_aero"), 5);
   taskQueue.run_every(std::bind(sqlQuery, "cur_cms"), 5);
   taskQueue.run_every(std::bind(sqlQuery, "cur_csls"), 5);
   taskQueue.run_every(std::bind(sqlQuery, "cur_et"), 5);
   taskQueue.run_every(std::bind(sqlQuery, "cur_vd"), 5);
   taskQueue.run_every(std::bind(sqlQuery, "cur_vi"), 5);
   //go([]
   //   { sqlQuery("cur_aero"); });
   //go([]
   //   { sqlQuery("cur_cms"); });
   //go([]
   //   { sqlQuery("cur_csls"); });
   //go([]
   //   { sqlQuery("cur_et"); });
   //go([]
   //   { sqlQuery("cur_vd"); });
   //go([]
   //   { sqlQuery("cur_vi"); });

#endif

#ifndef TEST
   taskQueue.run_every(std::bind(sqlQuery,"lq_car_light"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_car_luiliang"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_co"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_shoubao"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_shuibeng_jiance"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_fengji"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_vi"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_shuiwei_jiance"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_wengan"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_wenshidu"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_xinhaodeng"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_yangan"), 10);
   taskQueue.run_every(std::bind(sqlQuery, "lq_zhaoming_yunxing"), 10);

   //go([]
   //   { sqlQuery("lq_car_light"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_car_luiliang"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_co"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_shoubao"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_shuibeng_jiance"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_fengji"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_vi"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_shuiwei_jiance"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_wengan"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_wenshidu"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_xinhaodeng"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_yangan"); });
   //co::sleep(50);
   //go([]
   //   { sqlQuery("lq_zhaoming_yunxing"); });
#endif

   waitGroup.add();
   waitGroup.wait();
}
