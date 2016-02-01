-- MySQL dump 10.11
--
-- Host: clondb1    Database: daq_classvt
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
-- Current Database: `daq_classvt`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `daq_classvt` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `daq_classvt`;

--
-- Table structure for table `SVT257_ER`
--

DROP TABLE IF EXISTS `SVT257_ER`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT257_ER` (
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
-- Dumping data for table `SVT257_ER`
--

LOCK TABLES `SVT257_ER` WRITE;
/*!40000 ALTER TABLE `SVT257_ER` DISABLE KEYS */;
INSERT INTO `SVT257_ER` VALUES ('svt2','{hps1_master.so usr} {hps2.so usr} ','','EB2:svt2','yes','','2'),('svt5','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','yes','svt7','5'),('svt7','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','no','','7'),('EB2','{CODA} {CODA} ','svt2:svt2 svt5:svt5 svt7:svt7','','yes','','no'),('ER2','{CODA}  ','','coda_1','yes','','no'),('ETT2','  ','ET2:svt2','ET21:svtsystem1','yes','','no'),('ET2','{500} {200000} ','','ETT2:svt2','yes','','no'),('coda_1','{/work/svt/svt257er} {CODA} ','ER2:svt2','','yes','','no'),('ET21','{1000} {500000} ','ETT2:svt2','','no','','no');
/*!40000 ALTER TABLE `SVT257_ER` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT257_ER_option`
--

DROP TABLE IF EXISTS `SVT257_ER_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT257_ER_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `SVT257_ER_option`
--

LOCK TABLES `SVT257_ER_option` WRITE;
/*!40000 ALTER TABLE `SVT257_ER_option` DISABLE KEYS */;
INSERT INTO `SVT257_ER_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/svt/svt257er'),('confFile','/usr/clas12/release/0.3/parms/trigger/classvt.cnf'),('rocMask','0x00000000 0x00000000 0x00000000 0x000000a4');
/*!40000 ALTER TABLE `SVT257_ER_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT257_ER_pos`
--

DROP TABLE IF EXISTS `SVT257_ER_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT257_ER_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `SVT257_ER_pos`
--

LOCK TABLES `SVT257_ER_pos` WRITE;
/*!40000 ALTER TABLE `SVT257_ER_pos` DISABLE KEYS */;
INSERT INTO `SVT257_ER_pos` VALUES ('svt2',3,1),('svt5',5,1),('svt7',7,1),('EB2',3,3),('ER2',5,3),('ETT2',9,3),('ET2',3,5),('coda_1',5,5),('ET21',9,5);
/*!40000 ALTER TABLE `SVT257_ER_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT257_ER_script`
--

DROP TABLE IF EXISTS `SVT257_ER_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT257_ER_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `SVT257_ER_script`
--

LOCK TABLES `SVT257_ER_script` WRITE;
/*!40000 ALTER TABLE `SVT257_ER_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `SVT257_ER_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT_MVT`
--

DROP TABLE IF EXISTS `SVT_MVT`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT_MVT` (
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
-- Dumping data for table `SVT_MVT`
--

LOCK TABLES `SVT_MVT` WRITE;
/*!40000 ALTER TABLE `SVT_MVT` DISABLE KEYS */;
INSERT INTO `SVT_MVT` VALUES ('svt2','{hps1_master.so usr} {hps2.so usr} ','','EB2:svt2','yes','','2'),('svt5','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','yes','svt7','5'),('svt7','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','no','mvt1','7'),('mvt1','{hps1mvt_slave.so usr} {hps2mvtcomposite.so usr} ','','EB2:svt2','no','','69'),('EB2','{CODA} {CODA} ','svt2:svt2 svt5:svt5 svt7:svt7 mvt1:mvt1','ET2:svt2','yes','','no'),('ER2','{CODA}  ','','coda_1','yes','','no'),('ETT2','  ','ET2:svt2','ET21:svtsystem1','yes','','no'),('ET2','{100} {900000} ','EB2:svt2','ETT2:svt2','yes','','no'),('coda_1','{/scratch/svt/svtmvt} {CODA} ','ER2:svt2','','yes','','no'),('ET21','{100} {900000} ','ETT2:svt2','','no','','no');
/*!40000 ALTER TABLE `SVT_MVT` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT_MVT_option`
--

DROP TABLE IF EXISTS `SVT_MVT_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT_MVT_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `SVT_MVT_option`
--

LOCK TABLES `SVT_MVT_option` WRITE;
/*!40000 ALTER TABLE `SVT_MVT_option` DISABLE KEYS */;
INSERT INTO `SVT_MVT_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/scratch/svt/svtmvt'),('confFile','/usr/clas12/release/0.3/parms/trigger/classvt.cnf'),('rocMask','0x00000000 0x00000020 0x00000000 0x000000a4');
/*!40000 ALTER TABLE `SVT_MVT_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT_MVT_pos`
--

DROP TABLE IF EXISTS `SVT_MVT_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT_MVT_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `SVT_MVT_pos`
--

LOCK TABLES `SVT_MVT_pos` WRITE;
/*!40000 ALTER TABLE `SVT_MVT_pos` DISABLE KEYS */;
INSERT INTO `SVT_MVT_pos` VALUES ('svt2',3,1),('svt5',5,1),('svt7',7,1),('mvt1',9,1),('EB2',3,3),('ER2',5,3),('ETT2',9,3),('ET2',3,5),('coda_1',5,5),('ET21',9,5);
/*!40000 ALTER TABLE `SVT_MVT_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `SVT_MVT_script`
--

DROP TABLE IF EXISTS `SVT_MVT_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `SVT_MVT_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `SVT_MVT_script`
--

LOCK TABLES `SVT_MVT_script` WRITE;
/*!40000 ALTER TABLE `SVT_MVT_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `SVT_MVT_script` ENABLE KEYS */;
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
INSERT INTO `links` VALUES ('svt2->EB2','TCP','svt2','up',35969),('svt4->EB4','TCP','svt4','up',45557),('svt1->EB1','TCP','localhost','down',35080),('svt5->EB5','TCP','localhost','down',34859),('svt6->EB6','TCP','localhost','down',40565),('svt5->EB2','TCP','svt2','up',45601),('svt7->EB2','TCP','svt2','up',36156),('svt5->EB4','TCP','svt4','up',57530),('svt7->EB7','TCP','localhost','down',39131),('mvt1->EB77','TCP','mvt1','down',37072),('mvt1->EB2','TCP','svt2','up',36524);
/*!40000 ALTER TABLE `links` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1`
--

DROP TABLE IF EXISTS `mvt1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1` (
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
-- Dumping data for table `mvt1`
--

LOCK TABLES `mvt1` WRITE;
/*!40000 ALTER TABLE `mvt1` DISABLE KEYS */;
INSERT INTO `mvt1` VALUES ('mvt1','{hps1mvt.so usr} {hps2mvtcomposite.so usr} ','','EB77:mvt1','yes','','69'),('EB77','{CODA} {CODA} ','mvt1:mvt1','ET77:mvt1','yes','','no'),('ER77','{CODA}  ','ET77:mvt1','coda_0','yes','','no'),('ET77','{200} {500000} ','EB77:mvt1','ER77:mvt1','yes','','no'),('coda_0','{/work/mvt/mvt1} {CODA} ','ER77:mvt1','','yes','','no');
/*!40000 ALTER TABLE `mvt1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1_option`
--

DROP TABLE IF EXISTS `mvt1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mvt1_option`
--

LOCK TABLES `mvt1_option` WRITE;
/*!40000 ALTER TABLE `mvt1_option` DISABLE KEYS */;
INSERT INTO `mvt1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/mvt/mvt1'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000020 0x00000000 0x00000000');
/*!40000 ALTER TABLE `mvt1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1_pos`
--

DROP TABLE IF EXISTS `mvt1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mvt1_pos`
--

LOCK TABLES `mvt1_pos` WRITE;
/*!40000 ALTER TABLE `mvt1_pos` DISABLE KEYS */;
INSERT INTO `mvt1_pos` VALUES ('mvt1',2,1),('EB77',2,3),('ER77',4,3),('ET77',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `mvt1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1_script`
--

DROP TABLE IF EXISTS `mvt1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mvt1_script`
--

LOCK TABLES `mvt1_script` WRITE;
/*!40000 ALTER TABLE `mvt1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `mvt1_script` ENABLE KEYS */;
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
INSERT INTO `process` VALUES ('EB2',42,'coda_ebc','EB','svt2',0,'active',-999,'5007',''),('EB3',43,'','EB','localhost',-999,'',-999,'',''),('EB4',44,'','EB','svt4',-999,'configured',-999,'5004',''),('EB5',45,'','EB','localhost',-999,'downloaded',-999,'5004',''),('EB6',46,'','EB','localhost',-999,'downloaded',-999,'5004',''),('ER2',82,'coda_erc','ER','svt2',-999,'active',-999,'5005',''),('ER3',83,'','ER','localhost',-999,'',-999,'',''),('ER4',84,'','ER','svt4',-999,'configured',-999,'5001',''),('ER5',85,'','ER','localhost',-999,'downloaded',-999,'5002',''),('ER6',86,'','ER','localhost',-999,'downloaded',-999,'5001',''),('svt3',3,'','ROC','localhost',-999,'',-999,'',''),('svt4',4,'$CODA_BIN/coda_roc','TS','svt4',-999,'configured',-999,'5007',''),('svt5',5,'coda_roc_gef','ROC','svt5',-999,'active',-999,'5002',''),('svt6',6,'','ROC','localhost',-999,'downloaded',-999,'5006',''),('EB1',41,'$CODA_BIN/coda_eb','EB','localhost',0,'booted',0,'5004','no'),('svt1',1,'$CODA_BIN/coda_roc','ROC','localhost',0,'downloaded',0,'5002','no'),('ER1',81,'$CODA_BIN/coda_er','ER','localhost',0,'booted',0,'5002','no'),('svt2',2,'coda_roc_gef','TS','svt2',0,'active',0,'5009','no'),('classvt',0,'rcServer','RCS','svt4',30164,'dormant',0,'yes','no'),('classvt2',0,'rcServer','RCS','svt2',52966,'dormant',0,'yes','no'),('ROC0',0,'$CODA_BIN/coda_roc','ROC','svt5',0,'booted',0,'5006','no'),('EB0',40,'$CODA_BIN/coda_eb','EB','svt5',0,'dormant',0,'no','no'),('svt7',7,'coda_roc_gef','ROC','svt7',0,'active',0,'5002','no'),('classvt25',0,'rcServer','RCS','svt2',52966,'dormant',0,'yes','no'),('ER77',90,'coda_erc','ER','mvt1',0,'downloaded',0,'5002','no'),('classvt4',0,'rcServer','RCS','svt4',52966,'dormant',0,'yes','no'),('classvt45',0,'rcServer','RCS','svt4',52966,'dormant',0,'yes','no'),('ER7',64,'$CODA_BIN/coda_er','ER','localhost',0,'booted',0,'5001','no'),('EB7',64,'$CODA_BIN/coda_eb','EB','localhost',0,'booted',0,'5004','no'),('classvt7',0,'rcServer','RCS','svt7',52966,'dormant',0,'yes','no'),('mvt1',69,'coda_roc_gef','ROC','mvt1',0,'active',0,'5002','no'),('ET2',2,'coda_et','ET','svt2',11111,'active',0,'5010','no'),('ETT2',22,'coda_ett','ETT','svt2',0,'active',0,'5003','no'),('ET21',21,'coda_et','ET','svtsystem1',11111,'active',0,'5002','no'),('ET77',99,'coda_et','ET','mvt1',11111,'downloaded',0,'5008','no'),('EB77',80,'coda_ebc','EB','mvt1',0,'downloaded',0,'5005','no'),('classvt257',0,'rcServer','RCS','svtsystem1.jlab.org',48565,'dormant',0,'yes','no');
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
INSERT INTO `runTypes` VALUES ('svt6',0,'no',''),('svt3',1,'no',''),('svt2_er',2,'yes',''),('svt5',3,'no',''),('svt4',4,'no',''),('svt2',5,'no',''),('svt1',6,'no',''),('svt25_er',7,'yes',''),('svt3_er',8,'no',''),('svt4_er',9,'yes',''),('svt5_er',10,'yes',''),('svt6_er',11,'yes',''),('svt1_er',12,'no',''),('svt5new',13,'no',''),('test0',14,'no',''),('svt257_er',15,'yes',''),('svt45_er',16,'yes',''),('svt7_er',17,'yes',''),('SVT257_ER',18,'yes',''),('SVT_MVT',20,'yes',''),('mvt1',19,'no','');
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
  `owner` varchar(64) NOT NULL default '',
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
INSERT INTO `sessions` VALUES ('classvt',0,'svt4 clasrun 2508 9998','yes','booted','RunControl',906,''),('clastest0',-999,'svt5 boiarino 1538 146','','','',0,''),('classvt2',0,'svt2 clasrun 2508 9998','yes','downloaded','RunControl',13,'svt2_er'),('classvt257',0,'svtsystem1.jlab.org clasrun 2508 9998','yes','configured','RunControl',232,'SVT_MVT'),('classvt25',0,'svt2 clasrun 2508 9998','yes','booted','RunControl',1,'svt25_er'),('classvt4',0,'svt4 clasrun 2508 9998','yes','configured','RunControl',55,'svt4_er'),('classvt45',0,'svt4 clasrun 2508 9998','yes','configured','RunControl',33,'svt45_er'),('classvt7',0,'svt7 clasrun 2508 9998','yes','booted','RunControl',1,''),('mvttest',0,'mvt1 clasrun 2508 9998','no','mvttest_msg','RunControl',3,'');
/*!40000 ALTER TABLE `sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1`
--

DROP TABLE IF EXISTS `svt1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1` (
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
-- Dumping data for table `svt1`
--

LOCK TABLES `svt1` WRITE;
/*!40000 ALTER TABLE `svt1` DISABLE KEYS */;
INSERT INTO `svt1` VALUES ('svt1','{svt1.so usr} {svt2.so usr} ','','EB1:localhost','yes','','no'),('EB1','  ','svt1:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_er`
--

DROP TABLE IF EXISTS `svt1_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_er` (
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
-- Dumping data for table `svt1_er`
--

LOCK TABLES `svt1_er` WRITE;
/*!40000 ALTER TABLE `svt1_er` DISABLE KEYS */;
INSERT INTO `svt1_er` VALUES ('svt1','{svt1.so usr} {svt2.so usr} ','','EB1:localhost','yes','','no'),('EB1','  ','svt1:localhost','','yes','','no'),('ER1','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER1:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt1_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_er_option`
--

DROP TABLE IF EXISTS `svt1_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt1_er_option`
--

LOCK TABLES `svt1_er_option` WRITE;
/*!40000 ALTER TABLE `svt1_er_option` DISABLE KEYS */;
INSERT INTO `svt1_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/w/stage3/BUFFER/svt/svt1er'),('SPLITMB','2047'),('rocMask','2');
/*!40000 ALTER TABLE `svt1_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_er_pos`
--

DROP TABLE IF EXISTS `svt1_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt1_er_pos`
--

LOCK TABLES `svt1_er_pos` WRITE;
/*!40000 ALTER TABLE `svt1_er_pos` DISABLE KEYS */;
INSERT INTO `svt1_er_pos` VALUES ('svt1',2,1),('EB1',2,3),('ER1',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `svt1_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_er_script`
--

DROP TABLE IF EXISTS `svt1_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt1_er_script`
--

LOCK TABLES `svt1_er_script` WRITE;
/*!40000 ALTER TABLE `svt1_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt1_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_option`
--

DROP TABLE IF EXISTS `svt1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt1_option`
--

LOCK TABLES `svt1_option` WRITE;
/*!40000 ALTER TABLE `svt1_option` DISABLE KEYS */;
INSERT INTO `svt1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','CODA'),('rocMask','2');
/*!40000 ALTER TABLE `svt1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_pos`
--

DROP TABLE IF EXISTS `svt1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt1_pos`
--

LOCK TABLES `svt1_pos` WRITE;
/*!40000 ALTER TABLE `svt1_pos` DISABLE KEYS */;
INSERT INTO `svt1_pos` VALUES ('svt1',2,1),('EB1',2,3);
/*!40000 ALTER TABLE `svt1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt1_script`
--

DROP TABLE IF EXISTS `svt1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt1_script`
--

LOCK TABLES `svt1_script` WRITE;
/*!40000 ALTER TABLE `svt1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt1_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2`
--

DROP TABLE IF EXISTS `svt2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2` (
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
-- Dumping data for table `svt2`
--

LOCK TABLES `svt2` WRITE;
/*!40000 ALTER TABLE `svt2` DISABLE KEYS */;
INSERT INTO `svt2` VALUES ('svt2','{hps1.so usr} {hps2.so usr} ','','EB2:localhost','yes','','2'),('EB2','{CODA} {CODA} ','svt2:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt2` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt257_er`
--

DROP TABLE IF EXISTS `svt257_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt257_er` (
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
-- Dumping data for table `svt257_er`
--

LOCK TABLES `svt257_er` WRITE;
/*!40000 ALTER TABLE `svt257_er` DISABLE KEYS */;
INSERT INTO `svt257_er` VALUES ('svt2','{hps1_master.so usr} {hps2.so usr} ','','EB2:svt2','yes','','2'),('svt5','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','yes','svt7','5'),('svt7','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','no','','7'),('EB2','{CODA} {CODA} ','svt2:svt2 svt5:svt5 svt7:svt7','','yes','','no'),('ER2','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER2:svt2','','yes','','no');
/*!40000 ALTER TABLE `svt257_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt257_er_option`
--

DROP TABLE IF EXISTS `svt257_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt257_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt257_er_option`
--

LOCK TABLES `svt257_er_option` WRITE;
/*!40000 ALTER TABLE `svt257_er_option` DISABLE KEYS */;
INSERT INTO `svt257_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/svt/svt257er'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x000000a4');
/*!40000 ALTER TABLE `svt257_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt257_er_pos`
--

DROP TABLE IF EXISTS `svt257_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt257_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt257_er_pos`
--

LOCK TABLES `svt257_er_pos` WRITE;
/*!40000 ALTER TABLE `svt257_er_pos` DISABLE KEYS */;
INSERT INTO `svt257_er_pos` VALUES ('svt2',3,1),('svt5',5,1),('svt7',7,1),('EB2',3,3),('ER2',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `svt257_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt257_er_script`
--

DROP TABLE IF EXISTS `svt257_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt257_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt257_er_script`
--

LOCK TABLES `svt257_er_script` WRITE;
/*!40000 ALTER TABLE `svt257_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt257_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt25_er`
--

DROP TABLE IF EXISTS `svt25_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt25_er` (
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
-- Dumping data for table `svt25_er`
--

LOCK TABLES `svt25_er` WRITE;
/*!40000 ALTER TABLE `svt25_er` DISABLE KEYS */;
INSERT INTO `svt25_er` VALUES ('svt2','{hps1_master.so usr} {hps2.so usr} ','','EB2:svt2','yes','','2'),('svt5','{hps1_slave.so usr} {hps2.so usr} ','','EB2:svt2','yes','','5'),('EB2','{CODA} {CODA} ','svt2:svt2 svt5:svt5','','yes','','no'),('ER2','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER2:svt2','','yes','','no');
/*!40000 ALTER TABLE `svt25_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt25_er_option`
--

DROP TABLE IF EXISTS `svt25_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt25_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt25_er_option`
--

LOCK TABLES `svt25_er_option` WRITE;
/*!40000 ALTER TABLE `svt25_er_option` DISABLE KEYS */;
INSERT INTO `svt25_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/w/stage3/BUFFER/svt/svt25er'),('SPLITMB','2047'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000024');
/*!40000 ALTER TABLE `svt25_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt25_er_pos`
--

DROP TABLE IF EXISTS `svt25_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt25_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt25_er_pos`
--

LOCK TABLES `svt25_er_pos` WRITE;
/*!40000 ALTER TABLE `svt25_er_pos` DISABLE KEYS */;
INSERT INTO `svt25_er_pos` VALUES ('svt2',3,1),('svt5',5,1),('EB2',3,3),('ER2',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `svt25_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt25_er_script`
--

DROP TABLE IF EXISTS `svt25_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt25_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt25_er_script`
--

LOCK TABLES `svt25_er_script` WRITE;
/*!40000 ALTER TABLE `svt25_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt25_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_er`
--

DROP TABLE IF EXISTS `svt2_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_er` (
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
-- Dumping data for table `svt2_er`
--

LOCK TABLES `svt2_er` WRITE;
/*!40000 ALTER TABLE `svt2_er` DISABLE KEYS */;
INSERT INTO `svt2_er` VALUES ('svt2','{hps1.so usr} {hps2.so usr} ','','EB2:localhost','yes','','2'),('EB2','{CODA} {CODA} ','svt2:localhost','','yes','','no'),('ER2','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER2:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt2_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_er_option`
--

DROP TABLE IF EXISTS `svt2_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt2_er_option`
--

LOCK TABLES `svt2_er_option` WRITE;
/*!40000 ALTER TABLE `svt2_er_option` DISABLE KEYS */;
INSERT INTO `svt2_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/svt/svt2er'),('SPLITMB','2047'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000004'),('confFile','none');
/*!40000 ALTER TABLE `svt2_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_er_pos`
--

DROP TABLE IF EXISTS `svt2_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt2_er_pos`
--

LOCK TABLES `svt2_er_pos` WRITE;
/*!40000 ALTER TABLE `svt2_er_pos` DISABLE KEYS */;
INSERT INTO `svt2_er_pos` VALUES ('svt2',3,1),('EB2',3,3),('ER2',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `svt2_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_er_script`
--

DROP TABLE IF EXISTS `svt2_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt2_er_script`
--

LOCK TABLES `svt2_er_script` WRITE;
/*!40000 ALTER TABLE `svt2_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt2_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_option`
--

DROP TABLE IF EXISTS `svt2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt2_option`
--

LOCK TABLES `svt2_option` WRITE;
/*!40000 ALTER TABLE `svt2_option` DISABLE KEYS */;
INSERT INTO `svt2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000004'),('dataFile','CODA'),('confFile','none');
/*!40000 ALTER TABLE `svt2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_pos`
--

DROP TABLE IF EXISTS `svt2_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt2_pos`
--

LOCK TABLES `svt2_pos` WRITE;
/*!40000 ALTER TABLE `svt2_pos` DISABLE KEYS */;
INSERT INTO `svt2_pos` VALUES ('svt2',3,1),('EB2',3,3);
/*!40000 ALTER TABLE `svt2_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt2_script`
--

DROP TABLE IF EXISTS `svt2_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt2_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt2_script`
--

LOCK TABLES `svt2_script` WRITE;
/*!40000 ALTER TABLE `svt2_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt2_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3`
--

DROP TABLE IF EXISTS `svt3`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3` (
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
-- Dumping data for table `svt3`
--

LOCK TABLES `svt3` WRITE;
/*!40000 ALTER TABLE `svt3` DISABLE KEYS */;
INSERT INTO `svt3` VALUES ('svt3','{svt1.so usr} {svt2.so usr} ','','EB3:localhost','yes','','no'),('EB3','  ','svt3:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt3` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_er`
--

DROP TABLE IF EXISTS `svt3_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_er` (
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
-- Dumping data for table `svt3_er`
--

LOCK TABLES `svt3_er` WRITE;
/*!40000 ALTER TABLE `svt3_er` DISABLE KEYS */;
INSERT INTO `svt3_er` VALUES ('svt3','{/usr/local/clas/clas12/coda/src/rol/rol/Linux_i686/obj/svt2.so usr} {/usr/local/clas/clas12/coda/src/rol/rol/Linux_i686/obj/urol2_tt_testsetup.so usr} ','','EB3:localhost','yes','','no'),('EB3','  ','svt3:localhost','','yes','','no'),('ER3','{CODA}  ','','coda_1','yes','','no'),('coda_1','','ER3:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt3_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_er_option`
--

DROP TABLE IF EXISTS `svt3_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt3_er_option`
--

LOCK TABLES `svt3_er_option` WRITE;
/*!40000 ALTER TABLE `svt3_er_option` DISABLE KEYS */;
INSERT INTO `svt3_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/w/stage3/BUFFER/svt/svt3er');
/*!40000 ALTER TABLE `svt3_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_er_pos`
--

DROP TABLE IF EXISTS `svt3_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt3_er_pos`
--

LOCK TABLES `svt3_er_pos` WRITE;
/*!40000 ALTER TABLE `svt3_er_pos` DISABLE KEYS */;
INSERT INTO `svt3_er_pos` VALUES ('svt3',2,1),('EB3',2,3),('ER3',4,3),('coda_1',4,5);
/*!40000 ALTER TABLE `svt3_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_er_script`
--

DROP TABLE IF EXISTS `svt3_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt3_er_script`
--

LOCK TABLES `svt3_er_script` WRITE;
/*!40000 ALTER TABLE `svt3_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt3_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_option`
--

DROP TABLE IF EXISTS `svt3_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt3_option`
--

LOCK TABLES `svt3_option` WRITE;
/*!40000 ALTER TABLE `svt3_option` DISABLE KEYS */;
INSERT INTO `svt3_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64');
/*!40000 ALTER TABLE `svt3_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_pos`
--

DROP TABLE IF EXISTS `svt3_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt3_pos`
--

LOCK TABLES `svt3_pos` WRITE;
/*!40000 ALTER TABLE `svt3_pos` DISABLE KEYS */;
INSERT INTO `svt3_pos` VALUES ('svt3',2,1),('EB3',2,3);
/*!40000 ALTER TABLE `svt3_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt3_script`
--

DROP TABLE IF EXISTS `svt3_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt3_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt3_script`
--

LOCK TABLES `svt3_script` WRITE;
/*!40000 ALTER TABLE `svt3_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt3_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4`
--

DROP TABLE IF EXISTS `svt4`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4` (
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
-- Dumping data for table `svt4`
--

LOCK TABLES `svt4` WRITE;
/*!40000 ALTER TABLE `svt4` DISABLE KEYS */;
INSERT INTO `svt4` VALUES ('svt4','{hps1.so usr} {hps2.so usr} ','','EB4:localhost','yes','','4'),('EB4','  ','svt4:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt4` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt45_er`
--

DROP TABLE IF EXISTS `svt45_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt45_er` (
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
-- Dumping data for table `svt45_er`
--

LOCK TABLES `svt45_er` WRITE;
/*!40000 ALTER TABLE `svt45_er` DISABLE KEYS */;
INSERT INTO `svt45_er` VALUES ('svt4','{hps1_master.so usr} {hps2.so usr} ','','EB4:svt4','yes','svt5','4'),('svt5','{hps1_slave.so usr} {hps2.so usr} ','','EB4:svt4','no','','5'),('EB4','  ','svt4:svt4 svt5:svt5','','yes','','no'),('ER4','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER4:svt4','','yes','','no');
/*!40000 ALTER TABLE `svt45_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt45_er_option`
--

DROP TABLE IF EXISTS `svt45_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt45_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt45_er_option`
--

LOCK TABLES `svt45_er_option` WRITE;
/*!40000 ALTER TABLE `svt45_er_option` DISABLE KEYS */;
INSERT INTO `svt45_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/w/stage3/BUFFER/svt/svt45'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000030');
/*!40000 ALTER TABLE `svt45_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt45_er_pos`
--

DROP TABLE IF EXISTS `svt45_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt45_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt45_er_pos`
--

LOCK TABLES `svt45_er_pos` WRITE;
/*!40000 ALTER TABLE `svt45_er_pos` DISABLE KEYS */;
INSERT INTO `svt45_er_pos` VALUES ('svt4',3,1),('svt5',5,1),('EB4',3,3),('ER4',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `svt45_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt45_er_script`
--

DROP TABLE IF EXISTS `svt45_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt45_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt45_er_script`
--

LOCK TABLES `svt45_er_script` WRITE;
/*!40000 ALTER TABLE `svt45_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt45_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_er`
--

DROP TABLE IF EXISTS `svt4_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_er` (
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
-- Dumping data for table `svt4_er`
--

LOCK TABLES `svt4_er` WRITE;
/*!40000 ALTER TABLE `svt4_er` DISABLE KEYS */;
INSERT INTO `svt4_er` VALUES ('svt4','{hps1.so usr} {hps2.so usr} ','','EB4:svt4','yes','','4'),('EB4','{CODA} {CODA}','svt4:svt4','','yes','','no'),('ER4','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER4:svt4','','yes','','no');
/*!40000 ALTER TABLE `svt4_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_er_option`
--

DROP TABLE IF EXISTS `svt4_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt4_er_option`
--

LOCK TABLES `svt4_er_option` WRITE;
/*!40000 ALTER TABLE `svt4_er_option` DISABLE KEYS */;
INSERT INTO `svt4_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/svt/svt4er'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000010');
/*!40000 ALTER TABLE `svt4_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_er_pos`
--

DROP TABLE IF EXISTS `svt4_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt4_er_pos`
--

LOCK TABLES `svt4_er_pos` WRITE;
/*!40000 ALTER TABLE `svt4_er_pos` DISABLE KEYS */;
INSERT INTO `svt4_er_pos` VALUES ('svt4',2,1),('EB4',2,3),('ER4',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `svt4_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_er_script`
--

DROP TABLE IF EXISTS `svt4_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt4_er_script`
--

LOCK TABLES `svt4_er_script` WRITE;
/*!40000 ALTER TABLE `svt4_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt4_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_option`
--

DROP TABLE IF EXISTS `svt4_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt4_option`
--

LOCK TABLES `svt4_option` WRITE;
/*!40000 ALTER TABLE `svt4_option` DISABLE KEYS */;
INSERT INTO `svt4_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000010'),('confFile','none');
/*!40000 ALTER TABLE `svt4_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_pos`
--

DROP TABLE IF EXISTS `svt4_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt4_pos`
--

LOCK TABLES `svt4_pos` WRITE;
/*!40000 ALTER TABLE `svt4_pos` DISABLE KEYS */;
INSERT INTO `svt4_pos` VALUES ('svt4',3,1),('EB4',2,3);
/*!40000 ALTER TABLE `svt4_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt4_script`
--

DROP TABLE IF EXISTS `svt4_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt4_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt4_script`
--

LOCK TABLES `svt4_script` WRITE;
/*!40000 ALTER TABLE `svt4_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt4_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5`
--

DROP TABLE IF EXISTS `svt5`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5` (
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
-- Dumping data for table `svt5`
--

LOCK TABLES `svt5` WRITE;
/*!40000 ALTER TABLE `svt5` DISABLE KEYS */;
INSERT INTO `svt5` VALUES ('svt5','{svt1.so us} {svt2.so usr} ','','EB5:localhost','yes','','no'),('EB5','  ','svt5:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt5` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_er`
--

DROP TABLE IF EXISTS `svt5_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_er` (
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
-- Dumping data for table `svt5_er`
--

LOCK TABLES `svt5_er` WRITE;
/*!40000 ALTER TABLE `svt5_er` DISABLE KEYS */;
INSERT INTO `svt5_er` VALUES ('svt5','{svt1.so usr} {svt2.so usr} ','','EB5:localhost','yes','','5'),('EB5','  ','svt5:localhost','','yes','','no'),('ER5','{CODA}  ','','coda_3','yes','','no'),('coda_3','  ','ER5:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt5_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_er_option`
--

DROP TABLE IF EXISTS `svt5_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt5_er_option`
--

LOCK TABLES `svt5_er_option` WRITE;
/*!40000 ALTER TABLE `svt5_er_option` DISABLE KEYS */;
INSERT INTO `svt5_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/w/stage3/BUFFER/svt/svt5er'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000020'),('confFile','none');
/*!40000 ALTER TABLE `svt5_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_er_pos`
--

DROP TABLE IF EXISTS `svt5_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt5_er_pos`
--

LOCK TABLES `svt5_er_pos` WRITE;
/*!40000 ALTER TABLE `svt5_er_pos` DISABLE KEYS */;
INSERT INTO `svt5_er_pos` VALUES ('svt5',2,1),('EB5',2,3),('ER5',4,3),('coda_3',4,5);
/*!40000 ALTER TABLE `svt5_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_er_script`
--

DROP TABLE IF EXISTS `svt5_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt5_er_script`
--

LOCK TABLES `svt5_er_script` WRITE;
/*!40000 ALTER TABLE `svt5_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt5_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_option`
--

DROP TABLE IF EXISTS `svt5_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt5_option`
--

LOCK TABLES `svt5_option` WRITE;
/*!40000 ALTER TABLE `svt5_option` DISABLE KEYS */;
INSERT INTO `svt5_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','32');
/*!40000 ALTER TABLE `svt5_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_pos`
--

DROP TABLE IF EXISTS `svt5_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt5_pos`
--

LOCK TABLES `svt5_pos` WRITE;
/*!40000 ALTER TABLE `svt5_pos` DISABLE KEYS */;
INSERT INTO `svt5_pos` VALUES ('svt5',2,1),('EB5',2,3);
/*!40000 ALTER TABLE `svt5_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt5_script`
--

DROP TABLE IF EXISTS `svt5_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt5_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt5_script`
--

LOCK TABLES `svt5_script` WRITE;
/*!40000 ALTER TABLE `svt5_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt5_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6`
--

DROP TABLE IF EXISTS `svt6`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6` (
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
-- Dumping data for table `svt6`
--

LOCK TABLES `svt6` WRITE;
/*!40000 ALTER TABLE `svt6` DISABLE KEYS */;
INSERT INTO `svt6` VALUES ('svt6','{svt1.so us} {svt2.so usr} ','','EB6:localhost','yes','','no'),('EB6','  ','svt6:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt6` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_er`
--

DROP TABLE IF EXISTS `svt6_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_er` (
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
-- Dumping data for table `svt6_er`
--

LOCK TABLES `svt6_er` WRITE;
/*!40000 ALTER TABLE `svt6_er` DISABLE KEYS */;
INSERT INTO `svt6_er` VALUES ('svt6','{svt1.so us} {svt2.so usr} ','','EB6:localhost','yes','','6'),('EB6','  ','svt6:localhost','','yes','','no'),('ER6','{CODA}  ','','coda_4','yes','','no'),('coda_4','  ','ER6:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt6_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_er_option`
--

DROP TABLE IF EXISTS `svt6_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt6_er_option`
--

LOCK TABLES `svt6_er_option` WRITE;
/*!40000 ALTER TABLE `svt6_er_option` DISABLE KEYS */;
INSERT INTO `svt6_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/w/stage3/BUFFER/svt/svt6er'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000040');
/*!40000 ALTER TABLE `svt6_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_er_pos`
--

DROP TABLE IF EXISTS `svt6_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt6_er_pos`
--

LOCK TABLES `svt6_er_pos` WRITE;
/*!40000 ALTER TABLE `svt6_er_pos` DISABLE KEYS */;
INSERT INTO `svt6_er_pos` VALUES ('svt6',2,1),('EB6',2,3),('ER6',4,3),('coda_4',4,5);
/*!40000 ALTER TABLE `svt6_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_er_script`
--

DROP TABLE IF EXISTS `svt6_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt6_er_script`
--

LOCK TABLES `svt6_er_script` WRITE;
/*!40000 ALTER TABLE `svt6_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt6_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_option`
--

DROP TABLE IF EXISTS `svt6_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt6_option`
--

LOCK TABLES `svt6_option` WRITE;
/*!40000 ALTER TABLE `svt6_option` DISABLE KEYS */;
INSERT INTO `svt6_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','64'),('confFile','none');
/*!40000 ALTER TABLE `svt6_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_pos`
--

DROP TABLE IF EXISTS `svt6_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt6_pos`
--

LOCK TABLES `svt6_pos` WRITE;
/*!40000 ALTER TABLE `svt6_pos` DISABLE KEYS */;
INSERT INTO `svt6_pos` VALUES ('svt6',2,1),('EB6',2,3);
/*!40000 ALTER TABLE `svt6_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt6_script`
--

DROP TABLE IF EXISTS `svt6_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt6_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt6_script`
--

LOCK TABLES `svt6_script` WRITE;
/*!40000 ALTER TABLE `svt6_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt6_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt7_er`
--

DROP TABLE IF EXISTS `svt7_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt7_er` (
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
-- Dumping data for table `svt7_er`
--

LOCK TABLES `svt7_er` WRITE;
/*!40000 ALTER TABLE `svt7_er` DISABLE KEYS */;
INSERT INTO `svt7_er` VALUES ('svt7','{hps1.so usr} {hps2.so usr} ','','EB7:localhost','yes','','7'),('EB7','{CODA} {CODA} ','svt7:svt7','','yes','','no'),('ER7','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER7:localhost','','yes','','no');
/*!40000 ALTER TABLE `svt7_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt7_er_option`
--

DROP TABLE IF EXISTS `svt7_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt7_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt7_er_option`
--

LOCK TABLES `svt7_er_option` WRITE;
/*!40000 ALTER TABLE `svt7_er_option` DISABLE KEYS */;
INSERT INTO `svt7_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/svt/svt7'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000080');
/*!40000 ALTER TABLE `svt7_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt7_er_pos`
--

DROP TABLE IF EXISTS `svt7_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt7_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt7_er_pos`
--

LOCK TABLES `svt7_er_pos` WRITE;
/*!40000 ALTER TABLE `svt7_er_pos` DISABLE KEYS */;
INSERT INTO `svt7_er_pos` VALUES ('svt7',3,1),('EB7',3,3),('ER7',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `svt7_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt7_er_script`
--

DROP TABLE IF EXISTS `svt7_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt7_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt7_er_script`
--

LOCK TABLES `svt7_er_script` WRITE;
/*!40000 ALTER TABLE `svt7_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `svt7_er_script` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-01-26 16:02:58
