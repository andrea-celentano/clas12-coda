-- MySQL dump 10.11
--
-- Host: clondb1    Database: daq_clasrun
-- ------------------------------------------------------
-- Server version	4.1.20-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `daq_clasrun`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `daq_clasrun` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `daq_clasrun`;

--
-- Table structure for table `HPS35`
--

DROP TABLE IF EXISTS `HPS35`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `code` text NOT NULL,
  `inputs` text NOT NULL,
  `outputs` text NOT NULL,
  `first` varchar(32) NOT NULL default '',
  `next` varchar(32) NOT NULL default '',
  `inuse` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35`
--

LOCK TABLES `HPS35` WRITE;
/*!40000 ALTER TABLE `HPS35` DISABLE KEYS */;
INSERT INTO `HPS35` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB35:clondaq3','yes','','46'),('hps1','{hps1_slave.so usr} {hps2.so usr} ','','EB35:clondaq3','yes','hps2','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB35:clondaq3','no','dpm0','39'),('hps1gtp','{gtp1.so usr}  ','','','no','dpm0','no'),('hps2gtp','{gtp1.so usr}  ','','','no','dpm0','no'),('dpm0','{dpm.so usr}  ','','EB35:clondaq3','no','dpm1','51'),('dpm1','{dpm.so usr}  ','','EB35:clondaq3','no','dpm2','52'),('dpm2','{dpm.so usr}  ','','EB35:clondaq3','no','dpm3','53'),('dpm3','{dpm.so usr}  ','','EB35:clondaq3','no','dpm4','54'),('dpm4','{dpm.so usr}  ','','EB35:clondaq3','no','dpm5','55'),('dpm5','{dpm.so usr}  ','','EB35:clondaq3','no','dpm6','56'),('dpm6','{dpm.so usr}  ','','EB35:clondaq3','no','dpm7','57'),('dpm7','{dpm.so usr}  ','','EB35:clondaq3','no','dpm8','66'),('dpm8','{dpm.so usr}  ','','EB35:clondaq3','no','dpm9','59'),('dpm9','{dpm.so usr}  ','','EB35:clondaq3','no','dpm10','60'),('dpm10','{dpm.so usr}  ','','EB35:clondaq3','no','dpm11','61'),('dpm11','{dpm.so usr}  ','','EB35:clondaq3','no','dpm12','62'),('dpm12','{dpm.so usr}  ','','EB35:clondaq3','no','dpm13','63'),('dpm13','{dpm.so usr}  ','','EB35:clondaq3','no','dpm14','64'),('dpm14','{dpm.so usr}  ','','EB35:clondaq3','no','','65'),('dtm0','{dtm.so usr}  ','','','no','','no'),('dtm1','{dtm.so usr}  ','','','no','','no'),('EB35','{CODA} {CODA} ','hps11:hps11 hps1:hps1 hps2:hps2 dpm0:dpm0 dpm1:dpm1 dpm2:dpm2 dpm3:dpm3 dpm4:dpm4 dpm5:dpm5 dpm6:dpm6 dpm7:dpm7 dpm8:dpm8 dpm9:dpm9 dpm10:dpm10 dpm11:dpm11 dpm12:dpm12 dpm13:dpm13 dpm14:dpm14','ET35:clondaq3','yes','','no'),('ETT35','  ','ET35:clondaq3','ET351:clondaq5','yes','','no'),('ER35','{CODA}  ','ET351:clondaq5','coda_0','yes','','no'),('ET35','{1000} {500000} ','EB35:clondaq3','ETT35:clondaq3','yes','ET351','no'),('ET351','{1000} {500000} ','ETT35:clondaq3','ER35:clondaq5','no','','no'),('coda_0','{/data/hps/hps} {CODA} ','ER35:clondaq5','','yes','','no');
/*!40000 ALTER TABLE `HPS35` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_ECAL`
--

DROP TABLE IF EXISTS `HPS35_ECAL`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_ECAL` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `code` text NOT NULL,
  `inputs` text NOT NULL,
  `outputs` text NOT NULL,
  `first` varchar(32) NOT NULL default '',
  `next` varchar(32) NOT NULL default '',
  `inuse` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_ECAL`
--

LOCK TABLES `HPS35_ECAL` WRITE;
/*!40000 ALTER TABLE `HPS35_ECAL` DISABLE KEYS */;
INSERT INTO `HPS35_ECAL` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB35:clondaq3','yes','','46'),('hps1','{hps1_slave.so usr} {hps2.so usr} ','','EB35:clondaq3','yes','hps2','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB35:clondaq3','no','','39'),('hps1gtp','{gtp1.so usr}  ','','','no','','no'),('hps2gtp','{gtp1.so usr}  ','','','no','','no'),('EB35','{CODA} {CODA} ','hps11:hps11 hps1:hps1 hps2:hps2','ET35:clondaq3','yes','','no'),('ETT35','  ','ET35:clondaq3','ET351:clondaq5','yes','','no'),('ER35','{CODA}  ','ET351:clondaq5','coda_0','yes','','no'),('ET35','{1000} {500000} ','EB35:clondaq3','ETT35:clondaq3','yes','ET351','no'),('ET351','{1000} {500000} ','ETT35:clondaq3','ER35:clondaq5','no','','no'),('coda_0','{/data/hps/hpsecal} {CODA} ','ER35:clondaq5','','yes','','no');
/*!40000 ALTER TABLE `HPS35_ECAL` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_ECAL_option`
--

DROP TABLE IF EXISTS `HPS35_ECAL_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_ECAL_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_ECAL_option`
--

LOCK TABLES `HPS35_ECAL_option` WRITE;
/*!40000 ALTER TABLE `HPS35_ECAL_option` DISABLE KEYS */;
INSERT INTO `HPS35_ECAL_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/data/hps/hpsecal'),('confFile','/usr/clas12/release/0.3/parms/trigger/HPS/ECAL/hps_cosmic.cnf'),('rocMask','0x00000000 0x00000000 0x000040a0 0x00000000');
/*!40000 ALTER TABLE `HPS35_ECAL_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_ECAL_pos`
--

DROP TABLE IF EXISTS `HPS35_ECAL_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_ECAL_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_ECAL_pos`
--

LOCK TABLES `HPS35_ECAL_pos` WRITE;
/*!40000 ALTER TABLE `HPS35_ECAL_pos` DISABLE KEYS */;
INSERT INTO `HPS35_ECAL_pos` VALUES ('hps11',1,1),('hps1',2,1),('hps2',3,1),('hps1gtp',5,1),('hps2gtp',6,1),('EB35',11,4),('ETT35',13,4),('ER35',15,4),('ET35',11,6),('ET351',13,6),('coda_0',15,6);
/*!40000 ALTER TABLE `HPS35_ECAL_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_ECAL_script`
--

DROP TABLE IF EXISTS `HPS35_ECAL_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_ECAL_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_ECAL_script`
--

LOCK TABLES `HPS35_ECAL_script` WRITE;
/*!40000 ALTER TABLE `HPS35_ECAL_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPS35_ECAL_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_SVT`
--

DROP TABLE IF EXISTS `HPS35_SVT`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_SVT` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `code` text NOT NULL,
  `inputs` text NOT NULL,
  `outputs` text NOT NULL,
  `first` varchar(32) NOT NULL default '',
  `next` varchar(32) NOT NULL default '',
  `inuse` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_SVT`
--

LOCK TABLES `HPS35_SVT` WRITE;
/*!40000 ALTER TABLE `HPS35_SVT` DISABLE KEYS */;
INSERT INTO `HPS35_SVT` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB35:clondaq3','yes','','46'),('dpm0','{dpm.so usr}  ','','EB35:clondaq3','yes','dpm1','51'),('dpm1','{dpm.so usr}  ','','EB35:clondaq3','no','dpm2','52'),('dpm2','{dpm.so usr}  ','','EB35:clondaq3','no','dpm3','53'),('dpm3','{dpm.so usr}  ','','EB35:clondaq3','no','dpm4','54'),('dpm4','{dpm.so usr}  ','','EB35:clondaq3','no','dpm5','55'),('dpm5','{dpm.so usr}  ','','EB35:clondaq3','no','dpm6','56'),('dpm6','{dpm.so usr}  ','','EB35:clondaq3','no','dpm7','57'),('dpm7','{dpm.so usr}  ','','EB35:clondaq3','no','dpm8','66'),('dpm8','{dpm.so usr}  ','','EB35:clondaq3','no','dpm9','59'),('dpm9','{dpm.so usr}  ','','EB35:clondaq3','no','dpm10','60'),('dpm10','{dpm.so usr}  ','','EB35:clondaq3','no','dpm11','61'),('dpm11','{dpm.so usr}  ','','EB35:clondaq3','no','dpm12','62'),('dpm12','{dpm.so usr}  ','','EB35:clondaq3','no','dpm13','63'),('dpm13','{dpm.so usr}  ','','EB35:clondaq3','no','dpm14','64'),('dpm14','{dpm.so usr}  ','','EB35:clondaq3','no','','65'),('dtm0','{dtm.so usr}  ','','','no','','no'),('dtm1','{dtm.so usr}  ','','','no','','no'),('EB35','{CODA} {CODA} ','hps11:hps11 dpm0:dpm0 dpm1:dpm1 dpm2:dpm2 dpm3:dpm3 dpm4:dpm4 dpm5:dpm5 dpm6:dpm6 dpm7:dpm7 dpm8:dpm8 dpm9:dpm9 dpm10:dpm10 dpm11:dpm11 dpm12:dpm12 dpm13:dpm13 dpm14:dpm14','ET35:clondaq3','yes','','no'),('ETT35','  ','ET35:clondaq3','ET351:clondaq5','yes','','no'),('ER35','{CODA}  ','ET351:clondaq5','coda_0','yes','','no'),('ET35','{1000} {500000} ','EB35:clondaq3','ETT35:clondaq3','yes','ET351','no'),('ET351','{1000} {500000} ','ETT35:clondaq3','ER35:clondaq5','no','','no'),('coda_0','{/data/hps/hpssvt} {CODA} ','ER35:clondaq5','','yes','','no');
/*!40000 ALTER TABLE `HPS35_SVT` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_SVT_option`
--

DROP TABLE IF EXISTS `HPS35_SVT_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_SVT_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_SVT_option`
--

LOCK TABLES `HPS35_SVT_option` WRITE;
/*!40000 ALTER TABLE `HPS35_SVT_option` DISABLE KEYS */;
INSERT INTO `HPS35_SVT_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/data/hps/hpssvt'),('confFile','/usr/clas12/release/0.3/parms/trigger/HPS/TEST/MakeDaqCrash2-baseline.cnf'),('rocMask','0x00000000 0x00000007 0xfbf84000 0x00000000');
/*!40000 ALTER TABLE `HPS35_SVT_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_SVT_pos`
--

DROP TABLE IF EXISTS `HPS35_SVT_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_SVT_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_SVT_pos`
--

LOCK TABLES `HPS35_SVT_pos` WRITE;
/*!40000 ALTER TABLE `HPS35_SVT_pos` DISABLE KEYS */;
INSERT INTO `HPS35_SVT_pos` VALUES ('hps11',1,1),('dpm0',8,1),('dpm1',9,1),('dpm2',10,1),('dpm3',11,1),('dpm4',12,1),('dpm5',13,1),('dpm6',14,1),('dpm7',15,1),('dpm8',16,1),('dpm9',17,1),('dpm10',18,1),('dpm11',19,1),('dpm12',20,1),('dpm13',21,1),('dpm14',22,1),('dtm0',24,1),('dtm1',25,1),('EB35',11,4),('ETT35',13,4),('ER35',15,4),('ET35',11,6),('ET351',13,6),('coda_0',15,6);
/*!40000 ALTER TABLE `HPS35_SVT_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_SVT_script`
--

DROP TABLE IF EXISTS `HPS35_SVT_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_SVT_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_SVT_script`
--

LOCK TABLES `HPS35_SVT_script` WRITE;
/*!40000 ALTER TABLE `HPS35_SVT_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPS35_SVT_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_option`
--

DROP TABLE IF EXISTS `HPS35_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_option`
--

LOCK TABLES `HPS35_option` WRITE;
/*!40000 ALTER TABLE `HPS35_option` DISABLE KEYS */;
INSERT INTO `HPS35_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/data/hps/hps'),('confFile','/usr/clas12/release/0.3/parms/trigger/HPS/TEST/MakeDaqCrash2.cnf'),('rocMask','0x00000000 0x00000007 0xfbf840a0 0x00000000');
/*!40000 ALTER TABLE `HPS35_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_pos`
--

DROP TABLE IF EXISTS `HPS35_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_pos`
--

LOCK TABLES `HPS35_pos` WRITE;
/*!40000 ALTER TABLE `HPS35_pos` DISABLE KEYS */;
INSERT INTO `HPS35_pos` VALUES ('hps11',1,1),('hps1',2,1),('hps2',3,1),('hps1gtp',5,1),('hps2gtp',6,1),('dpm0',8,1),('dpm1',9,1),('dpm2',10,1),('dpm3',11,1),('dpm4',12,1),('dpm5',13,1),('dpm6',14,1),('dpm7',15,1),('dpm8',16,1),('dpm9',17,1),('dpm10',18,1),('dpm11',19,1),('dpm12',20,1),('dpm13',21,1),('dpm14',22,1),('dtm0',24,1),('dtm1',25,1),('EB35',11,4),('ETT35',13,4),('ER35',15,4),('ET35',11,6),('ET351',13,6),('coda_0',15,6);
/*!40000 ALTER TABLE `HPS35_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS35_script`
--

DROP TABLE IF EXISTS `HPS35_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS35_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS35_script`
--

LOCK TABLES `HPS35_script` WRITE;
/*!40000 ALTER TABLE `HPS35_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPS35_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `defaults`
--

DROP TABLE IF EXISTS `defaults`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `defaults` (
  `class` varchar(32) NOT NULL default '',
  `code` text NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `defaults`
--

LOCK TABLES `defaults` WRITE;
/*!40000 ALTER TABLE `defaults` DISABLE KEYS */;
INSERT INTO `defaults` VALUES ('TS','{hps1_master.so usr} {hps2.so usr}'),('ROC','{hps1_slave.so usr} {hps2.so usr}');
/*!40000 ALTER TABLE `defaults` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `links`
--

DROP TABLE IF EXISTS `links`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `links` (
  `name` char(100) NOT NULL default '',
  `type` char(4) NOT NULL default '',
  `host` char(30) default NULL,
  `state` char(10) default NULL,
  `port` int(11) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `links`
--

LOCK TABLES `links` WRITE;
/*!40000 ALTER TABLE `links` DISABLE KEYS */;
INSERT INTO `links` VALUES ('dpm14->EB35','TCP','clondaq3','down',59570),('dpm13->EB35','TCP','clondaq3','down',51858),('dpm12->EB35','TCP','clondaq3','down',49507),('dpm11->EB35','TCP','clondaq3','down',39409),('dpm10->EB35','TCP','clondaq3','down',54663),('dpm9->EB35','TCP','clondaq3','down',35349),('dpm8->EB35','TCP','clondaq3','down',47699),('dpm7->EB35','TCP','clondaq3','down',40769),('dpm6->EB35','TCP','clondaq3','down',45244),('dpm5->EB35','TCP','clondaq3','down',58785),('dpm4->EB35','TCP','clondaq3','down',54976),('dpm3->EB35','TCP','clondaq3','down',50729),('dpm2->EB35','TCP','clondaq3','down',44109),('dpm1->EB35','TCP','clondaq3','down',54098),('dpm0->EB35','TCP','clondaq3','down',48554),('hps2->EB35','TCP','clondaq3','down',42943),('hps1->EB35','TCP','clondaq3','down',48644),('hps11->EB35','TCP','clondaq3','down',51330);
/*!40000 ALTER TABLE `links` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `priority`
--

DROP TABLE IF EXISTS `priority`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `priority` (
  `class` char(32) NOT NULL default '',
  `priority` int(11) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `priority`
--

LOCK TABLES `priority` WRITE;
/*!40000 ALTER TABLE `priority` DISABLE KEYS */;
INSERT INTO `priority` VALUES ('ET',25),('ET2ET',24),('ROC',11),('EB',15),('ANA',19),('ER',23),('LOG',27),('TS',-27);
/*!40000 ALTER TABLE `priority` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `process`
--

DROP TABLE IF EXISTS `process`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `process` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `id` int(11) NOT NULL default '0',
  `cmd` varchar(128) NOT NULL default '',
  `type` varchar(32) NOT NULL default '',
  `host` varchar(32) NOT NULL default '',
  `port` int(11) NOT NULL default '0',
  `state` varchar(32) NOT NULL default '',
  `pid` int(11) NOT NULL default '0',
  `inuse` varchar(32) NOT NULL default '',
  `clone` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `process`
--

LOCK TABLES `process` WRITE;
/*!40000 ALTER TABLE `process` DISABLE KEYS */;
INSERT INTO `process` VALUES ('dpm3',54,'coda_roc','ROC','dpm3',0,'booted',0,'5002','no'),('dpm4',55,'coda_roc','ROC','dpm4',0,'booted',0,'5001','no'),('dtm1',50,'coda_roc','ROC','dtm1',0,'booted',0,'5002','no'),('dtm0',49,'coda_roc','ROC','dtm0',0,'booted',0,'5001','no'),('hps2gtp',40,'coda_roc','ROC','hps2gtp',0,'configured',0,'5004','no'),('hps1gtp',38,'coda_roc','ROC','hps1gtp',-999,'configured',-999,'5003',''),('hps2',39,'coda_roc_gef','ROC','hps2',0,'configured',0,'5002','no'),('hps1',37,'coda_roc_gef','ROC','hps1',0,'configured',0,'5002','no'),('hps11',46,'coda_roc_gef','TS','hps11',0,'booted',0,'5002','no'),('hps12',58,'$CODA_BIN/coda_roc','ROC','hps12',0,'booted',0,'5002','no'),('dpm9',60,'coda_roc','ROC','dpm9',0,'booted',0,'5002','no'),('dpm8',59,'coda_roc','ROC','dpm8',0,'booted',0,'5001','no'),('dpm7',66,'coda_roc','ROC','dpm7',0,'booted',0,'5001','no'),('dpm6',57,'coda_roc','ROC','dpm6',0,'booted',0,'5001','no'),('dpm5',56,'coda_roc','ROC','dpm5',0,'booted',0,'5001','no'),('dpm2',53,'coda_roc','ROC','dpm2',0,'booted',0,'5001','no'),('dpm12',63,'coda_roc','ROC','dpm12',0,'booted',0,'5001','no'),('dpm13',64,'coda_roc','ROC','dpm13',0,'booted',0,'5001','no'),('dpm14',65,'coda_roc','ROC','dpm14',0,'booted',0,'5001','no'),('dpm1',52,'coda_roc','ROC','dpm1',0,'booted',0,'5001','no'),('dpm10',61,'coda_roc','ROC','dpm10',0,'booted',0,'5001','no'),('dpm11',62,'coda_roc','ROC','dpm11',0,'booted',0,'5002','no'),('dpm0',51,'coda_roc','ROC','dpm0',0,'booted',0,'5001','no'),('ER35',93,'coda_erc','ER','clondaq5',0,'booted',0,'5003','no'),('ETT35',92,'coda_ett','ETT','clondaq3',0,'booted',0,'5002','no'),('ET35',91,'coda_et','ET','clondaq3',11111,'booted',0,'5006','no'),('EB35',90,'coda_ebc','EB','clondaq3',0,'booted',0,'5005','no'),('clashps',0,'rcServer','RCS','clondaq3.jlab.org',55206,'dormant',0,'yes','no'),('ET351',94,'coda_et','ET','clondaq5',11111,'booted',0,'5004','no');
/*!40000 ALTER TABLE `process` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `runTypes`
--

DROP TABLE IF EXISTS `runTypes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `runTypes` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `id` int(11) NOT NULL default '0',
  `inuse` varchar(32) NOT NULL default '',
  `category` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `runTypes`
--

LOCK TABLES `runTypes` WRITE;
/*!40000 ALTER TABLE `runTypes` DISABLE KEYS */;
INSERT INTO `runTypes` VALUES ('HPS35_SVT',2,'yes',''),('HPS35_ECAL',0,'no',''),('HPS35',1,'no','');
/*!40000 ALTER TABLE `runTypes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sessions`
--

DROP TABLE IF EXISTS `sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sessions` (
  `name` varchar(64) character set latin1 collate latin1_bin NOT NULL default '',
  `id` int(11) NOT NULL default '0',
  `owner` varchar(32) NOT NULL default '',
  `inuse` varchar(32) NOT NULL default '',
  `log_name` varchar(32) NOT NULL default '',
  `rc_name` varchar(32) NOT NULL default '',
  `runNumber` int(11) NOT NULL default '0',
  `config` varchar(32) NOT NULL default '',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sessions`
--

LOCK TABLES `sessions` WRITE;
/*!40000 ALTER TABLE `sessions` DISABLE KEYS */;
INSERT INTO `sessions` VALUES ('clashps',0,'clondaq3.jlab.org clasrun 2508 9','yes','clashps_msg','RunControl',7042,'HPS35_SVT');
/*!40000 ALTER TABLE `sessions` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-01-26 16:02:48
