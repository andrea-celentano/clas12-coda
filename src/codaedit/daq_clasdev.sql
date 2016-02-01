-- MySQL dump 10.11
--
-- Host: clondb1    Database: daq_clasdev
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
-- Current Database: `daq_clasdev`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `daq_clasdev` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `daq_clasdev`;

--
-- Table structure for table `ECAL`
--

DROP TABLE IF EXISTS `ECAL`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ECAL` (
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
-- Dumping data for table `ECAL`
--

LOCK TABLES `ECAL` WRITE;
/*!40000 ALTER TABLE `ECAL` DISABLE KEYS */;
INSERT INTO `ECAL` VALUES ('hps1','{/usr/local/clas/clas12/coda/src/rol/rol/Linux_i686/obj/tid1_master.so usr} {/usr/local/clas/clas12/coda/src/rol/rol/Linux_i686/obj/urol2_tt_testsetup.so usr} ','','EB9:clonusr3','yes','','1'),('hps2','{/usr/local/clas/clas12/coda/src/rol/rol/Linux_i686/obj/tid1_slave.so usr} {/usr/local/clas/clas12/coda/src/rol/rol/Linux_i686/obj/urol2_tt_testsetup.so usr} ','','EB9:clonusr3','yes','','2'),('EB9','{CODA} {CODA} ','hps1:hps1 hps2:hps2','','yes','','no'),('ER9','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER9:clonusr3','','yes','','no');
/*!40000 ALTER TABLE `ECAL` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ECAL_option`
--

DROP TABLE IF EXISTS `ECAL_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ECAL_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ECAL_option`
--

LOCK TABLES `ECAL_option` WRITE;
/*!40000 ALTER TABLE `ECAL_option` DISABLE KEYS */;
INSERT INTO `ECAL_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/hps/ecal'),('SPLITMB','2047'),('rocMask','6'),('confFile','none');
/*!40000 ALTER TABLE `ECAL_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ECAL_pos`
--

DROP TABLE IF EXISTS `ECAL_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ECAL_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ECAL_pos`
--

LOCK TABLES `ECAL_pos` WRITE;
/*!40000 ALTER TABLE `ECAL_pos` DISABLE KEYS */;
INSERT INTO `ECAL_pos` VALUES ('hps1',3,1),('hps2',5,1),('EB9',3,3),('ER9',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `ECAL_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ECAL_script`
--

DROP TABLE IF EXISTS `ECAL_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ECAL_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ECAL_script`
--

LOCK TABLES `ECAL_script` WRITE;
/*!40000 ALTER TABLE `ECAL_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ECAL_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT`
--

DROP TABLE IF EXISTS `FT`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT` (
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
-- Dumping data for table `FT`
--

LOCK TABLES `FT` WRITE;
/*!40000 ALTER TABLE `FT` DISABLE KEYS */;
INSERT INTO `FT` VALUES ('adcft1','{hps1_master.so usr} {hps2.so usr} ','','EB81:jlab12daq1','yes','adcft2','70'),('adcft2','{hps1_slave.so usr} {hps2.so usr} ','','EB81:jlab12daq1','no','','71'),('EB81','{CODA} {CODA} ','adcft1:adcft1 adcft2:adcft2','ET81:jlab12daq1','yes','','no'),('ER81','{CODA}  ','ET81:jlab12daq1','coda_0','yes','','no'),('ET81','{1000} {500000} ','EB81:jlab12daq1','ER81:jlab12daq1','yes','','no'),('coda_0','{/work/ft/run_jlabftcal} {CODA} ','ER81:jlab12daq1','','yes','','no');
/*!40000 ALTER TABLE `FT` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_NOER`
--

DROP TABLE IF EXISTS `FT_NOER`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_NOER` (
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
-- Dumping data for table `FT_NOER`
--

LOCK TABLES `FT_NOER` WRITE;
/*!40000 ALTER TABLE `FT_NOER` DISABLE KEYS */;
INSERT INTO `FT_NOER` VALUES ('adcft1','{hps1_master.so usr} {hps2.so usr} ','','EB81:jlab12daq1','yes','adcft2','70'),('adcft2','{hps1_slave.so usr} {hps2.so usr} ','','EB81:jlab12daq1','no','','71'),('EB81','{CODA} {CODA} ','adcft1:adcft1 adcft2:adcft2','ET81:jlab12daq1','yes','','no'),('ET81','{1000} {500000} ','EB81:jlab12daq1','','yes','','no');
/*!40000 ALTER TABLE `FT_NOER` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_NOER_option`
--

DROP TABLE IF EXISTS `FT_NOER_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_NOER_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_NOER_option`
--

LOCK TABLES `FT_NOER_option` WRITE;
/*!40000 ALTER TABLE `FT_NOER_option` DISABLE KEYS */;
INSERT INTO `FT_NOER_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x000000c0 0x00000000 0x00000000'),('dataFile','1000');
/*!40000 ALTER TABLE `FT_NOER_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_NOER_pos`
--

DROP TABLE IF EXISTS `FT_NOER_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_NOER_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_NOER_pos`
--

LOCK TABLES `FT_NOER_pos` WRITE;
/*!40000 ALTER TABLE `FT_NOER_pos` DISABLE KEYS */;
INSERT INTO `FT_NOER_pos` VALUES ('adcft1',2,1),('adcft2',4,1),('EB81',2,3),('ET81',2,5);
/*!40000 ALTER TABLE `FT_NOER_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_NOER_script`
--

DROP TABLE IF EXISTS `FT_NOER_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_NOER_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_NOER_script`
--

LOCK TABLES `FT_NOER_script` WRITE;
/*!40000 ALTER TABLE `FT_NOER_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `FT_NOER_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_option`
--

DROP TABLE IF EXISTS `FT_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_option`
--

LOCK TABLES `FT_option` WRITE;
/*!40000 ALTER TABLE `FT_option` DISABLE KEYS */;
INSERT INTO `FT_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ft/run_jlabftcal'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x000000c0 0x00000000 0x00000000');
/*!40000 ALTER TABLE `FT_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_pos`
--

DROP TABLE IF EXISTS `FT_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_pos`
--

LOCK TABLES `FT_pos` WRITE;
/*!40000 ALTER TABLE `FT_pos` DISABLE KEYS */;
INSERT INTO `FT_pos` VALUES ('adcft1',2,1),('adcft2',4,1),('EB81',2,3),('ER81',4,3),('ET81',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `FT_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_script`
--

DROP TABLE IF EXISTS `FT_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_script`
--

LOCK TABLES `FT_script` WRITE;
/*!40000 ALTER TABLE `FT_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `FT_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_test`
--

DROP TABLE IF EXISTS `FT_test`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_test` (
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
-- Dumping data for table `FT_test`
--

LOCK TABLES `FT_test` WRITE;
/*!40000 ALTER TABLE `FT_test` DISABLE KEYS */;
INSERT INTO `FT_test` VALUES ('adcft1','{hps1.so usr} {hps2.so usr} ','','EB78:adcft1','yes','','70'),('EB78','{CODA} {CODA} ','adcft1:adcft1','ET78:adcft1','yes','','no'),('ER78','{CODA}  ','ET78:adcft1','coda_0','yes','','no'),('ET78','{1000} {50000} ','EB78:adcft1','ER78:adcft1','yes','','no'),('coda_0','{/work/ft/run_jlabftcal} {CODA} ','ER78:adcft1','','yes','','no');
/*!40000 ALTER TABLE `FT_test` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_test_option`
--

DROP TABLE IF EXISTS `FT_test_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_test_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_test_option`
--

LOCK TABLES `FT_test_option` WRITE;
/*!40000 ALTER TABLE `FT_test_option` DISABLE KEYS */;
INSERT INTO `FT_test_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ft/run_jlabftcal'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000040 0x00000000 0x00000000');
/*!40000 ALTER TABLE `FT_test_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_test_pos`
--

DROP TABLE IF EXISTS `FT_test_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_test_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_test_pos`
--

LOCK TABLES `FT_test_pos` WRITE;
/*!40000 ALTER TABLE `FT_test_pos` DISABLE KEYS */;
INSERT INTO `FT_test_pos` VALUES ('adcft1',2,1),('EB78',2,3),('ER78',4,3),('ET78',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `FT_test_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `FT_test_script`
--

DROP TABLE IF EXISTS `FT_test_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `FT_test_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `FT_test_script`
--

LOCK TABLES `FT_test_script` WRITE;
/*!40000 ALTER TABLE `FT_test_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `FT_test_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3`
--

DROP TABLE IF EXISTS `HPS3`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3` (
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
-- Dumping data for table `HPS3`
--

LOCK TABLES `HPS3` WRITE;
/*!40000 ALTER TABLE `HPS3` DISABLE KEYS */;
INSERT INTO `HPS3` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB31:clondaq3','yes','','46'),('hps1','{hps1_slave.so usr} {hps2.so usr} ','','EB31:clondaq3','yes','hps2','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB31:clondaq3','no','dpm0','39'),('hps1gtp','{gtp1.so usr}  ','','','no','dpm0','no'),('hps2gtp','{gtp1.so usr}  ','','','no','dpm0','no'),('dpm0','{dpm.so usr}  ','','EB31:clondaq3','no','dpm1','51'),('dpm1','{dpm.so usr}  ','','EB31:clondaq3','no','dpm2','52'),('dpm2','{dpm.so usr}  ','','EB31:clondaq3','no','dpm3','53'),('dpm3','{dpm.so usr}  ','','EB31:clondaq3','no','dpm4','54'),('dpm4','{dpm.so usr}  ','','EB31:clondaq3','no','dpm5','55'),('dpm5','{dpm.so usr}  ','','EB31:clondaq3','no','dpm6','56'),('dpm6','{dpm.so usr}  ','','EB31:clondaq3','no','dpm7','57'),('dpm7','{dpm.so usr}  ','','EB31:clondaq3','no','dpm8','66'),('dpm8','{dpm.so usr}  ','','EB31:clondaq3','no','dpm9','59'),('dpm9','{dpm.so usr}  ','','EB31:clondaq3','no','dpm10','60'),('dpm10','{dpm.so usr}  ','','EB31:clondaq3','no','dpm11','61'),('dpm11','{dpm.so usr}  ','','EB31:clondaq3','no','dpm12','62'),('dpm12','{dpm.so usr}  ','','EB31:clondaq3','no','dpm13','63'),('dpm13','{dpm.so usr}  ','','EB31:clondaq3','no','dpm14','64'),('dpm14','{dpm.so usr}  ','','EB31:clondaq3','no','','65'),('dtm0','{dtm.so usr}  ','','','no','','no'),('dtm1','{dtm.so usr}  ','','','no','','no'),('EB31','{CODA} {CODA} ','hps11:hps11 hps1:hps1 hps2:hps2 dpm0:dpm0 dpm1:dpm1 dpm2:dpm2 dpm3:dpm3 dpm4:dpm4 dpm5:dpm5 dpm6:dpm6 dpm7:dpm7 dpm8:dpm8 dpm9:dpm9 dpm10:dpm10 dpm11:dpm11 dpm12:dpm12 dpm13:dpm13 dpm14:dpm14','','yes','','no'),('ER31','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER31:clondaq3','','yes','','no');
/*!40000 ALTER TABLE `HPS3` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3LED`
--

DROP TABLE IF EXISTS `HPS3LED`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3LED` (
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
-- Dumping data for table `HPS3LED`
--

LOCK TABLES `HPS3LED` WRITE;
/*!40000 ALTER TABLE `HPS3LED` DISABLE KEYS */;
INSERT INTO `HPS3LED` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB31:clondaq3','yes','','46'),('hps1','{hps1_slave.so usr} {hps2.so usr} ','','EB31:clondaq3','yes','hps2','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB31:clondaq3','no','','39'),('hps1gtp','{gtp1.so usr}  ','','','no','','no'),('hps2gtp','{gtp1.so usr}  ','','','no','','no'),('ET31','{1000} {200000} ','','','yes','','no'),('EB31','{CODA} {CODA} ','hps11:hps11 hps1:hps1 hps2:hps2','','yes','','no'),('ER31','{CODA}  ','','coda_0','yes','','no'),('coda_0','{/data/hps/hpsecal} {CODA} ','ER31:clondaq3','','yes','','no');
/*!40000 ALTER TABLE `HPS3LED` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3LED_option`
--

DROP TABLE IF EXISTS `HPS3LED_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3LED_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS3LED_option`
--

LOCK TABLES `HPS3LED_option` WRITE;
/*!40000 ALTER TABLE `HPS3LED_option` DISABLE KEYS */;
INSERT INTO `HPS3LED_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/data/hps/hpsecal'),('confFile','/usr/clas12/release/0.3/parms/trigger/HPS/ECAL/HPS_ECAL_LED_RUN.cnf'),('rocMask','0x00000000 0x00000000 0x000040a0 0x00000000');
/*!40000 ALTER TABLE `HPS3LED_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3LED_pos`
--

DROP TABLE IF EXISTS `HPS3LED_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3LED_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS3LED_pos`
--

LOCK TABLES `HPS3LED_pos` WRITE;
/*!40000 ALTER TABLE `HPS3LED_pos` DISABLE KEYS */;
INSERT INTO `HPS3LED_pos` VALUES ('hps11',1,1),('hps1',2,1),('hps2',3,1),('hps1gtp',6,1),('hps2gtp',7,1),('ET31',4,3),('EB31',6,3),('ER31',8,3),('coda_0',8,5);
/*!40000 ALTER TABLE `HPS3LED_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3LED_script`
--

DROP TABLE IF EXISTS `HPS3LED_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3LED_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS3LED_script`
--

LOCK TABLES `HPS3LED_script` WRITE;
/*!40000 ALTER TABLE `HPS3LED_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPS3LED_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3_option`
--

DROP TABLE IF EXISTS `HPS3_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS3_option`
--

LOCK TABLES `HPS3_option` WRITE;
/*!40000 ALTER TABLE `HPS3_option` DISABLE KEYS */;
INSERT INTO `HPS3_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile',''),('confFile','/usr/clas12/release/0.3/parms/trigger/HPS/TEST/Crash2-charge-injection.cnf'),('rocMask','0x00000000 0x00000007 0xfbf840a0 0x00000000');
/*!40000 ALTER TABLE `HPS3_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3_pos`
--

DROP TABLE IF EXISTS `HPS3_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS3_pos`
--

LOCK TABLES `HPS3_pos` WRITE;
/*!40000 ALTER TABLE `HPS3_pos` DISABLE KEYS */;
INSERT INTO `HPS3_pos` VALUES ('hps11',1,1),('hps1',2,1),('hps2',3,1),('hps1gtp',6,1),('hps2gtp',7,1),('dpm0',9,1),('dpm1',10,1),('dpm2',11,1),('dpm3',12,1),('dpm4',13,1),('dpm5',14,1),('dpm6',15,1),('dpm7',16,1),('dpm8',17,1),('dpm9',18,1),('dpm10',19,1),('dpm11',20,1),('dpm12',21,1),('dpm13',22,1),('dpm14',23,1),('dtm0',25,1),('dtm1',26,1),('EB31',12,3),('ER31',14,3),('coda_0',14,5);
/*!40000 ALTER TABLE `HPS3_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS3_script`
--

DROP TABLE IF EXISTS `HPS3_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS3_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS3_script`
--

LOCK TABLES `HPS3_script` WRITE;
/*!40000 ALTER TABLE `HPS3_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPS3_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL`
--

DROP TABLE IF EXISTS `HPSECAL`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL` (
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
-- Dumping data for table `HPSECAL`
--

LOCK TABLES `HPSECAL` WRITE;
/*!40000 ALTER TABLE `HPSECAL` DISABLE KEYS */;
INSERT INTO `HPSECAL` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB20:clondaq5','yes','','46'),('hps1','{hps1_slave.so usr} {hps2.so usr} ','','EB20:clondaq5','yes','hps2','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB20:clondaq5','no','hps12','39'),('hps1gtp','{gtp1.so usr} {hps2.so usr} ','','','no','hps12','no'),('hps2gtp','{gtp1.so usr} {hps2.so usr} ','','','no','hps12','no'),('hps12','{hps1_slave.so usr} {hps2.so usr} ','','EB20:clondaq5','no','','58'),('EB20','{CODA} {CODA} ','hps11:hps11 hps1:hps1 hps2:hps2 hps12:hps12','','yes','','no'),('ER20','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER20:clondaq5','','yes','','no');
/*!40000 ALTER TABLE `HPSECAL` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_NOER`
--

DROP TABLE IF EXISTS `HPSECAL_NOER`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_NOER` (
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
-- Dumping data for table `HPSECAL_NOER`
--

LOCK TABLES `HPSECAL_NOER` WRITE;
/*!40000 ALTER TABLE `HPSECAL_NOER` DISABLE KEYS */;
INSERT INTO `HPSECAL_NOER` VALUES ('hps11','{hps1_master.so usr} {hps2.so usr} ','','EB20:clondaq5','yes','','46'),('hps1','{hps1_slave.so usr} {hps2.so usr} ','','EB20:clondaq5','yes','hps2','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB20:clondaq5','no','','39'),('hps1gtp','{gtp1.so usr}  ','','','no','','no'),('hps2gtp','{gtp1.so usr}  ','','','no','','no'),('EB20','{CODA} {CODA} ','hps11:hps11 hps1:hps1 hps2:hps2','','yes','','no'),('ET20','  ','','','yes','','no');
/*!40000 ALTER TABLE `HPSECAL_NOER` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_NOER_option`
--

DROP TABLE IF EXISTS `HPSECAL_NOER_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_NOER_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPSECAL_NOER_option`
--

LOCK TABLES `HPSECAL_NOER_option` WRITE;
/*!40000 ALTER TABLE `HPSECAL_NOER_option` DISABLE KEYS */;
INSERT INTO `HPSECAL_NOER_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','/usr/clas12/release/0.3/parms/trigger/HPS/ECAL/HPS_ECAL_LED_RUN.cnf'),('rocMask','0x00000000 0x00000000 0x000040a0 0x00000000');
/*!40000 ALTER TABLE `HPSECAL_NOER_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_NOER_pos`
--

DROP TABLE IF EXISTS `HPSECAL_NOER_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_NOER_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPSECAL_NOER_pos`
--

LOCK TABLES `HPSECAL_NOER_pos` WRITE;
/*!40000 ALTER TABLE `HPSECAL_NOER_pos` DISABLE KEYS */;
INSERT INTO `HPSECAL_NOER_pos` VALUES ('hps11',1,1),('hps1',3,1),('hps2',5,1),('hps1gtp',7,1),('hps2gtp',9,1),('EB20',3,3),('ET20',5,3);
/*!40000 ALTER TABLE `HPSECAL_NOER_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_NOER_script`
--

DROP TABLE IF EXISTS `HPSECAL_NOER_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_NOER_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPSECAL_NOER_script`
--

LOCK TABLES `HPSECAL_NOER_script` WRITE;
/*!40000 ALTER TABLE `HPSECAL_NOER_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPSECAL_NOER_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_option`
--

DROP TABLE IF EXISTS `HPSECAL_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPSECAL_option`
--

LOCK TABLES `HPSECAL_option` WRITE;
/*!40000 ALTER TABLE `HPSECAL_option` DISABLE KEYS */;
INSERT INTO `HPSECAL_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/data/hps/hpsecal'),('confFile','/usr/local/clas12/release/0.2/parms/trigger/HPS/ECAL/hps_cosmic.cnf'),('rocMask','0x00000000 0x00000000 0x040040a0 0x00000000');
/*!40000 ALTER TABLE `HPSECAL_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_pos`
--

DROP TABLE IF EXISTS `HPSECAL_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPSECAL_pos`
--

LOCK TABLES `HPSECAL_pos` WRITE;
/*!40000 ALTER TABLE `HPSECAL_pos` DISABLE KEYS */;
INSERT INTO `HPSECAL_pos` VALUES ('hps11',1,1),('hps1',3,1),('hps2',5,1),('hps1gtp',7,1),('hps2gtp',9,1),('hps12',11,1),('EB20',3,3),('ER20',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `HPSECAL_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPSECAL_script`
--

DROP TABLE IF EXISTS `HPSECAL_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPSECAL_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPSECAL_script`
--

LOCK TABLES `HPSECAL_script` WRITE;
/*!40000 ALTER TABLE `HPSECAL_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `HPSECAL_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `HPS_NOER_NOTDC_64_option`
--

DROP TABLE IF EXISTS `HPS_NOER_NOTDC_64_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `HPS_NOER_NOTDC_64_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `HPS_NOER_NOTDC_64_option`
--

LOCK TABLES `HPS_NOER_NOTDC_64_option` WRITE;
/*!40000 ALTER TABLE `HPS_NOER_NOTDC_64_option` DISABLE KEYS */;
INSERT INTO `HPS_NOER_NOTDC_64_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64');
/*!40000 ALTER TABLE `HPS_NOER_NOTDC_64_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MMFT`
--

DROP TABLE IF EXISTS `MMFT`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MMFT` (
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
-- Dumping data for table `MMFT`
--

LOCK TABLES `MMFT` WRITE;
/*!40000 ALTER TABLE `MMFT` DISABLE KEYS */;
INSERT INTO `MMFT` VALUES ('mmft1','{hps1mvt.so usr} {hps2mvtcomposite.so usr} ','','EB79:mmft1','yes','','73'),('ER79','{CODA}  ','ET79:mmft1','coda_0','yes','','no'),('EB79','{CODA} {CODA} ','mmft1:mmft1','ET79:mmft1','yes','','no'),('coda_0','{/work/mvt/mmft1} {CODA} ','ER79:mmft1','','yes','','no'),('ET79','{1000} {300000} ','EB79:mmft1','ER79:mmft1','yes','','no');
/*!40000 ALTER TABLE `MMFT` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MMFT_option`
--

DROP TABLE IF EXISTS `MMFT_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MMFT_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MMFT_option`
--

LOCK TABLES `MMFT_option` WRITE;
/*!40000 ALTER TABLE `MMFT_option` DISABLE KEYS */;
INSERT INTO `MMFT_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/mvt/mmft1'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000200 0x00000000 0x00000000');
/*!40000 ALTER TABLE `MMFT_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MMFT_pos`
--

DROP TABLE IF EXISTS `MMFT_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MMFT_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MMFT_pos`
--

LOCK TABLES `MMFT_pos` WRITE;
/*!40000 ALTER TABLE `MMFT_pos` DISABLE KEYS */;
INSERT INTO `MMFT_pos` VALUES ('mmft1',1,1),('ER79',4,2),('EB79',1,3),('coda_0',4,4),('ET79',1,5);
/*!40000 ALTER TABLE `MMFT_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `MMFT_script`
--

DROP TABLE IF EXISTS `MMFT_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MMFT_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `MMFT_script`
--

LOCK TABLES `MMFT_script` WRITE;
/*!40000 ALTER TABLE `MMFT_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `MMFT_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1`
--

DROP TABLE IF EXISTS `adcecal1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1` (
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
-- Dumping data for table `adcecal1`
--

LOCK TABLES `adcecal1` WRITE;
/*!40000 ALTER TABLE `adcecal1` DISABLE KEYS */;
INSERT INTO `adcecal1` VALUES ('adcecal1','{fadc1.so usr} {fadc2.so usr} ','','EB1:adcecal1','yes','','8'),('EB1','{CODA} {CODA} ','adcecal1:adcecal1','','yes','','no');
/*!40000 ALTER TABLE `adcecal1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_er`
--

DROP TABLE IF EXISTS `adcecal1_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_er` (
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
-- Dumping data for table `adcecal1_er`
--

LOCK TABLES `adcecal1_er` WRITE;
/*!40000 ALTER TABLE `adcecal1_er` DISABLE KEYS */;
INSERT INTO `adcecal1_er` VALUES ('adcecal1','{fadc1.so usr} {fadc2.so usr} ','','EB1:adcecal1','yes','','8'),('ER1','{CODA}  ','','coda_0','yes','','no'),('EB1','{CODA} {CODA} ','adcecal1:adcecal1','','yes','','no'),('coda_0','{test.dat} {CODA} ','ER1:adcecal1','','yes','','no');
/*!40000 ALTER TABLE `adcecal1_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_er_option`
--

DROP TABLE IF EXISTS `adcecal1_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal1_er_option`
--

LOCK TABLES `adcecal1_er_option` WRITE;
/*!40000 ALTER TABLE `adcecal1_er_option` DISABLE KEYS */;
INSERT INTO `adcecal1_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','test.dat'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000100'),('confFile','/usr/local/clas12/release/0.1/parms/trigger/clasdev.cnf');
/*!40000 ALTER TABLE `adcecal1_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_er_pos`
--

DROP TABLE IF EXISTS `adcecal1_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal1_er_pos`
--

LOCK TABLES `adcecal1_er_pos` WRITE;
/*!40000 ALTER TABLE `adcecal1_er_pos` DISABLE KEYS */;
INSERT INTO `adcecal1_er_pos` VALUES ('adcecal1',2,1),('ER1',4,1),('EB1',2,3),('coda_0',4,3);
/*!40000 ALTER TABLE `adcecal1_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_er_script`
--

DROP TABLE IF EXISTS `adcecal1_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal1_er_script`
--

LOCK TABLES `adcecal1_er_script` WRITE;
/*!40000 ALTER TABLE `adcecal1_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `adcecal1_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_option`
--

DROP TABLE IF EXISTS `adcecal1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal1_option`
--

LOCK TABLES `adcecal1_option` WRITE;
/*!40000 ALTER TABLE `adcecal1_option` DISABLE KEYS */;
INSERT INTO `adcecal1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/adcecal1/adcecal1'),('rocMask','0x00000000 0x00000000 0x00000000 0x00000100'),('confFile','none');
/*!40000 ALTER TABLE `adcecal1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_pos`
--

DROP TABLE IF EXISTS `adcecal1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal1_pos`
--

LOCK TABLES `adcecal1_pos` WRITE;
/*!40000 ALTER TABLE `adcecal1_pos` DISABLE KEYS */;
INSERT INTO `adcecal1_pos` VALUES ('adcecal1',2,1),('EB1',2,3);
/*!40000 ALTER TABLE `adcecal1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal1_script`
--

DROP TABLE IF EXISTS `adcecal1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal1_script`
--

LOCK TABLES `adcecal1_script` WRITE;
/*!40000 ALTER TABLE `adcecal1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `adcecal1_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal5`
--

DROP TABLE IF EXISTS `adcecal5`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal5` (
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
-- Dumping data for table `adcecal5`
--

LOCK TABLES `adcecal5` WRITE;
/*!40000 ALTER TABLE `adcecal5` DISABLE KEYS */;
INSERT INTO `adcecal5` VALUES ('adcecal5','{fadc1.so usr} {fadc2.so usr} ','','EB15:adcecal5','yes','','25'),('EB15','{CODA} {CODA} ','adcecal5:adcecal5','','yes','','no');
/*!40000 ALTER TABLE `adcecal5` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal5_option`
--

DROP TABLE IF EXISTS `adcecal5_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal5_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal5_option`
--

LOCK TABLES `adcecal5_option` WRITE;
/*!40000 ALTER TABLE `adcecal5_option` DISABLE KEYS */;
INSERT INTO `adcecal5_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000000 0x02000000');
/*!40000 ALTER TABLE `adcecal5_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal5_pos`
--

DROP TABLE IF EXISTS `adcecal5_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal5_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal5_pos`
--

LOCK TABLES `adcecal5_pos` WRITE;
/*!40000 ALTER TABLE `adcecal5_pos` DISABLE KEYS */;
INSERT INTO `adcecal5_pos` VALUES ('adcecal5',1,1),('EB15',1,3);
/*!40000 ALTER TABLE `adcecal5_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcecal5_script`
--

DROP TABLE IF EXISTS `adcecal5_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcecal5_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcecal5_script`
--

LOCK TABLES `adcecal5_script` WRITE;
/*!40000 ALTER TABLE `adcecal5_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `adcecal5_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft1_test`
--

DROP TABLE IF EXISTS `adcft1_test`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft1_test` (
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
-- Dumping data for table `adcft1_test`
--

LOCK TABLES `adcft1_test` WRITE;
/*!40000 ALTER TABLE `adcft1_test` DISABLE KEYS */;
INSERT INTO `adcft1_test` VALUES ('adcft1','{hps1.so usr} {hps2.so usr} ','','EB78:adcft1','yes','','70'),('EB78','{CODA} {CODA} ','adcft1:adcft1','ET78:adcft1','yes','','no'),('ER78','{CODA}  ','ET78:adcft1','coda_0','yes','','no'),('ET78','{1000} {50000} ','EB78:adcft1','ER78:adcft1','yes','','no'),('coda_0','{/work/ft/run_jlabftcal} {CODA} ','ER78:adcft1','','yes','','no');
/*!40000 ALTER TABLE `adcft1_test` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft1_test_option`
--

DROP TABLE IF EXISTS `adcft1_test_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft1_test_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft1_test_option`
--

LOCK TABLES `adcft1_test_option` WRITE;
/*!40000 ALTER TABLE `adcft1_test_option` DISABLE KEYS */;
INSERT INTO `adcft1_test_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ft/run_jlabftcal'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000040 0x00000000 0x00000000');
/*!40000 ALTER TABLE `adcft1_test_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft1_test_pos`
--

DROP TABLE IF EXISTS `adcft1_test_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft1_test_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft1_test_pos`
--

LOCK TABLES `adcft1_test_pos` WRITE;
/*!40000 ALTER TABLE `adcft1_test_pos` DISABLE KEYS */;
INSERT INTO `adcft1_test_pos` VALUES ('adcft1',2,1),('EB78',2,3),('ER78',4,3),('ET78',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `adcft1_test_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft1_test_script`
--

DROP TABLE IF EXISTS `adcft1_test_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft1_test_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft1_test_script`
--

LOCK TABLES `adcft1_test_script` WRITE;
/*!40000 ALTER TABLE `adcft1_test_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `adcft1_test_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft2_test`
--

DROP TABLE IF EXISTS `adcft2_test`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft2_test` (
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
-- Dumping data for table `adcft2_test`
--

LOCK TABLES `adcft2_test` WRITE;
/*!40000 ALTER TABLE `adcft2_test` DISABLE KEYS */;
INSERT INTO `adcft2_test` VALUES ('adcft2','{hps1.so usr} {hps2.so usr} ','','EB80:adcft2','yes','','71'),('EB80','{CODA} {CODA} ','adcft2:adcft2','ET80:adcft2','yes','','no'),('ER80','{CODA}  ','ET80:adcft2','coda_0','yes','','no'),('ET80','{1000} {50000} ','EB80:adcft2','ER80:adcft2','yes','','no'),('coda_0','{/wotk/ft/run_jlabftcal} {CODA} ','ER80:adcft2','','yes','','no');
/*!40000 ALTER TABLE `adcft2_test` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft2_test_option`
--

DROP TABLE IF EXISTS `adcft2_test_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft2_test_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft2_test_option`
--

LOCK TABLES `adcft2_test_option` WRITE;
/*!40000 ALTER TABLE `adcft2_test_option` DISABLE KEYS */;
INSERT INTO `adcft2_test_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/wotk/ft/run_jlabftcal'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000080 0x00000000 0x00000000');
/*!40000 ALTER TABLE `adcft2_test_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft2_test_pos`
--

DROP TABLE IF EXISTS `adcft2_test_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft2_test_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft2_test_pos`
--

LOCK TABLES `adcft2_test_pos` WRITE;
/*!40000 ALTER TABLE `adcft2_test_pos` DISABLE KEYS */;
INSERT INTO `adcft2_test_pos` VALUES ('adcft2',2,1),('EB80',2,3),('ER80',4,3),('ET80',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `adcft2_test_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft2_test_script`
--

DROP TABLE IF EXISTS `adcft2_test_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft2_test_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft2_test_script`
--

LOCK TABLES `adcft2_test_script` WRITE;
/*!40000 ALTER TABLE `adcft2_test_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `adcft2_test_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft3_test`
--

DROP TABLE IF EXISTS `adcft3_test`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft3_test` (
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
-- Dumping data for table `adcft3_test`
--

LOCK TABLES `adcft3_test` WRITE;
/*!40000 ALTER TABLE `adcft3_test` DISABLE KEYS */;
INSERT INTO `adcft3_test` VALUES ('adcft3','{hps1.so usr} {hps2.so usr} ','','EB82:adcft3','yes','','72'),('EB82','{CODA} {CODA} ','adcft3:adcft3','ET82:adcft3','yes','','no'),('ER82','{CODA}  ','ET82:adcft3','coda_1','yes','','no'),('ET82','{500} {200000} ','EB82:adcft3','ER82:adcft3','yes','','no'),('coda_1','{/work/ft/run_jlabfthodo} {CODA} ','ER82:adcft3','','yes','','no');
/*!40000 ALTER TABLE `adcft3_test` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft3_test_option`
--

DROP TABLE IF EXISTS `adcft3_test_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft3_test_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft3_test_option`
--

LOCK TABLES `adcft3_test_option` WRITE;
/*!40000 ALTER TABLE `adcft3_test_option` DISABLE KEYS */;
INSERT INTO `adcft3_test_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ft/run_jlabfthodo'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000100 0x00000000 0x00000000');
/*!40000 ALTER TABLE `adcft3_test_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft3_test_pos`
--

DROP TABLE IF EXISTS `adcft3_test_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft3_test_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft3_test_pos`
--

LOCK TABLES `adcft3_test_pos` WRITE;
/*!40000 ALTER TABLE `adcft3_test_pos` DISABLE KEYS */;
INSERT INTO `adcft3_test_pos` VALUES ('adcft3',2,1),('EB82',2,3),('ER82',4,3),('ET82',2,5),('coda_1',4,5);
/*!40000 ALTER TABLE `adcft3_test_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `adcft3_test_script`
--

DROP TABLE IF EXISTS `adcft3_test_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `adcft3_test_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `adcft3_test_script`
--

LOCK TABLES `adcft3_test_script` WRITE;
/*!40000 ALTER TABLE `adcft3_test_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `adcft3_test_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `croctest10_option`
--

DROP TABLE IF EXISTS `croctest10_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `croctest10_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `croctest10_option`
--

LOCK TABLES `croctest10_option` WRITE;
/*!40000 ALTER TABLE `croctest10_option` DISABLE KEYS */;
INSERT INTO `croctest10_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','134217728');
/*!40000 ALTER TABLE `croctest10_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `croctest3_option`
--

DROP TABLE IF EXISTS `croctest3_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `croctest3_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `croctest3_option`
--

LOCK TABLES `croctest3_option` WRITE;
/*!40000 ALTER TABLE `croctest3_option` DISABLE KEYS */;
INSERT INTO `croctest3_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','671088640');
/*!40000 ALTER TABLE `croctest3_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctof1`
--

DROP TABLE IF EXISTS `ctof1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctof1` (
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
-- Dumping data for table `ctof1`
--

LOCK TABLES `ctof1` WRITE;
/*!40000 ALTER TABLE `ctof1` DISABLE KEYS */;
INSERT INTO `ctof1` VALUES ('ctof1','{hps1.so usr} {hps2.so usr} ','','EB66:ctof1','yes','','68'),('EB66','{CODA} {CODA} ','ctof1:ctof1','ET66:ctof1','yes','','no'),('ER66','{CODA}  ','ET66:ctof1','coda_2','yes','','no'),('ET66','{500} {100000} ','EB66:ctof1','ER66:ctof1','yes','','no'),('coda_2','{/work/ctof/ctof1} {CODA} ','ER66:ctof1','','yes','','no');
/*!40000 ALTER TABLE `ctof1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctof1_option`
--

DROP TABLE IF EXISTS `ctof1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctof1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctof1_option`
--

LOCK TABLES `ctof1_option` WRITE;
/*!40000 ALTER TABLE `ctof1_option` DISABLE KEYS */;
INSERT INTO `ctof1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ctof/ctof1'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf'),('rocMask','0x00000000 0x00000010 0x00000000 0x00000000');
/*!40000 ALTER TABLE `ctof1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctof1_pos`
--

DROP TABLE IF EXISTS `ctof1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctof1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctof1_pos`
--

LOCK TABLES `ctof1_pos` WRITE;
/*!40000 ALTER TABLE `ctof1_pos` DISABLE KEYS */;
INSERT INTO `ctof1_pos` VALUES ('ctof1',3,1),('EB66',3,3),('ER66',7,3),('ET66',3,5),('coda_2',7,5);
/*!40000 ALTER TABLE `ctof1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctof1_script`
--

DROP TABLE IF EXISTS `ctof1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctof1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctof1_script`
--

LOCK TABLES `ctof1_script` WRITE;
/*!40000 ALTER TABLE `ctof1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ctof1_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctofpc1`
--

DROP TABLE IF EXISTS `ctofpc1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctofpc1` (
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
-- Dumping data for table `ctofpc1`
--

LOCK TABLES `ctofpc1` WRITE;
/*!40000 ALTER TABLE `ctofpc1` DISABLE KEYS */;
INSERT INTO `ctofpc1` VALUES ('croctest5','{/usr/local/clas/devel/coda/src/rol/rol/Linux_i686/obj/urol1_pci.so usr} {/usr/local/clas/devel/coda/src/rol/rol/Linux_i686/obj/urol2.so usr} ','','EB5:clon00','yes','','30'),('EB5','{BOS} {BOS} ','croctest5:croctest5','','yes','','no');
/*!40000 ALTER TABLE `ctofpc1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctofpc1_option`
--

DROP TABLE IF EXISTS `ctofpc1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctofpc1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctofpc1_option`
--

LOCK TABLES `ctofpc1_option` WRITE;
/*!40000 ALTER TABLE `ctofpc1_option` DISABLE KEYS */;
INSERT INTO `ctofpc1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','1073741824'),('dataFile','/usr/local/clas/devel/coda/src/rol/rol/Linux_i686/obj/urol1_pci.so usr');
/*!40000 ALTER TABLE `ctofpc1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctofpc1_pos`
--

DROP TABLE IF EXISTS `ctofpc1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctofpc1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctofpc1_pos`
--

LOCK TABLES `ctofpc1_pos` WRITE;
/*!40000 ALTER TABLE `ctofpc1_pos` DISABLE KEYS */;
INSERT INTO `ctofpc1_pos` VALUES ('croctest5',3,1),('EB5',3,3);
/*!40000 ALTER TABLE `ctofpc1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctofpc1_script`
--

DROP TABLE IF EXISTS `ctofpc1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctofpc1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctofpc1_script`
--

LOCK TABLES `ctofpc1_script` WRITE;
/*!40000 ALTER TABLE `ctofpc1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ctofpc1_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctoftest`
--

DROP TABLE IF EXISTS `ctoftest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctoftest` (
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
-- Dumping data for table `ctoftest`
--

LOCK TABLES `ctoftest` WRITE;
/*!40000 ALTER TABLE `ctoftest` DISABLE KEYS */;
INSERT INTO `ctoftest` VALUES ('ctoftest1','{$CODA/VXWORKS_ppc/rol/ctoftest1.o usr} {$CODA/VXWORKS_ppc/rol/rol2_tt.o usr} ','','EB6:clon00','yes','','0'),('EB6','{BOS} {BOS} ','ctoftest1:ctoftest1','','yes','','no'),('ER6','{FPACK}  ','','file_1','yes','','no'),('file_1','  ','ER6:clon00','','yes','','no');
/*!40000 ALTER TABLE `ctoftest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctoftest_option`
--

DROP TABLE IF EXISTS `ctoftest_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctoftest_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctoftest_option`
--

LOCK TABLES `ctoftest_option` WRITE;
/*!40000 ALTER TABLE `ctoftest_option` DISABLE KEYS */;
INSERT INTO `ctoftest_option` VALUES ('dataLimit','0'),('eventLimit','170000'),('tokenInterval','64'),('dataFile','/work/ctof/ctof_%06d.A00'),('SPLITMB','2047'),('rocMask','1');
/*!40000 ALTER TABLE `ctoftest_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctoftest_pos`
--

DROP TABLE IF EXISTS `ctoftest_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctoftest_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctoftest_pos`
--

LOCK TABLES `ctoftest_pos` WRITE;
/*!40000 ALTER TABLE `ctoftest_pos` DISABLE KEYS */;
INSERT INTO `ctoftest_pos` VALUES ('ctoftest1',3,1),('EB6',3,3),('ER6',5,3),('file_1',5,5);
/*!40000 ALTER TABLE `ctoftest_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ctoftest_script`
--

DROP TABLE IF EXISTS `ctoftest_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ctoftest_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ctoftest_script`
--

LOCK TABLES `ctoftest_script` WRITE;
/*!40000 ALTER TABLE `ctoftest_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ctoftest_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb1`
--

DROP TABLE IF EXISTS `dcrb1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb1` (
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
-- Dumping data for table `dcrb1`
--

LOCK TABLES `dcrb1` WRITE;
/*!40000 ALTER TABLE `dcrb1` DISABLE KEYS */;
INSERT INTO `dcrb1` VALUES ('dcrb1','{dcrb1.so usr} {dcrb2.so usr} ','','EB8:dcrb1','yes','','20'),('dcrb1gtp','{gtp1.so usr} {gtp2.so usr} ','','EB8:dcrb1','yes','','11'),('EB8','{CODA} {CODA} ','dcrb1:dcrb1 dcrb1gtp:dcrb1','','yes','','no'),('ER8','{CODA}  ','','coda_1','yes','','no'),('coda_1','  ','ER8:dcrb1','','yes','','no');
/*!40000 ALTER TABLE `dcrb1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb1_option`
--

DROP TABLE IF EXISTS `dcrb1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb1_option`
--

LOCK TABLES `dcrb1_option` WRITE;
/*!40000 ALTER TABLE `dcrb1_option` DISABLE KEYS */;
INSERT INTO `dcrb1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000000 0x00100800'),('dataFile','/work/dcrb/dcrbgtp');
/*!40000 ALTER TABLE `dcrb1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb1_pos`
--

DROP TABLE IF EXISTS `dcrb1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb1_pos`
--

LOCK TABLES `dcrb1_pos` WRITE;
/*!40000 ALTER TABLE `dcrb1_pos` DISABLE KEYS */;
INSERT INTO `dcrb1_pos` VALUES ('dcrb1',3,1),('dcrb1gtp',5,1),('EB8',3,3),('ER8',5,3),('coda_1',5,5);
/*!40000 ALTER TABLE `dcrb1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb1_script`
--

DROP TABLE IF EXISTS `dcrb1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb1_script`
--

LOCK TABLES `dcrb1_script` WRITE;
/*!40000 ALTER TABLE `dcrb1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `dcrb1_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb2`
--

DROP TABLE IF EXISTS `dcrb2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb2` (
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
-- Dumping data for table `dcrb2`
--

LOCK TABLES `dcrb2` WRITE;
/*!40000 ALTER TABLE `dcrb2` DISABLE KEYS */;
INSERT INTO `dcrb2` VALUES ('dcrb2','{hps1.so usr} {hps2.so usr} ','','EB23:dcrb2','yes','','67'),('EB23','{CODA} {CODA} ','dcrb2:dcrb2','','yes','','no'),('ER23','{CODA}  ','','coda_0','yes','','no'),('ET223','{500} {200000} ','','','yes','','no'),('coda_0','{/work/dcrb/dcrb2} {CODA} ','ER23:dcrb2','','yes','','no');
/*!40000 ALTER TABLE `dcrb2` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb2_er_option`
--

DROP TABLE IF EXISTS `dcrb2_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb2_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb2_er_option`
--

LOCK TABLES `dcrb2_er_option` WRITE;
/*!40000 ALTER TABLE `dcrb2_er_option` DISABLE KEYS */;
INSERT INTO `dcrb2_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/dcrb/dcrbtest'),('rocMask','2097152');
/*!40000 ALTER TABLE `dcrb2_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb2_option`
--

DROP TABLE IF EXISTS `dcrb2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb2_option`
--

LOCK TABLES `dcrb2_option` WRITE;
/*!40000 ALTER TABLE `dcrb2_option` DISABLE KEYS */;
INSERT INTO `dcrb2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','2097152'),('dataFile','/work/dcrb/dcrb2');
/*!40000 ALTER TABLE `dcrb2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb2_pos`
--

DROP TABLE IF EXISTS `dcrb2_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb2_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb2_pos`
--

LOCK TABLES `dcrb2_pos` WRITE;
/*!40000 ALTER TABLE `dcrb2_pos` DISABLE KEYS */;
INSERT INTO `dcrb2_pos` VALUES ('dcrb2',3,1),('EB23',3,3),('ER23',5,3),('ET223',3,5),('coda_0',5,5);
/*!40000 ALTER TABLE `dcrb2_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dcrb2_script`
--

DROP TABLE IF EXISTS `dcrb2_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dcrb2_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dcrb2_script`
--

LOCK TABLES `dcrb2_script` WRITE;
/*!40000 ALTER TABLE `dcrb2_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `dcrb2_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dvcs_er`
--

DROP TABLE IF EXISTS `dvcs_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dvcs_er` (
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
-- Dumping data for table `dvcs_er`
--

LOCK TABLES `dvcs_er` WRITE;
/*!40000 ALTER TABLE `dvcs_er` DISABLE KEYS */;
INSERT INTO `dvcs_er` VALUES ('dvcs2','{/usr/clas/devel/coda/src/rol/rol/Linux_i686/obj/tid1.so usr} {/usr/clas/devel/coda/src/rol/rol/Linux_i686/obj/urol2_tt_testsetup.so usr} ','','EB5:clon00','yes','','17'),('EB5','{BOS} {BOS} ','dvcs2:dvcs2','','yes','','no'),('ER5','{FPACK}  ','','file_0','yes','','no'),('file_0','','ER5:clon00','','yes','','no');
/*!40000 ALTER TABLE `dvcs_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dvcs_er_option`
--

DROP TABLE IF EXISTS `dvcs_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dvcs_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dvcs_er_option`
--

LOCK TABLES `dvcs_er_option` WRITE;
/*!40000 ALTER TABLE `dvcs_er_option` DISABLE KEYS */;
INSERT INTO `dvcs_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/stage_in/dvcs2_%06d.A00'),('SPLITMB','2047'),('rocMask','131072');
/*!40000 ALTER TABLE `dvcs_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dvcs_er_pos`
--

DROP TABLE IF EXISTS `dvcs_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dvcs_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dvcs_er_pos`
--

LOCK TABLES `dvcs_er_pos` WRITE;
/*!40000 ALTER TABLE `dvcs_er_pos` DISABLE KEYS */;
INSERT INTO `dvcs_er_pos` VALUES ('dvcs2',3,1),('EB5',3,3),('ER5',5,3),('file_0',5,5);
/*!40000 ALTER TABLE `dvcs_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dvcs_er_script`
--

DROP TABLE IF EXISTS `dvcs_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dvcs_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dvcs_er_script`
--

LOCK TABLES `dvcs_er_script` WRITE;
/*!40000 ALTER TABLE `dvcs_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `dvcs_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dvcs_option`
--

DROP TABLE IF EXISTS `dvcs_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `dvcs_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dvcs_option`
--

LOCK TABLES `dvcs_option` WRITE;
/*!40000 ALTER TABLE `dvcs_option` DISABLE KEYS */;
INSERT INTO `dvcs_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','131072');
/*!40000 ALTER TABLE `dvcs_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fadc17200_option`
--

DROP TABLE IF EXISTS `fadc17200_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fadc17200_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fadc17200_option`
--

LOCK TABLES `fadc17200_option` WRITE;
/*!40000 ALTER TABLE `fadc17200_option` DISABLE KEYS */;
INSERT INTO `fadc17200_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','134217729'),('dataFile','test.dat');
/*!40000 ALTER TABLE `fadc17200_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fadc1720_alone_option`
--

DROP TABLE IF EXISTS `fadc1720_alone_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fadc1720_alone_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fadc1720_alone_option`
--

LOCK TABLES `fadc1720_alone_option` WRITE;
/*!40000 ALTER TABLE `fadc1720_alone_option` DISABLE KEYS */;
INSERT INTO `fadc1720_alone_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none');
/*!40000 ALTER TABLE `fadc1720_alone_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `fadc1720_option`
--

DROP TABLE IF EXISTS `fadc1720_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fadc1720_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `fadc1720_option`
--

LOCK TABLES `fadc1720_option` WRITE;
/*!40000 ALTER TABLE `fadc1720_option` DISABLE KEYS */;
INSERT INTO `fadc1720_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','200'),('rocMask','134217729'),('dataFile','test.dat');
/*!40000 ALTER TABLE `fadc1720_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ft`
--

DROP TABLE IF EXISTS `ft`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ft` (
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
-- Dumping data for table `ft`
--

LOCK TABLES `ft` WRITE;
/*!40000 ALTER TABLE `ft` DISABLE KEYS */;
INSERT INTO `ft` VALUES ('adcft1','{hps1_master.so usr} {hps2.so usr} ','','EB78:adcft1','yes','adcft2','no'),('adcft2','{hps1_slave.so usr} {hps2.so usr} ','','EB78:adcft1','no','','no'),('EB78','{CODA} {CODA} ','adcft1:adcft1 adcft2:adcft2','ET78:adcft1','yes','','no'),('ER78','{CODA}  ','ET78:adcft1','coda_0','yes','','no'),('ET78','{500} {100000} ','EB78:adcft1','ER78:adcft1','yes','','no'),('coda_0','{/work/ft/run_jlabftcal} {CODA} ','ER78:adcft1','','yes','','no');
/*!40000 ALTER TABLE `ft` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ft_option`
--

DROP TABLE IF EXISTS `ft_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ft_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ft_option`
--

LOCK TABLES `ft_option` WRITE;
/*!40000 ALTER TABLE `ft_option` DISABLE KEYS */;
INSERT INTO `ft_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ft/run_jlabftcal');
/*!40000 ALTER TABLE `ft_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ft_pos`
--

DROP TABLE IF EXISTS `ft_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ft_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ft_pos`
--

LOCK TABLES `ft_pos` WRITE;
/*!40000 ALTER TABLE `ft_pos` DISABLE KEYS */;
INSERT INTO `ft_pos` VALUES ('adcft1',2,1),('adcft2',4,1),('EB78',2,3),('ER78',4,3),('ET78',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `ft_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ft_script`
--

DROP TABLE IF EXISTS `ft_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ft_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ft_script`
--

LOCK TABLES `ft_script` WRITE;
/*!40000 ALTER TABLE `ft_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ftof0_option`
--

DROP TABLE IF EXISTS `ftof0_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ftof0_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ftof0_option`
--

LOCK TABLES `ftof0_option` WRITE;
/*!40000 ALTER TABLE `ftof0_option` DISABLE KEYS */;
INSERT INTO `ftof0_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ftof/ftof0test'),('rocMask','134217728');
/*!40000 ALTER TABLE `ftof0_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ftof1_option`
--

DROP TABLE IF EXISTS `ftof1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ftof1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ftof1_option`
--

LOCK TABLES `ftof1_option` WRITE;
/*!40000 ALTER TABLE `ftof1_option` DISABLE KEYS */;
INSERT INTO `ftof1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','268435456'),('dataFile','/work/ftof/ftof1test');
/*!40000 ALTER TABLE `ftof1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ftof_option`
--

DROP TABLE IF EXISTS `ftof_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ftof_option` (
  `name` char(32) default NULL,
  `value` char(80) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ftof_option`
--

LOCK TABLES `ftof_option` WRITE;
/*!40000 ALTER TABLE `ftof_option` DISABLE KEYS */;
INSERT INTO `ftof_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/ftof/ftof6'),('rocMask','0x00000000 0x00000000 0x00000000 0x18000000'),('confFile','/usr/local/clas12/release/0.1/parms/trigger/clasdev.cnf');
/*!40000 ALTER TABLE `ftof_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ftoftest_option`
--

DROP TABLE IF EXISTS `ftoftest_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ftoftest_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ftoftest_option`
--

LOCK TABLES `ftoftest_option` WRITE;
/*!40000 ALTER TABLE `ftoftest_option` DISABLE KEYS */;
INSERT INTO `ftoftest_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','402653184');
/*!40000 ALTER TABLE `ftoftest_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `halldtrg3_er_option`
--

DROP TABLE IF EXISTS `halldtrg3_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `halldtrg3_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `halldtrg3_er_option`
--

LOCK TABLES `halldtrg3_er_option` WRITE;
/*!40000 ALTER TABLE `halldtrg3_er_option` DISABLE KEYS */;
INSERT INTO `halldtrg3_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/stage_in/halld_%06d.A00'),('SPLITMB','2047'),('rocMask','65536');
/*!40000 ALTER TABLE `halldtrg3_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `halldtrg3_option`
--

DROP TABLE IF EXISTS `halldtrg3_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `halldtrg3_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `halldtrg3_option`
--

LOCK TABLES `halldtrg3_option` WRITE;
/*!40000 ALTER TABLE `halldtrg3_option` DISABLE KEYS */;
INSERT INTO `halldtrg3_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','65536');
/*!40000 ALTER TABLE `halldtrg3_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest`
--

DROP TABLE IF EXISTS `hbtest`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest` (
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
-- Dumping data for table `hbtest`
--

LOCK TABLES `hbtest` WRITE;
/*!40000 ALTER TABLE `hbtest` DISABLE KEYS */;
INSERT INTO `hbtest` VALUES ('highbtest3','{hbtest1.so usr} {hps2.so usr} ','','EB21:highbtest2','yes','','47'),('EB21','{CODA} {CODA} ','highbtest3:highbtest3','','yes','','no');
/*!40000 ALTER TABLE `hbtest` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2`
--

DROP TABLE IF EXISTS `hbtest2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2` (
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
-- Dumping data for table `hbtest2`
--

LOCK TABLES `hbtest2` WRITE;
/*!40000 ALTER TABLE `hbtest2` DISABLE KEYS */;
INSERT INTO `hbtest2` VALUES ('highbtest3','{hbtest1.so usr} {hps2.so usr} ','','EB221:highbtest3','yes','','47'),('EB221','{CODA} {CODA} ','highbtest3:highbtest3','','yes','','no'),('ER221','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER221:highbtest3','','yes','','no');
/*!40000 ALTER TABLE `hbtest2` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_noer`
--

DROP TABLE IF EXISTS `hbtest2_noer`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_noer` (
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
-- Dumping data for table `hbtest2_noer`
--

LOCK TABLES `hbtest2_noer` WRITE;
/*!40000 ALTER TABLE `hbtest2_noer` DISABLE KEYS */;
INSERT INTO `hbtest2_noer` VALUES ('highbtest3','{hbtest1.so usr} {hps2.so usr} ','','EB221:highbtest3','yes','','47'),('EB221','{CODA} {CODA} ','highbtest3:highbtest3','','yes','','no');
/*!40000 ALTER TABLE `hbtest2_noer` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_noer_option`
--

DROP TABLE IF EXISTS `hbtest2_noer_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_noer_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest2_noer_option`
--

LOCK TABLES `hbtest2_noer_option` WRITE;
/*!40000 ALTER TABLE `hbtest2_noer_option` DISABLE KEYS */;
INSERT INTO `hbtest2_noer_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00008000 0x00000000');
/*!40000 ALTER TABLE `hbtest2_noer_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_noer_pos`
--

DROP TABLE IF EXISTS `hbtest2_noer_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_noer_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest2_noer_pos`
--

LOCK TABLES `hbtest2_noer_pos` WRITE;
/*!40000 ALTER TABLE `hbtest2_noer_pos` DISABLE KEYS */;
INSERT INTO `hbtest2_noer_pos` VALUES ('highbtest3',2,1),('EB221',2,3);
/*!40000 ALTER TABLE `hbtest2_noer_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_noer_script`
--

DROP TABLE IF EXISTS `hbtest2_noer_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_noer_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest2_noer_script`
--

LOCK TABLES `hbtest2_noer_script` WRITE;
/*!40000 ALTER TABLE `hbtest2_noer_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hbtest2_noer_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_option`
--

DROP TABLE IF EXISTS `hbtest2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest2_option`
--

LOCK TABLES `hbtest2_option` WRITE;
/*!40000 ALTER TABLE `hbtest2_option` DISABLE KEYS */;
INSERT INTO `hbtest2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00008000 0x00000000'),('dataFile','/work/hbtest/hbtest');
/*!40000 ALTER TABLE `hbtest2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_pos`
--

DROP TABLE IF EXISTS `hbtest2_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest2_pos`
--

LOCK TABLES `hbtest2_pos` WRITE;
/*!40000 ALTER TABLE `hbtest2_pos` DISABLE KEYS */;
INSERT INTO `hbtest2_pos` VALUES ('highbtest3',2,1),('EB221',2,3),('ER221',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `hbtest2_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest2_script`
--

DROP TABLE IF EXISTS `hbtest2_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest2_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest2_script`
--

LOCK TABLES `hbtest2_script` WRITE;
/*!40000 ALTER TABLE `hbtest2_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hbtest2_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_er`
--

DROP TABLE IF EXISTS `hbtest_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_er` (
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
-- Dumping data for table `hbtest_er`
--

LOCK TABLES `hbtest_er` WRITE;
/*!40000 ALTER TABLE `hbtest_er` DISABLE KEYS */;
INSERT INTO `hbtest_er` VALUES ('highbtest3','{hbtest1.so usr} {hps2.so usr} ','','EB21:highbtest2','yes','','47'),('EB21','{CODA} {CODA} ','highbtest3:highbtest3','','yes','','no'),('ER21','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER21:highbtest2','','yes','','no');
/*!40000 ALTER TABLE `hbtest_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_er_option`
--

DROP TABLE IF EXISTS `hbtest_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest_er_option`
--

LOCK TABLES `hbtest_er_option` WRITE;
/*!40000 ALTER TABLE `hbtest_er_option` DISABLE KEYS */;
INSERT INTO `hbtest_er_option` VALUES ('dataLimit','0'),('eventLimit','700000'),('tokenInterval','64'),('dataFile','/home/data/hbtest'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00008000 0x00000000');
/*!40000 ALTER TABLE `hbtest_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_er_pos`
--

DROP TABLE IF EXISTS `hbtest_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest_er_pos`
--

LOCK TABLES `hbtest_er_pos` WRITE;
/*!40000 ALTER TABLE `hbtest_er_pos` DISABLE KEYS */;
INSERT INTO `hbtest_er_pos` VALUES ('highbtest3',2,1),('EB21',2,3),('ER21',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `hbtest_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_er_script`
--

DROP TABLE IF EXISTS `hbtest_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest_er_script`
--

LOCK TABLES `hbtest_er_script` WRITE;
/*!40000 ALTER TABLE `hbtest_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hbtest_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_option`
--

DROP TABLE IF EXISTS `hbtest_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest_option`
--

LOCK TABLES `hbtest_option` WRITE;
/*!40000 ALTER TABLE `hbtest_option` DISABLE KEYS */;
INSERT INTO `hbtest_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00008000 0x00000000');
/*!40000 ALTER TABLE `hbtest_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_pos`
--

DROP TABLE IF EXISTS `hbtest_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest_pos`
--

LOCK TABLES `hbtest_pos` WRITE;
/*!40000 ALTER TABLE `hbtest_pos` DISABLE KEYS */;
INSERT INTO `hbtest_pos` VALUES ('highbtest3',2,1),('EB21',2,3);
/*!40000 ALTER TABLE `hbtest_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hbtest_script`
--

DROP TABLE IF EXISTS `hbtest_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hbtest_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hbtest_script`
--

LOCK TABLES `hbtest_script` WRITE;
/*!40000 ALTER TABLE `hbtest_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hbtest_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hist_ts1_option`
--

DROP TABLE IF EXISTS `hist_ts1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hist_ts1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hist_ts1_option`
--

LOCK TABLES `hist_ts1_option` WRITE;
/*!40000 ALTER TABLE `hist_ts1_option` DISABLE KEYS */;
INSERT INTO `hist_ts1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','-2013265920');
/*!40000 ALTER TABLE `hist_ts1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hist_ts2_option`
--

DROP TABLE IF EXISTS `hist_ts2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hist_ts2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hist_ts2_option`
--

LOCK TABLES `hist_ts2_option` WRITE;
/*!40000 ALTER TABLE `hist_ts2_option` DISABLE KEYS */;
INSERT INTO `hist_ts2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','134217729'),('dataFile','/work/stage_in/dsc2_%06d.A00'),('SPLITMB','2047');
/*!40000 ALTER TABLE `hist_ts2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps12_er_option`
--

DROP TABLE IF EXISTS `hps12_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps12_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps12_er_option`
--

LOCK TABLES `hps12_er_option` WRITE;
/*!40000 ALTER TABLE `hps12_er_option` DISABLE KEYS */;
INSERT INTO `hps12_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/hps/hps12'),('rocMask','6'),('SPLITMB','2047');
/*!40000 ALTER TABLE `hps12_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps12_option`
--

DROP TABLE IF EXISTS `hps12_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps12_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps12_option`
--

LOCK TABLES `hps12_option` WRITE;
/*!40000 ALTER TABLE `hps12_option` DISABLE KEYS */;
INSERT INTO `hps12_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','6');
/*!40000 ALTER TABLE `hps12_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps13_option`
--

DROP TABLE IF EXISTS `hps13_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps13_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps13_option`
--

LOCK TABLES `hps13_option` WRITE;
/*!40000 ALTER TABLE `hps13_option` DISABLE KEYS */;
INSERT INTO `hps13_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','10');
/*!40000 ALTER TABLE `hps13_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps1_er_option`
--

DROP TABLE IF EXISTS `hps1_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps1_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps1_er_option`
--

LOCK TABLES `hps1_er_option` WRITE;
/*!40000 ALTER TABLE `hps1_er_option` DISABLE KEYS */;
INSERT INTO `hps1_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','test.dat'),('rocMask','2');
/*!40000 ALTER TABLE `hps1_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps1_option`
--

DROP TABLE IF EXISTS `hps1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps1_option`
--

LOCK TABLES `hps1_option` WRITE;
/*!40000 ALTER TABLE `hps1_option` DISABLE KEYS */;
INSERT INTO `hps1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','2');
/*!40000 ALTER TABLE `hps1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps1gtp_option`
--

DROP TABLE IF EXISTS `hps1gtp_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps1gtp_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps1gtp_option`
--

LOCK TABLES `hps1gtp_option` WRITE;
/*!40000 ALTER TABLE `hps1gtp_option` DISABLE KEYS */;
INSERT INTO `hps1gtp_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000040 0x00000000');
/*!40000 ALTER TABLE `hps1gtp_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps_er_option`
--

DROP TABLE IF EXISTS `hps_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps_er_option`
--

LOCK TABLES `hps_er_option` WRITE;
/*!40000 ALTER TABLE `hps_er_option` DISABLE KEYS */;
INSERT INTO `hps_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/hps/hps'),('rocMask','14'),('SPLITMB','2047');
/*!40000 ALTER TABLE `hps_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hps_option`
--

DROP TABLE IF EXISTS `hps_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hps_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hps_option`
--

LOCK TABLES `hps_option` WRITE;
/*!40000 ALTER TABLE `hps_option` DISABLE KEYS */;
INSERT INTO `hps_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x000000a0 0x00000000');
/*!40000 ALTER TABLE `hps_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp`
--

DROP TABLE IF EXISTS `hpsgtp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp` (
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
-- Dumping data for table `hpsgtp`
--

LOCK TABLES `hpsgtp` WRITE;
/*!40000 ALTER TABLE `hpsgtp` DISABLE KEYS */;
INSERT INTO `hpsgtp` VALUES ('hps1','{hps1_master.so usr} {hps2.so usr} ','','EB9:hps1','yes','','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB9:hps1','yes','','39'),('hps1gtp','{gtp1.so usr} {rol2.so usr} ','','','no','','no'),('hps2gtp','{gtp1.so usr} {rol2.so usr} ','','','no','','no'),('EB9','{CODA} {CODA} ','hps1:hps1 hps2:hps2','','yes','','no');
/*!40000 ALTER TABLE `hpsgtp` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_er`
--

DROP TABLE IF EXISTS `hpsgtp_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_er` (
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
-- Dumping data for table `hpsgtp_er`
--

LOCK TABLES `hpsgtp_er` WRITE;
/*!40000 ALTER TABLE `hpsgtp_er` DISABLE KEYS */;
INSERT INTO `hpsgtp_er` VALUES ('hps1','{hps1_master.so usr} {hps2.so usr} ','','EB9:hps1','yes','','37'),('hps2','{hps1_slave.so usr} {hps2.so usr} ','','EB9:hps1','yes','','39'),('hps1gtp','{gtp1.so usr} {rol2.so usr} ','','','no','','no'),('hps2gtp','{gtp1.so usr} {rol2.so usr} ','','','no','','no'),('EB9','{CODA} {CODA} ','hps1:hps1 hps2:hps2','','yes','','no'),('ER9','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER9:hps1','','yes','','no');
/*!40000 ALTER TABLE `hpsgtp_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_er_option`
--

DROP TABLE IF EXISTS `hpsgtp_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpsgtp_er_option`
--

LOCK TABLES `hpsgtp_er_option` WRITE;
/*!40000 ALTER TABLE `hpsgtp_er_option` DISABLE KEYS */;
INSERT INTO `hpsgtp_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/hps/hpsgtp'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x000000a0 0x00000000');
/*!40000 ALTER TABLE `hpsgtp_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_er_pos`
--

DROP TABLE IF EXISTS `hpsgtp_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpsgtp_er_pos`
--

LOCK TABLES `hpsgtp_er_pos` WRITE;
/*!40000 ALTER TABLE `hpsgtp_er_pos` DISABLE KEYS */;
INSERT INTO `hpsgtp_er_pos` VALUES ('hps1',3,1),('hps2',5,1),('hps1gtp',7,1),('hps2gtp',9,1),('EB9',3,3),('ER9',5,3),('coda_0',5,5);
/*!40000 ALTER TABLE `hpsgtp_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_er_script`
--

DROP TABLE IF EXISTS `hpsgtp_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpsgtp_er_script`
--

LOCK TABLES `hpsgtp_er_script` WRITE;
/*!40000 ALTER TABLE `hpsgtp_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hpsgtp_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_option`
--

DROP TABLE IF EXISTS `hpsgtp_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpsgtp_option`
--

LOCK TABLES `hpsgtp_option` WRITE;
/*!40000 ALTER TABLE `hpsgtp_option` DISABLE KEYS */;
INSERT INTO `hpsgtp_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x000000a0 0x00000000'),('confFile','none');
/*!40000 ALTER TABLE `hpsgtp_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_pos`
--

DROP TABLE IF EXISTS `hpsgtp_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpsgtp_pos`
--

LOCK TABLES `hpsgtp_pos` WRITE;
/*!40000 ALTER TABLE `hpsgtp_pos` DISABLE KEYS */;
INSERT INTO `hpsgtp_pos` VALUES ('hps1',3,1),('hps2',5,1),('hps1gtp',7,1),('hps2gtp',9,1),('EB9',3,3);
/*!40000 ALTER TABLE `hpsgtp_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpsgtp_script`
--

DROP TABLE IF EXISTS `hpsgtp_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpsgtp_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpsgtp_script`
--

LOCK TABLES `hpsgtp_script` WRITE;
/*!40000 ALTER TABLE `hpsgtp_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hpsgtp_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt`
--

DROP TABLE IF EXISTS `hpssvt`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt` (
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
-- Dumping data for table `hpssvt`
--

LOCK TABLES `hpssvt` WRITE;
/*!40000 ALTER TABLE `hpssvt` DISABLE KEYS */;
INSERT INTO `hpssvt` VALUES ('hps10','{hps1_master.so usr} {hps2.so usr} ','','EB19:clondaq3','yes','','48'),('dpm0','{dpm.so usr}  ','','EB19:clondaq3','yes','dpm1','51'),('dpm1','{dpm.so usr}  ','','EB19:clondaq3','no','dpm2','52'),('dpm2','{dpm.so usr}  ','','EB19:clondaq3','no','dpm3','53'),('dpm3','{dpm.so usr}  ','','EB19:clondaq3','no','dpm4','54'),('dpm4','{dpm.so usr}  ','','EB19:clondaq3','no','dpm5','55'),('dpm5','{dpm.so usr}  ','','EB19:clondaq3','no','dpm6','56'),('dpm6','{dpm.so usr}  ','','EB19:clondaq3','no','dpm7','57'),('dpm7','{dpm.so usr}  ','','EB19:clondaq3','no','dpm8','66'),('dpm8','{dpm.so usr}  ','','EB19:clondaq3','no','dpm9','59'),('dpm9','{dpm.so usr}  ','','EB19:clondaq3','no','dpm10','60'),('dpm10','{dpm.so usr}  ','','EB19:clondaq3','no','dpm11','61'),('dpm11','{dpm.so usr}  ','','EB19:clondaq3','no','dpm12','62'),('dpm12','{dpm.so usr}  ','','EB19:clondaq3','no','dpm13','63'),('dpm13','{dpm.so usr}  ','','EB19:clondaq3','no','dpm14','64'),('dpm14','{dpm.so usr}  ','','EB19:clondaq3','no','','65'),('dtm0','{dtm.so usr}  ','','','no','','no'),('dtm1','{dtm.so usr}  ','','','no','','no'),('EB19','  ','hps10:hps10 dpm0:dpm0 dpm1:dpm1 dpm2:dpm2 dpm3:dpm3 dpm4:dpm4 dpm5:dpm5 dpm6:dpm6 dpm7:dpm7 dpm8:dpm8 dpm9:dpm9 dpm10:dpm10 dpm11:dpm11 dpm12:dpm12 dpm13:dpm13 dpm14:dpm14','','yes','','no'),('ER19','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER19:clondaq3','','yes','','no');
/*!40000 ALTER TABLE `hpssvt` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_noer`
--

DROP TABLE IF EXISTS `hpssvt_noer`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_noer` (
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
-- Dumping data for table `hpssvt_noer`
--

LOCK TABLES `hpssvt_noer` WRITE;
/*!40000 ALTER TABLE `hpssvt_noer` DISABLE KEYS */;
INSERT INTO `hpssvt_noer` VALUES ('hps10','{hps1_master.so usr} {hps2.so usr} ','','EB19:clondaq3','yes','','48'),('dpm0','{dpm.so usr}  ','','EB19:clondaq3','yes','dpm1','51'),('dpm1','{dpm.so usr}  ','','EB19:clondaq3','no','dpm2','52'),('dpm2','{dpm.so usr}  ','','EB19:clondaq3','no','dpm3','53'),('dpm3','{dpm.so usr}  ','','EB19:clondaq3','no','dpm4','54'),('dpm4','{dpm.so usr}  ','','EB19:clondaq3','no','dpm5','55'),('dpm5','{dpm.so usr}  ','','EB19:clondaq3','no','dpm6','56'),('dpm6','{dpm.so usr}  ','','EB19:clondaq3','no','dpm7','57'),('dpm7','{dpm.so usr}  ','','EB19:clondaq3','no','dpm8','66'),('dpm8','{dpm.so usr}  ','','EB19:clondaq3','no','dpm9','59'),('dpm9','{dpm.so usr}  ','','EB19:clondaq3','no','dpm10','60'),('dpm10','{dpm.so usr}  ','','EB19:clondaq3','no','dpm11','61'),('dpm11','{dpm.so usr}  ','','EB19:clondaq3','no','dpm12','62'),('dpm12','{dpm.so usr}  ','','EB19:clondaq3','no','dpm13','63'),('dpm13','{dpm.so usr}  ','','EB19:clondaq3','no','dpm14','64'),('dpm14','{dpm.so usr}  ','','EB19:clondaq3','no','','65'),('dtm0','{dtm.so usr}  ','','','no','','no'),('dtm1','{dtm.so usr}  ','','','no','','no'),('EB19','  ','hps10:hps10 dpm0:dpm0 dpm1:dpm1 dpm2:dpm2 dpm3:dpm3 dpm4:dpm4 dpm5:dpm5 dpm6:dpm6 dpm7:dpm7 dpm8:dpm8 dpm9:dpm9 dpm10:dpm10 dpm11:dpm11 dpm12:dpm12 dpm13:dpm13 dpm14:dpm14','','yes','','no');
/*!40000 ALTER TABLE `hpssvt_noer` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_noer_option`
--

DROP TABLE IF EXISTS `hpssvt_noer_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_noer_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpssvt_noer_option`
--

LOCK TABLES `hpssvt_noer_option` WRITE;
/*!40000 ALTER TABLE `hpssvt_noer_option` DISABLE KEYS */;
INSERT INTO `hpssvt_noer_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','/usr/local/clas12/release/0.2/parms/trigger/SVT/HpsSvt-baseline.cnf'),('rocMask','0x00000000 0x00000007 0xfbf90000 0x00000000');
/*!40000 ALTER TABLE `hpssvt_noer_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_noer_pos`
--

DROP TABLE IF EXISTS `hpssvt_noer_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_noer_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpssvt_noer_pos`
--

LOCK TABLES `hpssvt_noer_pos` WRITE;
/*!40000 ALTER TABLE `hpssvt_noer_pos` DISABLE KEYS */;
INSERT INTO `hpssvt_noer_pos` VALUES ('hps10',1,1),('dpm0',3,1),('dpm1',4,1),('dpm2',5,1),('dpm3',6,1),('dpm4',7,1),('dpm5',8,1),('dpm6',9,1),('dpm7',10,1),('dpm8',11,1),('dpm9',12,1),('dpm10',13,1),('dpm11',14,1),('dpm12',15,1),('dpm13',16,1),('dpm14',17,1),('dtm0',19,1),('dtm1',20,1),('EB19',13,3);
/*!40000 ALTER TABLE `hpssvt_noer_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_noer_script`
--

DROP TABLE IF EXISTS `hpssvt_noer_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_noer_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpssvt_noer_script`
--

LOCK TABLES `hpssvt_noer_script` WRITE;
/*!40000 ALTER TABLE `hpssvt_noer_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hpssvt_noer_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_option`
--

DROP TABLE IF EXISTS `hpssvt_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpssvt_option`
--

LOCK TABLES `hpssvt_option` WRITE;
/*!40000 ALTER TABLE `hpssvt_option` DISABLE KEYS */;
INSERT INTO `hpssvt_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/hps/hpssvt'),('confFile','/usr/local/clas12/release/0.2/parms/trigger/SVT/HpsSvt-baseline.cnf'),('rocMask','0x00000000 0x00000007 0xfbf90000 0x00000000');
/*!40000 ALTER TABLE `hpssvt_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_pos`
--

DROP TABLE IF EXISTS `hpssvt_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpssvt_pos`
--

LOCK TABLES `hpssvt_pos` WRITE;
/*!40000 ALTER TABLE `hpssvt_pos` DISABLE KEYS */;
INSERT INTO `hpssvt_pos` VALUES ('hps10',1,1),('dpm0',3,1),('dpm1',4,1),('dpm2',5,1),('dpm3',6,1),('dpm4',7,1),('dpm5',8,1),('dpm6',9,1),('dpm7',10,1),('dpm8',11,1),('dpm9',12,1),('dpm10',13,1),('dpm11',14,1),('dpm12',15,1),('dpm13',16,1),('dpm14',17,1),('dtm0',19,1),('dtm1',20,1),('EB19',13,3),('ER19',15,3),('coda_0',15,5);
/*!40000 ALTER TABLE `hpssvt_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpssvt_script`
--

DROP TABLE IF EXISTS `hpssvt_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpssvt_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpssvt_script`
--

LOCK TABLES `hpssvt_script` WRITE;
/*!40000 ALTER TABLE `hpssvt_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `hpssvt_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpstracker_er_option`
--

DROP TABLE IF EXISTS `hpstracker_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpstracker_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpstracker_er_option`
--

LOCK TABLES `hpstracker_er_option` WRITE;
/*!40000 ALTER TABLE `hpstracker_er_option` DISABLE KEYS */;
INSERT INTO `hpstracker_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','test.dat'),('rocMask','8');
/*!40000 ALTER TABLE `hpstracker_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `hpstracker_option`
--

DROP TABLE IF EXISTS `hpstracker_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hpstracker_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `hpstracker_option`
--

LOCK TABLES `hpstracker_option` WRITE;
/*!40000 ALTER TABLE `hpstracker_option` DISABLE KEYS */;
INSERT INTO `hpstracker_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','8'),('dataFile','/w/stage/output/hallb/hps/data/hpstracker'),('SPLITMB','2047');
/*!40000 ALTER TABLE `hpstracker_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `lac2_option`
--

DROP TABLE IF EXISTS `lac2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `lac2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `lac2_option`
--

LOCK TABLES `lac2_option` WRITE;
/*!40000 ALTER TABLE `lac2_option` DISABLE KEYS */;
INSERT INTO `lac2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','16777216');
/*!40000 ALTER TABLE `lac2_option` ENABLE KEYS */;
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
INSERT INTO `links` VALUES ('wolfram1->EB2','TCP','clon00','down',60157),('dvcsfb->EB8','TCP','clasonl1','down',56768),('croctest10->EB6','TCP','clonxt1','down',36984),('croctest1->EB6','TCP','clonxt1','down',36977),('lac2->EB5','TCP','clon00','down',61103),('croctest5->EB5','TCP','clon00','down',35778),('croctest1->EB5','TCP','clon00','down',40550),('croctest10->EB5','TCP','clon00','down',43381),('ROC26->EB5','TCP','clon00','down',33325),('croctest2->EB5','TCP','clon00','down',43382),('ROC24->EB5','TCP','clon00','down',37630),('ctoftest1->EB6','TCP','clon00','down',56841),('bonuspc3->EB5','TCP','clon00','down',65345),('croctest3->EB5','TCP','clon00','down',48747),('croctest2->EB40','TCP','clonmon0','down',41827),('croctest10->EB40','TCP','clonmon0','down',44728),('croctest2->EB4','TCP','bonuspc1','down',46919),('croctest10->EB4','TCP','bonuspc1','down',39681),('croctest2->EB6','TCP','clon00','down',61310),('croctest3->EB40','TCP','clondaq1','down',45980),('jlab12vme->EB7','TCP','clon00','down',40408),('pcalfb->EB8','TCP','clon10','up',55522),('dvcs2->EB5','TCP','clon00','down',34762),('halldtrg3->EB5','TCP','clon00','down',62674),('hps1->EB9','TCP','hps1','up',37967),('hps2->EB9','TCP','hps1','up',56723),('hpstracker->EB9','TCP','clonusr3','down',46849),('svt1->EB10','TCP','clonusr2','down',35919),('svt2->EB10','TCP','svt2','down',46250),('dcrb1->EB8','TCP','dcrb1','waiting',48091),('dcrb2->EB8','TCP','clon00','down',44289),('ftof0->EB4','TCP','ftof0','down',41607),('ftof1->EB4','TCP','ftof0','down',39326),('ltcc0->EB3','TCP','ltcc0','down',53785),('adcecal1->EB1','TCP','adcecal1','down',40928),('tdcecal1->EB1','TCP','adcecal1','down',48469),('ROC0->EB0','TCP','clonpc0','down',40190),('trig1->EB0','TCP','clonioc1','down',35698),('dcrb1gtp->EB8','TCP','dcrb1','up',41971),('adcecal1->EB11','TCP','adcecal1','up',55963),('tdcecal1->EB11','TCP','adcecal1','up',35753),('tdcftof1->EB11','TCP','adcecal1','up',47705),('tdcpcal1->EB11','TCP','adcecal1','up',37212),('adcpcal1->EB11','TCP','adcecal1','up',40083),('adcftof1->EB11','TCP','adcecal1','up',52892),('adcecal5->EB15','TCP','adcecal5','down',41689),('tdcecal5->EB15','TCP','adcecal5','down',41863),('adcpcal5->EB15','TCP','adcecal5','down',48271),('tdcpcal5->EB15','TCP','adcecal5','down',43766),('adcftof5->EB15','TCP','adcecal5','down',60127),('tdcftof5->EB15','TCP','adcecal5','down',58509),('hps1gtp->EB9','TCP','hps1','down',53271),('hps2gtp->EB9','TCP','hps1','down',55982),('adcecal4->EB14','TCP','adcecal4','up',45241),('tdcecal4->EB14','TCP','adcecal4','up',46239),('adcpcal4->EB14','TCP','adcecal4','up',56641),('tdcpcal4->EB14','TCP','adcecal4','up',57570),('adcftof4->EB14','TCP','adcecal4','up',39013),('tdcftof4->EB14','TCP','adcecal4','up',56785),('adcecal3->EB13','TCP','adcecal3','up',53279),('tdcecal3->EB13','TCP','adcecal3','up',48055),('adcpcal3->EB13','TCP','adcecal3','up',41454),('tdcpcal3->EB13','TCP','adcecal3','up',39649),('adcftof3->EB13','TCP','adcecal3','up',39180),('tdcftof3->EB13','TCP','adcecal3','up',44208),('adcecal2->EB12','TCP','adcecal2','down',41171),('tdcecal2->EB12','TCP','adcecal2','down',48613),('adcpcal2->EB12','TCP','adcecal2','down',35701),('tdcpcal2->EB12','TCP','adcecal2','down',42955),('adcftof2->EB12','TCP','adcecal2','down',33742),('tdcftof2->EB12','TCP','adcecal2','down',51490),('adcecal6->EB16','TCP','adcecal6','down',50018),('adcpcal6->EB16','TCP','adcecal6','down',44613),('adcftof6->EB16','TCP','adcecal6','down',54177),('tdcftof6->EB16','TCP','adcecal6','down',56790),('tdcpcal6->EB16','TCP','adcecal6','down',41606),('zedboard1->EB0','TCP','clonpc0','down',54926);
/*!40000 ALTER TABLE `links` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0`
--

DROP TABLE IF EXISTS `ltcc0`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0` (
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
-- Dumping data for table `ltcc0`
--

LOCK TABLES `ltcc0` WRITE;
/*!40000 ALTER TABLE `ltcc0` DISABLE KEYS */;
INSERT INTO `ltcc0` VALUES ('ltcc0','{fadc1.so usr} {fadc2.so usr} ','','EB3:ltcc0','yes','','no'),('EB3','{CODA} {CODA} ','ltcc0:ltcc0','ET3:ltcc0','yes','','no'),('ER3','{CODA}  ','ET3:ltcc0','coda_0','yes','','no'),('ET3','{1000} {50000} ','EB3:ltcc0','ER3:ltcc0','yes','','no'),('coda_0','  ','ER3:ltcc0','','yes','','no');
/*!40000 ALTER TABLE `ltcc0` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_noer`
--

DROP TABLE IF EXISTS `ltcc0_noer`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_noer` (
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
-- Dumping data for table `ltcc0_noer`
--

LOCK TABLES `ltcc0_noer` WRITE;
/*!40000 ALTER TABLE `ltcc0_noer` DISABLE KEYS */;
INSERT INTO `ltcc0_noer` VALUES ('ltcc0','{fadc1.so usr} {fadc2.so usr} ','','EB3:ltcc0','yes','','no'),('EB3','{CODA} {CODA} ','ltcc0:ltcc0','ET3:ltcc0','yes','','no'),('ET3','{1000} {50000} ','EB3:ltcc0','','yes','','no');
/*!40000 ALTER TABLE `ltcc0_noer` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_noer_option`
--

DROP TABLE IF EXISTS `ltcc0_noer_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_noer_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ltcc0_noer_option`
--

LOCK TABLES `ltcc0_noer_option` WRITE;
/*!40000 ALTER TABLE `ltcc0_noer_option` DISABLE KEYS */;
INSERT INTO `ltcc0_noer_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64');
/*!40000 ALTER TABLE `ltcc0_noer_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_noer_pos`
--

DROP TABLE IF EXISTS `ltcc0_noer_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_noer_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ltcc0_noer_pos`
--

LOCK TABLES `ltcc0_noer_pos` WRITE;
/*!40000 ALTER TABLE `ltcc0_noer_pos` DISABLE KEYS */;
INSERT INTO `ltcc0_noer_pos` VALUES ('ltcc0',2,1),('EB3',2,3),('ET3',2,5);
/*!40000 ALTER TABLE `ltcc0_noer_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_noer_script`
--

DROP TABLE IF EXISTS `ltcc0_noer_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_noer_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ltcc0_noer_script`
--

LOCK TABLES `ltcc0_noer_script` WRITE;
/*!40000 ALTER TABLE `ltcc0_noer_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ltcc0_noer_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_option`
--

DROP TABLE IF EXISTS `ltcc0_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ltcc0_option`
--

LOCK TABLES `ltcc0_option` WRITE;
/*!40000 ALTER TABLE `ltcc0_option` DISABLE KEYS */;
INSERT INTO `ltcc0_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','');
/*!40000 ALTER TABLE `ltcc0_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_pos`
--

DROP TABLE IF EXISTS `ltcc0_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ltcc0_pos`
--

LOCK TABLES `ltcc0_pos` WRITE;
/*!40000 ALTER TABLE `ltcc0_pos` DISABLE KEYS */;
INSERT INTO `ltcc0_pos` VALUES ('ltcc0',2,1),('EB3',2,3),('ER3',4,3),('ET3',2,5),('coda_0',4,5);
/*!40000 ALTER TABLE `ltcc0_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ltcc0_script`
--

DROP TABLE IF EXISTS `ltcc0_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ltcc0_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ltcc0_script`
--

LOCK TABLES `ltcc0_script` WRITE;
/*!40000 ALTER TABLE `ltcc0_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `ltcc0_script` ENABLE KEYS */;
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
-- Table structure for table `mvt1_noer`
--

DROP TABLE IF EXISTS `mvt1_noer`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_noer` (
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
-- Dumping data for table `mvt1_noer`
--

LOCK TABLES `mvt1_noer` WRITE;
/*!40000 ALTER TABLE `mvt1_noer` DISABLE KEYS */;
INSERT INTO `mvt1_noer` VALUES ('mvt1','{hps1test.so usr} {hps2.so usr} ','','EB77:mvt1','yes','','69'),('EB77','{CODA} {CODA} ','mvt1:mvt1','','yes','','no'),('ET77','{1000} {50000} ','','','yes','','no');
/*!40000 ALTER TABLE `mvt1_noer` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1_noer_option`
--

DROP TABLE IF EXISTS `mvt1_noer_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_noer_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mvt1_noer_option`
--

LOCK TABLES `mvt1_noer_option` WRITE;
/*!40000 ALTER TABLE `mvt1_noer_option` DISABLE KEYS */;
INSERT INTO `mvt1_noer_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','/usr/clas12/release/0.3/parms/trigger/clasdev.cnf');
/*!40000 ALTER TABLE `mvt1_noer_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1_noer_pos`
--

DROP TABLE IF EXISTS `mvt1_noer_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_noer_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mvt1_noer_pos`
--

LOCK TABLES `mvt1_noer_pos` WRITE;
/*!40000 ALTER TABLE `mvt1_noer_pos` DISABLE KEYS */;
INSERT INTO `mvt1_noer_pos` VALUES ('mvt1',3,1),('EB77',3,3),('ET77',3,5);
/*!40000 ALTER TABLE `mvt1_noer_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mvt1_noer_script`
--

DROP TABLE IF EXISTS `mvt1_noer_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mvt1_noer_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mvt1_noer_script`
--

LOCK TABLES `mvt1_noer_script` WRITE;
/*!40000 ALTER TABLE `mvt1_noer_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `mvt1_noer_script` ENABLE KEYS */;
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
INSERT INTO `mvt1_pos` VALUES ('mvt1',3,1),('EB77',3,3),('ER77',5,3),('ET77',3,5),('coda_0',5,5);
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
-- Table structure for table `preshower_er_option`
--

DROP TABLE IF EXISTS `preshower_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `preshower_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `preshower_er_option`
--

LOCK TABLES `preshower_er_option` WRITE;
/*!40000 ALTER TABLE `preshower_er_option` DISABLE KEYS */;
INSERT INTO `preshower_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/pcal/pcal_%06d.A00'),('SPLITMB','2047'),('rocMask','16384');
/*!40000 ALTER TABLE `preshower_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `preshower_option`
--

DROP TABLE IF EXISTS `preshower_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `preshower_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `preshower_option`
--

LOCK TABLES `preshower_option` WRITE;
/*!40000 ALTER TABLE `preshower_option` DISABLE KEYS */;
INSERT INTO `preshower_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','16384');
/*!40000 ALTER TABLE `preshower_option` ENABLE KEYS */;
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
INSERT INTO `priority` VALUES ('ROC',11),('EB',15),('ANA',19),('ER',23),('LOG',27),('TS',-27),('ROC',11),('EB',15),('ANA',19),('ER',23),('LOG',27),('TS',-27);
/*!40000 ALTER TABLE `priority` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pro`
--

DROP TABLE IF EXISTS `pro`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pro` (
  `name` char(32) default NULL,
  `id` int(11) default NULL,
  `cmd` char(128) default NULL,
  `type` char(32) default NULL,
  `host` char(32) default NULL,
  `port` int(11) default NULL,
  `state` char(32) default NULL,
  `pid` int(11) default NULL,
  `inuse` char(32) default NULL,
  `clone` char(32) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `pro`
--

LOCK TABLES `pro` WRITE;
/*!40000 ALTER TABLE `pro` DISABLE KEYS */;
INSERT INTO `pro` VALUES ('EB5',88,'$CODA_BIN/coda_eb','EB','clon00',0,'booted',0,'5006','no'),('ER5',85,'$CODA_BIN/coda_er','ER','clon00',0,'booted',2257,'5003','no'),('clasdev',0,'rcServer','RCS','clon00',57856,'dormant',0,'','no'),('wolfram1',25,'none','ROC','wolfram1',0,'booted',0,'5002','no'),('ER2',83,'$CODA_BIN/coda_er','ER','clon00',-999,'downloaded',-999,'5007',''),('EB2',82,'$CODA_BIN/coda_eb','EB','clon00',-999,'downloaded',-999,'5005',''),('tdcftof4',24,'$CODA_BIN/coda_roc','ROC','tdcftof4',0,'configured',0,'5002','no'),('claswolf',0,'rcServer','RCS','clon00',58101,'dormant',0,'yes','no'),('pretrig3',-1,'none','ROC','pretrig3',5001,'dormant',-999,'','no'),('ER8',87,'$CODA_BIN/coda_er','ER','dcrb1',0,'downloaded',0,'5002','no'),('EB8',66,'$CODA_BIN/coda_eb','EB','dcrb1',0,'downloaded',0,'5006','no'),('adcpcal4',21,'$CODA_BIN/coda_roc','ROC','adcpcal4',0,'configured',0,'5002','no'),('level3',-1,'$CODA_BIN/coda_mon','MON','level3',0,'booted',0,'5001','no'),('clastest1',0,'rcServer','RCS','clonioc1.jlab.org',58033,'dormant',0,'yes','no'),('clastest3',0,'rcServer','RCS','clonpc3',2052,'dormant',0,'yes','no'),('pretrig1',-1,'none','ROC','pretrig1',5001,'dormant',-999,'',''),('croctest10',-27,'','TS','croctest10',5001,'booted',0,'5002','no'),('EB6',76,'$CODA_BIN/coda_eb','EB','clon00',32795,'booted',374,'5004','no'),('ER6',77,'$CODA_BIN/coda_er','ER','clon00',32932,'downloaded',1738,'5002','no'),('clastrig2',-1,'none','TS','clastrig2',5002,'downloading',-1,'5001','no'),('croctest2',0,'none','ROC','croctest2',5001,'booted',0,'5002','no'),('ctoftest',0,'rcServer','RCS','clon00',62985,'dormant',0,'yes','no'),('croctest4',-1,'','USER','croctest4',5001,'booted',0,'5001','no'),('clasprod3',0,'rcServer','RCS','adcecal3',49093,'dormant',0,'yes','no'),('dvcs2',41,'$CODA_BIN/coda_roc','ROC','dvcs2',0,'downloaded',0,'5002','no'),('dvcstrig',-1,'none','ROC','dvcstrig',5002,'booted',-999,'5002',''),('pretrig2',-1,'none','ROC','pretrig2',5001,'dormant',-999,'',''),('dccntrl',-1,'none','ROC','dccntrl',5001,'dormant',-999,'',''),('camac1',-1,'none','ROC','camac1',5001,'dormant',-999,'',''),('camac2',-1,'none','ROC','camac2',5001,'dormant',-999,'',''),('hps2gtp',40,'$CODA_BIN/coda_roc','ROC','hps2gtp',0,'paused',0,'5001','no'),('EB40',97,'$CODA_BIN/coda_eb','EB','clondaq1',-999,'configured',-999,'5004',''),('adcecal4',19,'$CODA_BIN/coda_roc','TS','adcecal4',0,'configured',0,'5005','no'),('clasftof',0,'rcServer','RCS','ftof0',54740,'dormant',0,'yes','no'),('clasprod4',0,'rcServer','RCS','adcecal4',52902,'dormant',0,'yes','no'),('primex',0,'rcServer','RCS','clon00',63228,'dormant',0,'yes','no'),('fadc1720a',0,'rcServer','RCS','clon00',62099,'dormant',0,'yes','no'),('fadc1720',0,'rcServer','RCS','clon00',56620,'dormant',0,'yes','no'),('EB4',99,'$CODA_BIN/coda_eb','EB','ftof0',-999,'configured',-999,'5004',''),('ER4',98,'$CODA_BIN/coda_er','ER','ftof0',-999,'configured',-999,'5002',''),('EB10',44,'$CODA_BIN/coda_eb','EB','svt2',0,'booted',0,'5002','no'),('ER7',43,'$CODA_BIN/coda_er','ER','clon00',0,'configured',0,'5004','no'),('EB7',54,'$CODA_BIN/coda_eb','EB','clon00',0,'configured',0,'5002','no'),('adcftof4',23,'$CODA_BIN/coda_roc','ROC','adcftof4',0,'configured',0,'5002','no'),('pcalvme',54,'$CODA_BIN/coda_roc','ROC','pcalvme',0,'booted',0,'5002','no'),('clasprod1',0,'rcServer','RCS','adcecal1',52902,'dormant',0,'yes','no'),('clastest',0,'rcServer','RCS','hps1',17876,'dormant',0,'yes','no'),('EB9',399,'$CODA_BIN/coda_eb','EB','hps1',0,'paused',0,'5004','no'),('hpstracker',-3,'$CODA_BIN/coda_roc','ROC','hpstracker',0,'booted',0,'5006','no'),('ER9',93,'$CODA_BIN/coda_er','ER','hps1',0,'booted',0,'5002','no'),('hps1',37,'','TS','hps1',0,'paused',0,'5006','no'),('hps2',39,'$CODA_BIN/coda_roc','ROC','hps2',0,'paused',0,'5002','no'),('clashps',0,'rcServer','RCS','hps1',33574,'dormant',0,'yes','no'),('svt1',-4,'none','ROC','svt1',0,'downloading',0,'5002','no'),('classvt',0,'rcServer','RCS','svt5',34260,'dormant',0,'yes','no'),('ER10',45,'$CODA_BIN/coda_er','ER','svt2',0,'booted',0,'5004','no'),('dcrb1',42,'','ROC','dcrb1',0,'downloading',0,'5008','no'),('svt2',-5,'$CODA_BIN/coda_roc','ROC','svt2',0,'downloading',0,'5002','no'),('dcrb2',2111,'$CODA_BIN/coda_roc','ROC','dcrb1',0,'configured',0,'5002','no'),('ltcc0',43,'','ROC','ltcc0',0,'downloaded',0,'5006','no'),('clasltcc',0,'rcServer','RCS','ltcc0',54740,'dormant',0,'yes','no'),('dcrb1gtp',44,'$CODA_BIN/coda_roc','ROC','dcrb1',0,'downloading',0,'5004','no'),('hps1gtp',38,'$CODA_BIN/coda_roc','ROC','hps1gtp',-999,'paused',-999,'5002',''),('EB3',61,'$CODA_BIN/coda_eb','EB','ltcc0',0,'downloaded',0,'5002','no'),('ER3',61,'$CODA_BIN/coda_er','ER','ltcc0',0,'downloaded',0,'5004','no'),('clasprod',0,'rcServer','RCS','adcecal1',49056,'dormant',0,'yes','no'),('EB1',81,'$CODA_BIN/coda_eb','EB','adcecal1',0,'downloaded',0,'5003','no'),('ER1',91,'$CODA_BIN/coda_er','ER','adcecal1',0,'booted',0,'5002','no'),('adcecal1',1,'','TS','adcecal1',0,'booted',0,'5006','no'),('adcpcal1',3,'$CODA_BIN/coda_roc','ROC','adcpcal1',0,'booted',0,'5002','no'),('claspc',0,'rcServer','RCS','pcal0',17876,'dormant',0,'yes','no'),('tdcecal1',2,'$CODA_BIN/coda_roc','ROC','tdcecal1',0,'booted',0,'5002','no'),('EB0',60,'','EB','clonioc1',-999,'downloaded',-999,'5002',''),('ROC0',-33,'$CODA_BIN/coda_roc','ROC','zedboard1',0,'downloaded',0,'5001','no'),('clasdcrb',0,'rcServer','RCS','dcrb1',30164,'dormant',0,'yes','no'),('ER0',44,'$CODA_BIN/coda_er','ER','clonioc1',0,'downloaded',0,'5003','no'),('trig1',-37,'$CODA_BIN/coda_roc','ROC','trig1',0,'configured',0,'5001','no'),('clastest0',0,'rcServer','RCS','clonpc0.jlab.org',49144,'dormant',0,'yes','no'),('EB11',91,'$CODA_BIN/coda_eb','EB','adcecal1',0,'downloaded',0,'5002','no'),('adcecal5',25,'','TS','adcecal5',0,'configured',0,'5006','no'),('tdcpcal1',4,'$CODA_BIN/coda_roc','ROC','tdcpcal1',0,'booted',0,'5002','no'),('adcftof1',5,'$CODA_BIN/coda_roc','ROC','adcftof1',0,'booted',0,'5002','no'),('tdcftof1',6,'$CODA_BIN/coda_roc','ROC','tdcftof1',0,'booted',0,'5002','no'),('ER11',81,'$CODA_BIN/coda_er','ER','adcecal1',0,'booted',0,'5004','no'),('tdcecal5',26,'$CODA_BIN/coda_roc','ROC','tdcecal5',0,'booted',0,'5002','no'),('ER15',75,'$CODA_BIN/coda_er','ER','adcecal5',0,'configured',0,'5001','no'),('EB15',65,'$CODA_BIN/coda_eb','EB','adcecal5',0,'configured',0,'5003','no'),('ctoftest1',0,'','ROC','ctoftest1',-999,'booted',-999,'5002',''),('adcpcal5',27,'$CODA_BIN/coda_roc','ROC','adcpcal5',0,'booted',0,'5002','no'),('tdcpcal5',28,'$CODA_BIN/coda_roc','ROC','tdcpcal5',0,'booted',0,'5002','no'),('adcftof5',29,'$CODA_BIN/coda_roc','ROC','adcftof5',0,'booted',0,'5002','no'),('tdcftof5',30,'$CODA_BIN/coda_roc','ROC','tdcftof5',0,'booted',0,'5002','no'),('tdcpcal4',22,'$CODA_BIN/coda_roc','ROC','tdcpcal4',0,'configured',0,'5002','no'),('tdcecal4',20,'$CODA_BIN/coda_roc','ROC','tdcecal4',0,'configured',0,'5002','no'),('ER14',84,'$CODA_BIN/coda_er','ER','adcecal4',0,'booted',0,'5004','no'),('EB14',94,'$CODA_BIN/coda_eb','EB','adcecal4',0,'downloaded',0,'5001','no'),('clasprod5',0,'rcServer','RCS','adcecal5',49059,'dormant',0,'yes','no'),('adcecal3',13,'','TS','adcecal3',0,'configured',0,'5004','no'),('tdcecal3',14,'$CODA_BIN/coda_roc','ROC','tdcecal3',0,'configured',0,'5002','no'),('adcpcal3',15,'$CODA_BIN/coda_roc','ROC','adcpcal3',0,'configured',0,'5002','no'),('tdcpcal3',16,'$CODA_BIN/coda_roc','ROC','tdcpcal3',0,'configured',0,'5002','no'),('tdcftof3',18,'$CODA_BIN/coda_roc','ROC','tdcftof3',0,'configured',0,'5002','no'),('adcftof3',17,'$CODA_BIN/coda_roc','ROC','adcftof3',0,'configured',0,'5002','no'),('EB13',93,'$CODA_BIN/coda_eb','EB','adcecal3',0,'configured',0,'5006','no'),('tdcpcal6',34,'$CODA_BIN/coda_roc','ROC','tdcpcal6',0,'configured',0,'5002','no'),('EB12',77,'$CODA_BIN/coda_eb','EB','adcecal2',0,'downloaded',0,'5004','no'),('adcecal2',7,'$CODA_BIN/coda_roc','TS','adcecal2',0,'booted',0,'5006','no'),('tdcecal2',8,'$CODA_BIN/coda_roc','ROC','tdcecal2',0,'configured',0,'5002','no'),('adcpcal2',9,'$CODA_BIN/coda_roc','ROC','adcpcal2',0,'configured',0,'5002','no'),('tdcpcal2',10,'$CODA_BIN/coda_roc','ROC','tdcpcal2',0,'booted',0,'5002','no'),('adcftof2',11,'$CODA_BIN/coda_roc','ROC','adcftof2',0,'configured',0,'5002','no'),('tdcftof2',12,'$CODA_BIN/coda_roc','ROC','tdcftof2',0,'configured',0,'5002','no'),('adcpcal6',33,'$CODA_BIN/coda_roc','ROC','adcpcal6',0,'configured',0,'5002','no'),('tdcecal6',32,'$CODA_BIN/coda_roc','ROC','tdcecal6',0,'dormant',0,'no','no'),('adcecal6',31,'$CODA_BIN/coda_roc','TS','adcecal6',0,'configured',0,'5004','no'),('tdcftof6',36,'$CODA_BIN/coda_roc','ROC','tdcftof6',0,'configured',0,'5002','no'),('adcftof6',35,'$CODA_BIN/coda_roc','ROC','adcftof6',0,'configured',0,'5002','no'),('EB16',78,'$CODA_BIN/coda_eb','EB','adcecal6',0,'configured',0,'5006','no'),('clasprod2',0,'rcServer','RCS','adcecal2',49131,'dormant',0,'yes','no'),('clasprod6',0,'rcServer','RCS','adcecal6',49148,'dormant',0,'yes','no'),('ER12',92,'$CODA_BIN/coda_er','ER','adcecal2',0,'booted',0,'5002','no'),('ER13',94,'$CODA_BIN/coda_er','ER','adcecal3',0,'dormant',0,'no','no'),('ER16',96,'$CODA_BIN/coda_er','ER','adcecal6',0,'dormant',0,'no','no');
/*!40000 ALTER TABLE `pro` ENABLE KEYS */;
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
INSERT INTO `process` VALUES ('EB5',88,'$CODA_BIN/coda_eb','EB','clon00',0,'booted',0,'5006','no'),('ER5',85,'$CODA_BIN/coda_er','ER','clon00',0,'booted',2257,'5003','no'),('clasdev',0,'rcServer','RCS','clon00',57856,'dormant',0,'','no'),('wolfram1',25,'none','ROC','wolfram1',0,'booted',0,'5002','no'),('ER2',83,'$CODA_BIN/coda_er','ER','clon00',-999,'downloaded',-999,'5007',''),('EB2',82,'$CODA_BIN/coda_eb','EB','clon00',-999,'downloaded',-999,'5005',''),('tdcftof4',24,'$CODA_BIN/coda_roc','ROC','tdcftof4',0,'active',0,'5002','no'),('claswolf',0,'rcServer','RCS','clon00',58101,'dormant',0,'yes','no'),('pretrig3',-1,'none','ROC','pretrig3',5001,'dormant',-999,'','no'),('ER8',87,'$CODA_BIN/coda_er','ER','dcrb1',0,'downloaded',0,'5002','no'),('EB8',66,'$CODA_BIN/coda_eb','EB','dcrb1',0,'downloaded',0,'5006','no'),('adcpcal4',21,'$CODA_BIN/coda_roc','ROC','adcpcal4',0,'active',0,'5002','no'),('level3',-1,'$CODA_BIN/coda_mon','MON','level3',0,'booted',0,'5001','no'),('clastest1',0,'rcServer','RCS','clonioc1.jlab.org',58033,'dormant',0,'yes','no'),('clastest3',0,'rcServer','RCS','clonpc3',2052,'dormant',0,'yes','no'),('pretrig1',-1,'none','ROC','pretrig1',5001,'dormant',-999,'',''),('croctest10',-27,'','TS','croctest10',5001,'booted',0,'5002','no'),('EB6',76,'$CODA_BIN/coda_eb','EB','clon00',32795,'downloaded',374,'5004','no'),('ER6',77,'$CODA_BIN/coda_er','ER','clon00',32932,'downloaded',1738,'5002','no'),('clastrig2',-1,'none','TS','clastrig2',5002,'downloading',-1,'5001','no'),('croctest2',0,'none','ROC','croctest2',5001,'booted',0,'5002','no'),('ctoftest',0,'rcServer','RCS','ctof1',61779,'dormant',0,'yes','no'),('croctest4',-1,'','USER','croctest4',5001,'booted',0,'5001','no'),('clasprod3',0,'rcServer','RCS','adcecal3',49093,'dormant',0,'yes','no'),('dvcs2',41,'$CODA_BIN/coda_roc','ROC','dvcs2',0,'downloaded',0,'5002','no'),('dvcstrig',-1,'none','ROC','dvcstrig',5002,'booted',-999,'5002',''),('pretrig2',-1,'none','ROC','pretrig2',5001,'dormant',-999,'',''),('dccntrl',-1,'none','ROC','dccntrl',5001,'dormant',-999,'',''),('camac1',-1,'none','ROC','camac1',5001,'dormant',-999,'',''),('camac2',-1,'none','ROC','camac2',5001,'dormant',-999,'',''),('hps2gtp',40,'$CODA_BIN/coda_roc','ROC','hps2gtp',0,'paused',0,'5001','no'),('clasprod1',0,'rcServer','RCS','adcecal1',52902,'dormant',0,'yes','no'),('EB40',97,'$CODA_BIN/coda_eb','EB','clondaq1',-999,'configured',-999,'5004',''),('adcecal4',19,'$CODA_BIN/coda_roc','TS','adcecal4',0,'active',0,'5006','no'),('clasftof',0,'rcServer','RCS','ftof0',54740,'dormant',0,'yes','no'),('clasprod4',0,'rcServer','RCS','adcecal4',52902,'dormant',0,'yes','no'),('primex',0,'rcServer','RCS','clon00',63228,'dormant',0,'yes','no'),('fadc1720a',0,'rcServer','RCS','clon00',62099,'dormant',0,'yes','no'),('fadc1720',0,'rcServer','RCS','clon00',56620,'dormant',0,'yes','no'),('EB4',99,'$CODA_BIN/coda_eb','EB','ftof0',-999,'configured',-999,'5004',''),('ER4',98,'$CODA_BIN/coda_er','ER','ftof0',-999,'configured',-999,'5002',''),('EB10',44,'$CODA_BIN/coda_eb','EB','svt2',0,'booted',0,'5002','no'),('ER7',43,'$CODA_BIN/coda_er','ER','clon00',0,'configured',0,'5004','no'),('EB7',54,'$CODA_BIN/coda_eb','EB','clon00',0,'configured',0,'5002','no'),('adcftof4',23,'$CODA_BIN/coda_roc','ROC','adcftof4',0,'active',0,'5002','no'),('pcalvme',54,'$CODA_BIN/coda_roc','ROC','pcalvme',0,'booted',0,'5002','no'),('clastest',0,'rcServer','RCS','hps1',17876,'dormant',0,'yes','no'),('EB9',399,'$CODA_BIN/coda_eb','EB','hps1',0,'paused',0,'5004','no'),('hpstracker',-3,'$CODA_BIN/coda_roc','ROC','hpstracker',0,'booted',0,'5006','no'),('ER9',93,'$CODA_BIN/coda_er','ER','hps1',0,'booted',0,'5002','no'),('hps1',37,'','TS','hps1',0,'paused',0,'5005','no'),('hps2',39,'$CODA_BIN/coda_roc','ROC','hps2',0,'paused',0,'5002','no'),('clashps',0,'rcServer','RCS','hps1',33574,'dormant',0,'yes','no'),('svt1',-4,'none','ROC','svt1',0,'downloading',0,'5002','no'),('classvt',0,'rcServer','RCS','svt5',34260,'dormant',0,'yes','no'),('ER10',47,'$CODA_BIN/coda_er','ER','svt2',0,'booted',0,'5004','no'),('dcrb1',42,'','ROC','dcrb1',0,'downloading',0,'5008','no'),('svt2',-5,'$CODA_BIN/coda_roc','ROC','svt2',0,'downloading',0,'5002','no'),('dcrb22',2111,'$CODA_BIN/coda_roc','ROC','dcrb1',0,'configured',0,'5002','no'),('ltcc0',43,'','ROC','ltcc0',0,'downloaded',0,'5006','no'),('clasltcc',0,'rcServer','RCS','ltcc0',54740,'dormant',0,'yes','no'),('dcrb1gtp',44,'$CODA_BIN/coda_roc','ROC','dcrb1',0,'downloading',0,'5004','no'),('hps1gtp',38,'$CODA_BIN/coda_roc','ROC','hps1gtp',-999,'paused',-999,'5002',''),('EB3',61,'$CODA_BIN/coda_eb','EB','ltcc0',0,'downloaded',0,'5002','no'),('ER3',61,'$CODA_BIN/coda_er','ER','ltcc0',0,'downloaded',0,'5004','no'),('clasprod',0,'rcServer','RCS','adcecal1',49056,'dormant',0,'yes','no'),('EB1',81,'$CODA_BIN/coda_eb','EB','adcecal1',0,'downloaded',0,'5003','no'),('ER1',91,'$CODA_BIN/coda_er','ER','adcecal1',0,'booted',0,'5002','no'),('adcecal1',1,'','TS','adcecal1',0,'active',0,'5005','no'),('zedboard1',45,'$CODA_BIN/coda_roc','ROC','zedboard1',0,'configured',0,'5001','no'),('adcpcal1',3,'$CODA_BIN/coda_roc','ROC','adcpcal1',0,'booted',0,'5002','no'),('claspc',0,'rcServer','RCS','pcal0',17876,'dormant',0,'yes','no'),('tdcecal1',2,'$CODA_BIN/coda_roc','ROC','tdcecal1',0,'booted',0,'5002','no'),('EB0',60,'','EB','clonpc0',-999,'configured',-999,'5004',''),('clasdcrb',0,'rcServer','RCS','dcrb1',30164,'dormant',0,'yes','no'),('ER0',44,'$CODA_BIN/coda_er','ER','clonpc0',0,'configured',0,'5002','no'),('trig1',-37,'$CODA_BIN/coda_roc','ROC','trig1',0,'configured',0,'5001','no'),('clastest0',0,'rcServer','RCS','clonpc0.jlab.org',49047,'dormant',0,'yes','no'),('EB11',91,'$CODA_BIN/coda_eb','EB','adcecal1',0,'active',0,'5004','no'),('adcecal5',25,'coda_roc_gef','TS','adcecal5',0,'downloaded',0,'5005','no'),('tdcpcal1',4,'$CODA_BIN/coda_roc','ROC','tdcpcal1',0,'booted',0,'5002','no'),('adcftof1',5,'$CODA_BIN/coda_roc','ROC','adcftof1',0,'booted',0,'5002','no'),('tdcftof1',6,'$CODA_BIN/coda_roc','ROC','tdcftof1',0,'booted',0,'5002','no'),('ER11',81,'$CODA_BIN/coda_er','ER','adcecal1',0,'booted',0,'5002','no'),('tdcecal5',26,'coda_roc_gef','ROC','tdcecal5',0,'downloaded',0,'5002','no'),('ER15',75,'$CODA_BIN/coda_er','ER','adcecal5',0,'booted',0,'5004','no'),('EB15',65,'coda_ebc','EB','adcecal5',0,'downloaded',0,'5003','no'),('ctoftest1',0,'','ROC','ctoftest1',-999,'downloaded',-999,'5002',''),('adcpcal5',27,'coda_roc_gef','ROC','adcpcal5',0,'downloaded',0,'5002','no'),('tdcpcal5',28,'coda_roc_gef','ROC','tdcpcal5',0,'downloaded',0,'5002','no'),('adcftof5',29,'coda_roc_gef','ROC','adcftof5',0,'downloaded',0,'5002','no'),('tdcftof5',30,'coda_roc_gef','ROC','tdcftof5',0,'downloaded',0,'5002','no'),('tdcpcal4',22,'$CODA_BIN/coda_roc','ROC','tdcpcal4',0,'active',0,'5002','no'),('tdcecal4',20,'$CODA_BIN/coda_roc','ROC','tdcecal4',0,'active',0,'5002','no'),('ER14',84,'$CODA_BIN/coda_er','ER','adcecal4',0,'booted',0,'5002','no'),('EB14',94,'$CODA_BIN/coda_eb','EB','adcecal4',0,'active',0,'5004','no'),('clasprod5',0,'rcServer','RCS','adcecal5',49059,'dormant',0,'yes','no'),('adcecal3',13,'','TS','adcecal3',0,'active',0,'5004','no'),('tdcecal3',14,'$CODA_BIN/coda_roc','ROC','tdcecal3',0,'active',0,'5002','no'),('adcpcal3',15,'$CODA_BIN/coda_roc','ROC','adcpcal3',0,'active',0,'5002','no'),('tdcpcal3',16,'$CODA_BIN/coda_roc','ROC','tdcpcal3',0,'active',0,'5002','no'),('tdcftof3',18,'$CODA_BIN/coda_roc','ROC','tdcftof3',0,'active',0,'5002','no'),('adcftof3',17,'$CODA_BIN/coda_roc','ROC','adcftof3',0,'active',0,'5002','no'),('EB13',93,'$CODA_BIN/coda_eb','EB','adcecal3',0,'active',0,'5002','no'),('tdcpcal6',34,'$CODA_BIN/coda_roc','ROC','tdcpcal6',0,'configured',0,'5002','no'),('EB12',77,'$CODA_BIN/coda_eb','EB','adcecal2',0,'configured',0,'5002','no'),('adcecal2',7,'$CODA_BIN/coda_roc','TS','adcecal2',0,'configured',0,'5006','no'),('tdcecal2',8,'$CODA_BIN/coda_roc','ROC','tdcecal2',0,'configured',0,'5002','no'),('adcpcal2',9,'$CODA_BIN/coda_roc','ROC','adcpcal2',0,'configured',0,'5002','no'),('tdcpcal2',10,'$CODA_BIN/coda_roc','ROC','tdcpcal2',0,'configured',0,'5002','no'),('adcftof2',11,'$CODA_BIN/coda_roc','ROC','adcftof2',0,'configured',0,'5002','no'),('tdcftof2',12,'$CODA_BIN/coda_roc','ROC','tdcftof2',0,'configured',0,'5002','no'),('adcpcal6',33,'$CODA_BIN/coda_roc','ROC','adcpcal6',0,'configured',0,'5002','no'),('tdcecal6',32,'$CODA_BIN/coda_roc','ROC','tdcecal6',0,'dormant',0,'no','no'),('adcecal6',31,'$CODA_BIN/coda_roc','TS','adcecal6',0,'configured',0,'5002','no'),('tdcftof6',36,'$CODA_BIN/coda_roc','ROC','tdcftof6',0,'configured',0,'5002','no'),('adcftof6',35,'$CODA_BIN/coda_roc','ROC','adcftof6',0,'configured',0,'5002','no'),('EB16',78,'$CODA_BIN/coda_eb','EB','adcecal6',0,'configured',0,'5006','no'),('clasprod2',0,'rcServer','RCS','adcecal2',49131,'dormant',0,'yes','no'),('clasprod6',0,'rcServer','RCS','adcecal6',49148,'dormant',0,'yes','no'),('ER12',92,'$CODA_BIN/coda_er','ER','adcecal2',0,'booted',0,'5003','no'),('ER13',94,'$CODA_BIN/coda_er','ER','adcecal3',0,'booted',0,'5006','no'),('ER16',96,'$CODA_BIN/coda_er','ER','adcecal6',0,'booted',0,'5003','no'),('fttest',0,'rcServer','RCS','jlab12daq1.jlab.org',60108,'dormant',0,'yes','no'),('ET15',22,'coda_et','ET','adcecal5',11111,'booted',0,'5006','no'),('adcft1',70,'coda_roc_gef','ROC','adcft1',-999,'',-999,'',''),('adcft2',71,'coda_roc_gef','ROC','adcft2',-999,'',-999,'',''),('EB81',81,'coda_ebc','EB','jlab12daq1',-999,'',-999,'',''),('adcft3',72,'coda_roc_gef','ROC','adcft3',-999,'',-999,'',''),('ER81',81,'coda_erc','ER','jlab12daq1',-999,'',-999,'',''),('ET81',81,'coda_et','ET','jlab12daq1',-999,'',-999,'',''),('EB78',78,'coda_ebc','EB','adcft1',-999,'',-999,'',''),('ER78',78,'coda_erc','ER','adcft1',-999,'',-999,'',''),('ET78',78,'coda_et','ET','adcft1',-999,'',-999,'',''),('EB79',79,'coda_ebc','EB','mmft1',-999,'',-999,'',''),('ER79',79,'coda_erc','ER','mmft1',-999,'',-999,'',''),('ET79',79,'coda_et','ET','mmft1',-999,'',-999,'',''),('EB80',80,'coda_ebc','EB','adcft2',-999,'',-999,'',''),('ER80',80,'coda_erc','ER','adcft2',-999,'',-999,'',''),('ET80',80,'coda_et','ET','adcft2',-999,'',-999,'',''),('EB82',82,'coda_ebc','EB','adcft3',-999,'',-999,'',''),('ER82',82,'coda_erc','ER','adcft3',-999,'',-999,'',''),('ET82',82,'coda_et','ET','adcft3',-999,'',-999,'',''),('ctof1',68,'coda_roc_gef','ROC','ctof1',-999,'booted',-999,'5005',''),('EB66',66,'coda_ebc','EB','ctof1',-999,'booted',-999,'5007',''),('ER66',66,'coda_erc','ER','ctof1',-999,'booted',-999,'5002',''),('ET66',66,'coda_et','ET','ctof1',11111,'booted',-999,'5008',''),('EB23',23,'coda_ebc','EB','dcrb2',-999,'',-999,'',''),('ER23',23,'coda_erc','ER','dcrb2',-999,'',-999,'',''),('ET223',223,'coda_et','ET','dcrb2',-999,'',-999,'',''),('dcrb2',67,'coda_roc_gef','ROC','dcrb2',-999,'',-999,'',''),('ET3',33,'coda_et','ET','ltcc0',0,'dormant',0,'no','no');
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
INSERT INTO `runTypes` VALUES ('wolf_er',0,'yes',''),('wolf',1,'no',''),('dvcs_er',2,'yes',''),('sector6_er',3,'yes',''),('sector1',4,'yes',''),('sector2_er',5,'yes',''),('sector3_er',6,'no',''),('test0',7,'yes',''),('sector6',8,'yes',''),('ltcc0',9,'no',''),('ctoftest',10,'yes',''),('ctofpc1',11,'no',''),('adcecal1_er',12,'no',''),('ltcc0_noer',13,'no',''),('sector1_er',14,'yes',''),('hpsgtp',15,'yes',''),('adcecal1',16,'yes',''),('sector2',17,'yes',''),('sector4_er',18,'yes',''),('dcrb1',19,'yes',''),('adcecal5',20,'no',''),('sector5',21,'no',''),('sector4',22,'yes',''),('sector5_er',23,'yes',''),('ECAL',24,'no',''),('hpsgtp_er',25,'yes',''),('sector3',26,'yes',''),('adcft1_test',27,'no',''),('adcft2_test',28,'no',''),('adcft3_test',29,'no',''),('ctof1',30,'yes',''),('dcrb2',31,'no',''),('FT',32,'yes',''),('FT_NOER',33,'no',''),('ft',34,'no',''),('FT_test',35,'no','');
/*!40000 ALTER TABLE `runTypes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sec1_option`
--

DROP TABLE IF EXISTS `sec1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sec1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sec1_option`
--

LOCK TABLES `sec1_option` WRITE;
/*!40000 ALTER TABLE `sec1_option` DISABLE KEYS */;
INSERT INTO `sec1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000000 0x00080150');
/*!40000 ALTER TABLE `sec1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1`
--

DROP TABLE IF EXISTS `sector1`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1` (
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
-- Dumping data for table `sector1`
--

LOCK TABLES `sector1` WRITE;
/*!40000 ALTER TABLE `sector1` DISABLE KEYS */;
INSERT INTO `sector1` VALUES ('adcecal1','{fadc1_master.so usr} {fadc2.so usr} ','','EB11:adcecal1','yes','','1'),('tdcecal1','{tdc1_slave.so usr} {tdc2.so usr} ','','EB11:adcecal1','yes','adcpcal1','2'),('adcpcal1','{fadc1_slave.so usr} {fadc2.so usr} ','','EB11:adcecal1','no','tdcpcal1','3'),('tdcpcal1','{tdc1_slave.so usr} {tdc2.so usr} ','','EB11:adcecal1','no','adcftof1','4'),('adcftof1','{fadc1_slave.so usr} {fadc2.so usr} ','','EB11:adcecal1','no','tdcftof1','5'),('tdcftof1','{tdc1_slave.so usr} {tdc2.so usr} ','','EB11:adcecal1','no','','6'),('EB11','{CODA} {CODA} ','adcecal1:adcecal1 tdcecal1:tdcecal1 adcpcal1:adcpcal1 tdcpcal1:tdcpcal1 adcftof1:adcftof1 tdcftof1:tdcftof1','','yes','','no');
/*!40000 ALTER TABLE `sector1` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_er`
--

DROP TABLE IF EXISTS `sector1_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_er` (
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
-- Dumping data for table `sector1_er`
--

LOCK TABLES `sector1_er` WRITE;
/*!40000 ALTER TABLE `sector1_er` DISABLE KEYS */;
INSERT INTO `sector1_er` VALUES ('adcecal1','{fadc1_master.so usr} {fadc2.so usr} ','','EB11:adcecal1','yes','','1'),('tdcecal1','{tdc1_slave.so usr} {tdc2.so usr} ','','EB11:adcecal1','yes','adcpcal1','2'),('adcpcal1','{fadc1_slave.so usr} {fadc2.so usr} ','','EB11:adcecal1','no','tdcpcal1','3'),('tdcpcal1','{tdc1_slave.so usr} {tdc2.so usr} ','','EB11:adcecal1','no','adcftof1','4'),('adcftof1','{fadc1_slave.so usr} {fadc2.so usr} ','','EB11:adcecal1','no','tdcftof1','5'),('tdcftof1','{tdc1_slave.so usr} {tdc2.so usr} ','','EB11:adcecal1','no','','6'),('EB11','{CODA} {CODA} ','adcecal1:adcecal1 tdcecal1:tdcecal1 adcpcal1:adcpcal1 tdcpcal1:tdcpcal1 adcftof1:adcftof1 tdcftof1:tdcftof1','','yes','','no'),('ER11','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER11:adcecal1','','yes','','no');
/*!40000 ALTER TABLE `sector1_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_er_option`
--

DROP TABLE IF EXISTS `sector1_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector1_er_option`
--

LOCK TABLES `sector1_er_option` WRITE;
/*!40000 ALTER TABLE `sector1_er_option` DISABLE KEYS */;
INSERT INTO `sector1_er_option` VALUES ('dataLimit','0'),('eventLimit','10000'),('tokenInterval','64'),('dataFile','/work/stage_in/sector1'),('rocMask','0x00000000 0x00000000 0x00000000 0x0000007e'),('confFile','none');
/*!40000 ALTER TABLE `sector1_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_er_pos`
--

DROP TABLE IF EXISTS `sector1_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector1_er_pos`
--

LOCK TABLES `sector1_er_pos` WRITE;
/*!40000 ALTER TABLE `sector1_er_pos` DISABLE KEYS */;
INSERT INTO `sector1_er_pos` VALUES ('adcecal1',2,1),('tdcecal1',3,1),('adcpcal1',5,1),('tdcpcal1',6,1),('adcftof1',8,1),('tdcftof1',9,1),('EB11',2,3),('ER11',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `sector1_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_er_script`
--

DROP TABLE IF EXISTS `sector1_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector1_er_script`
--

LOCK TABLES `sector1_er_script` WRITE;
/*!40000 ALTER TABLE `sector1_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector1_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_option`
--

DROP TABLE IF EXISTS `sector1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector1_option`
--

LOCK TABLES `sector1_option` WRITE;
/*!40000 ALTER TABLE `sector1_option` DISABLE KEYS */;
INSERT INTO `sector1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','test.dat'),('rocMask','0x00000000 0x00000000 0x00000000 0x0000007e'),('confFile','none');
/*!40000 ALTER TABLE `sector1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_pos`
--

DROP TABLE IF EXISTS `sector1_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector1_pos`
--

LOCK TABLES `sector1_pos` WRITE;
/*!40000 ALTER TABLE `sector1_pos` DISABLE KEYS */;
INSERT INTO `sector1_pos` VALUES ('adcecal1',2,1),('tdcecal1',3,1),('adcpcal1',5,1),('tdcpcal1',6,1),('adcftof1',8,1),('tdcftof1',9,1),('EB11',2,3);
/*!40000 ALTER TABLE `sector1_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector1_script`
--

DROP TABLE IF EXISTS `sector1_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector1_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector1_script`
--

LOCK TABLES `sector1_script` WRITE;
/*!40000 ALTER TABLE `sector1_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector1_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2`
--

DROP TABLE IF EXISTS `sector2`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2` (
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
-- Dumping data for table `sector2`
--

LOCK TABLES `sector2` WRITE;
/*!40000 ALTER TABLE `sector2` DISABLE KEYS */;
INSERT INTO `sector2` VALUES ('adcecal2','{fadc1_master.so usr} {fadc2.so usr} ','','EB12:adcecal2','yes','','7'),('tdcecal2','{tdc1_slave.so usr} {tdc2.so usr} ','','EB12:adcecal2','yes','adcpcal2','8'),('adcpcal2','{fadc1_slave.so usr} {fadc2.so usr} ','','EB12:adcecal2','no','tdcpcal2','9'),('tdcpcal2','{tdc1_slave.so usr} {tdc2.so usr} ','','EB12:adcecal2','no','adcftof2','10'),('adcftof2','{fadc1_slave.so usr} {fadc2.so usr} ','','EB12:adcecal2','no','tdcftof2','11'),('tdcftof2','{tdc1_slave.so usr} {tdc2.so usr} ','','EB12:adcecal2','no','','12'),('EB12','{CODA} {CODA} ','adcecal2:adcecal2 tdcecal2:tdcecal2 adcpcal2:adcpcal2 tdcpcal2:tdcpcal2 adcftof2:adcftof2 tdcftof2:tdcftof2','','yes','','no');
/*!40000 ALTER TABLE `sector2` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_er`
--

DROP TABLE IF EXISTS `sector2_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_er` (
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
-- Dumping data for table `sector2_er`
--

LOCK TABLES `sector2_er` WRITE;
/*!40000 ALTER TABLE `sector2_er` DISABLE KEYS */;
INSERT INTO `sector2_er` VALUES ('adcecal2','{fadc1_master.so usr} {fadc2.so usr} ','','EB12:adcecal2','yes','','7'),('tdcecal2','{tdc1_slave.so usr} {tdc2.so usr} ','','EB12:adcecal2','yes','tdcpcal2','8'),('tdcpcal2','{tdc1_slave.so usr} {tdc2.so usr} ','','EB12:adcecal2','no','adcftof2','10'),('adcftof2','{fadc1_slave.so usr} {fadc2.so usr} ','','EB12:adcecal2','no','tdcftof2','11'),('tdcftof2','{tdc1_slave.so usr} {tdc2.so usr} ','','EB12:adcecal2','no','','12'),('EB12','{CODA} {CODA} ','adcecal2:adcecal2 tdcecal2:tdcecal2 tdcpcal2:tdcpcal2 adcftof2:adcftof2 tdcftof2:tdcftof2','','yes','','no'),('ER12','{CODA}  ','','coda_0','yes','','no'),('coda_0','  ','ER12:adcecal2','','yes','','no');
/*!40000 ALTER TABLE `sector2_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_er_option`
--

DROP TABLE IF EXISTS `sector2_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector2_er_option`
--

LOCK TABLES `sector2_er_option` WRITE;
/*!40000 ALTER TABLE `sector2_er_option` DISABLE KEYS */;
INSERT INTO `sector2_er_option` VALUES ('dataLimit','0'),('eventLimit','10000'),('tokenInterval','64'),('dataFile','/work/stage_in/sector2'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00001d80');
/*!40000 ALTER TABLE `sector2_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_er_pos`
--

DROP TABLE IF EXISTS `sector2_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector2_er_pos`
--

LOCK TABLES `sector2_er_pos` WRITE;
/*!40000 ALTER TABLE `sector2_er_pos` DISABLE KEYS */;
INSERT INTO `sector2_er_pos` VALUES ('adcecal2',2,1),('tdcecal2',3,1),('tdcpcal2',6,1),('adcftof2',8,1),('tdcftof2',9,1),('EB12',2,3),('ER12',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `sector2_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_er_script`
--

DROP TABLE IF EXISTS `sector2_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector2_er_script`
--

LOCK TABLES `sector2_er_script` WRITE;
/*!40000 ALTER TABLE `sector2_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector2_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_option`
--

DROP TABLE IF EXISTS `sector2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector2_option`
--

LOCK TABLES `sector2_option` WRITE;
/*!40000 ALTER TABLE `sector2_option` DISABLE KEYS */;
INSERT INTO `sector2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x00001f80');
/*!40000 ALTER TABLE `sector2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_pos`
--

DROP TABLE IF EXISTS `sector2_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector2_pos`
--

LOCK TABLES `sector2_pos` WRITE;
/*!40000 ALTER TABLE `sector2_pos` DISABLE KEYS */;
INSERT INTO `sector2_pos` VALUES ('adcecal2',2,1),('tdcecal2',3,1),('adcpcal2',5,1),('tdcpcal2',6,1),('adcftof2',8,1),('tdcftof2',9,1),('EB12',2,3);
/*!40000 ALTER TABLE `sector2_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector2_script`
--

DROP TABLE IF EXISTS `sector2_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector2_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector2_script`
--

LOCK TABLES `sector2_script` WRITE;
/*!40000 ALTER TABLE `sector2_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector2_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3`
--

DROP TABLE IF EXISTS `sector3`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3` (
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
-- Dumping data for table `sector3`
--

LOCK TABLES `sector3` WRITE;
/*!40000 ALTER TABLE `sector3` DISABLE KEYS */;
INSERT INTO `sector3` VALUES ('adcecal3','{fadc1_master.so usr} {fadc2.so usr} ','','EB13:adcecal3','yes','tdcecal3','13'),('tdcecal3','{tdc1_slave.so usr} {tdc2.so usr} ','','EB13:adcecal3','no','adcpcal3','14'),('adcpcal3','{fadc1_slave.so usr} {fadc2.so usr} ','','EB13:adcecal3','no','tdcpcal3','15'),('tdcpcal3','{tdc1_slave.so usr} {tdc2.so usr} ','','EB13:adcecal3','no','adcftof3','16'),('adcftof3','{fadc1_slave.so usr} {fadc2.so usr} ','','EB13:adcecal3','no','tdcftof3','17'),('tdcftof3','{tdc1_slave.so usr} {tdc2.so usr} ','','EB13:adcecal3','no','','18'),('EB13','{CODA} {CODA} ','adcecal3:adcecal3 tdcecal3:tdcecal3 adcpcal3:adcpcal3 tdcpcal3:tdcpcal3 adcftof3:adcftof3 tdcftof3:tdcftof3','','yes','','no');
/*!40000 ALTER TABLE `sector3` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_er`
--

DROP TABLE IF EXISTS `sector3_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_er` (
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
-- Dumping data for table `sector3_er`
--

LOCK TABLES `sector3_er` WRITE;
/*!40000 ALTER TABLE `sector3_er` DISABLE KEYS */;
INSERT INTO `sector3_er` VALUES ('adcecal3','{fadc1_master.so usr} {fadc2.so usr} ','','EB13:adcecal3','yes','','13'),('tdcecal3','{tdc1_slave.so usr} {tdc2.so usr} ','','EB13:adcecal3','yes','adcpcal3','14'),('adcpcal3','{fadc1_slave.so usr} {fadc2.so usr} ','','EB13:adcecal3','no','tdcpcal3','15'),('tdcpcal3','{tdc1_slave.so usr} {tdc2.so usr} ','','EB13:adcecal3','no','adcftof3','16'),('adcftof3','{fadc1_slave.so usr} {fadc2.so usr} ','','EB13:adcecal3','no','','17'),('EB13','{CODA} {CODA} ','adcecal3:adcecal3 tdcecal3:tdcecal3 adcpcal3:adcpcal3 tdcpcal3:tdcpcal3 adcftof3:adcftof3','','yes','','no'),('ER13','{CODA}  ','','coda_1','yes','','no'),('coda_1','  ','ER13:adcecal3','','yes','','no');
/*!40000 ALTER TABLE `sector3_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_er_option`
--

DROP TABLE IF EXISTS `sector3_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector3_er_option`
--

LOCK TABLES `sector3_er_option` WRITE;
/*!40000 ALTER TABLE `sector3_er_option` DISABLE KEYS */;
INSERT INTO `sector3_er_option` VALUES ('dataLimit','0'),('eventLimit','10000'),('tokenInterval','64'),('dataFile','/work/stage_in/sector3'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x0003e000');
/*!40000 ALTER TABLE `sector3_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_er_pos`
--

DROP TABLE IF EXISTS `sector3_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector3_er_pos`
--

LOCK TABLES `sector3_er_pos` WRITE;
/*!40000 ALTER TABLE `sector3_er_pos` DISABLE KEYS */;
INSERT INTO `sector3_er_pos` VALUES ('adcecal3',2,1),('tdcecal3',3,1),('adcpcal3',5,1),('tdcpcal3',6,1),('adcftof3',8,1),('EB13',2,3),('ER13',4,3),('coda_1',4,5);
/*!40000 ALTER TABLE `sector3_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_er_script`
--

DROP TABLE IF EXISTS `sector3_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector3_er_script`
--

LOCK TABLES `sector3_er_script` WRITE;
/*!40000 ALTER TABLE `sector3_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector3_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_option`
--

DROP TABLE IF EXISTS `sector3_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector3_option`
--

LOCK TABLES `sector3_option` WRITE;
/*!40000 ALTER TABLE `sector3_option` DISABLE KEYS */;
INSERT INTO `sector3_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x00000000 0x0007e000');
/*!40000 ALTER TABLE `sector3_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_pos`
--

DROP TABLE IF EXISTS `sector3_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector3_pos`
--

LOCK TABLES `sector3_pos` WRITE;
/*!40000 ALTER TABLE `sector3_pos` DISABLE KEYS */;
INSERT INTO `sector3_pos` VALUES ('adcecal3',2,1),('tdcecal3',3,1),('adcpcal3',5,1),('tdcpcal3',6,1),('adcftof3',8,1),('tdcftof3',9,1),('EB13',2,3);
/*!40000 ALTER TABLE `sector3_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector3_script`
--

DROP TABLE IF EXISTS `sector3_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector3_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector3_script`
--

LOCK TABLES `sector3_script` WRITE;
/*!40000 ALTER TABLE `sector3_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector3_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4`
--

DROP TABLE IF EXISTS `sector4`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4` (
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
-- Dumping data for table `sector4`
--

LOCK TABLES `sector4` WRITE;
/*!40000 ALTER TABLE `sector4` DISABLE KEYS */;
INSERT INTO `sector4` VALUES ('adcecal4','{fadc1_master.so usr} {fadc2.so usr} ','','EB14:adcecal4','yes','','19'),('tdcecal4','{tdc1_slave.so usr} {tdc2.so usr} ','','EB14:adcecal4','yes','adcpcal4','20'),('adcpcal4','{fadc1_slave.so usr} {fadc2.so usr} ','','EB14:adcecal4','no','tdcpcal4','21'),('tdcpcal4','{tdc1_slave.so usr} {tdc2.so usr} ','','EB14:adcecal4','no','adcftof4','22'),('adcftof4','{fadc1_slave.so usr} {fadc2.so usr} ','','EB14:adcecal4','no','tdcftof4','23'),('tdcftof4','{tdc1_slave.so usr} {tdc2.so usr} ','','EB14:adcecal4','no','','24'),('EB14','{CODA} {CODA} ','adcecal4:adcecal4 tdcecal4:tdcecal4 adcpcal4:adcpcal4 tdcpcal4:tdcpcal4 adcftof4:adcftof4 tdcftof4:tdcftof4','','yes','','no');
/*!40000 ALTER TABLE `sector4` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_er`
--

DROP TABLE IF EXISTS `sector4_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_er` (
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
-- Dumping data for table `sector4_er`
--

LOCK TABLES `sector4_er` WRITE;
/*!40000 ALTER TABLE `sector4_er` DISABLE KEYS */;
INSERT INTO `sector4_er` VALUES ('adcecal4','{fadc1_master.so usr} {fadc2.so usr} ','','EB14:adcecal4','yes','tdcecal4','19'),('tdcecal4','{tdc1_slave.so usr} {tdc2.so usr} ','','EB14:adcecal4','no','adcpcal4','20'),('adcpcal4','{fadc1_slave.so usr} {fadc2.so usr} ','','EB14:adcecal4','no','tdcpcal4','21'),('tdcpcal4','{tdc1_slave.so usr} {tdc2.so usr} ','','EB14:adcecal4','no','adcftof4','22'),('adcftof4','{fadc1_slave.so usr} {fadc2.so usr} ','','EB14:adcecal4','no','tdcftof4','23'),('tdcftof4','{tdc1_slave.so usr} {tdc2.so usr} ','','EB14:adcecal4','no','','24'),('EB14','{CODA} {CODA} ','adcecal4:adcecal4 tdcecal4:tdcecal4 adcpcal4:adcpcal4 tdcpcal4:tdcpcal4 adcftof4:adcftof4 tdcftof4:tdcftof4','','yes','','no'),('ER14','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER14:adcecal4','','yes','','no');
/*!40000 ALTER TABLE `sector4_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_er_option`
--

DROP TABLE IF EXISTS `sector4_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector4_er_option`
--

LOCK TABLES `sector4_er_option` WRITE;
/*!40000 ALTER TABLE `sector4_er_option` DISABLE KEYS */;
INSERT INTO `sector4_er_option` VALUES ('dataLimit','0'),('eventLimit','10000'),('tokenInterval','64'),('dataFile','/work/stage_in/sector4'),('rocMask','0x00000000 0x00000000 0x00000000 0x01f80000'),('confFile','none');
/*!40000 ALTER TABLE `sector4_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_er_pos`
--

DROP TABLE IF EXISTS `sector4_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector4_er_pos`
--

LOCK TABLES `sector4_er_pos` WRITE;
/*!40000 ALTER TABLE `sector4_er_pos` DISABLE KEYS */;
INSERT INTO `sector4_er_pos` VALUES ('adcecal4',2,1),('tdcecal4',3,1),('adcpcal4',5,1),('tdcpcal4',6,1),('adcftof4',8,1),('tdcftof4',9,1),('EB14',2,3),('ER14',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `sector4_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_er_script`
--

DROP TABLE IF EXISTS `sector4_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector4_er_script`
--

LOCK TABLES `sector4_er_script` WRITE;
/*!40000 ALTER TABLE `sector4_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector4_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_option`
--

DROP TABLE IF EXISTS `sector4_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector4_option`
--

LOCK TABLES `sector4_option` WRITE;
/*!40000 ALTER TABLE `sector4_option` DISABLE KEYS */;
INSERT INTO `sector4_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00000000 0x01f80000'),('confFile','none');
/*!40000 ALTER TABLE `sector4_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_pos`
--

DROP TABLE IF EXISTS `sector4_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector4_pos`
--

LOCK TABLES `sector4_pos` WRITE;
/*!40000 ALTER TABLE `sector4_pos` DISABLE KEYS */;
INSERT INTO `sector4_pos` VALUES ('adcecal4',2,1),('tdcecal4',3,1),('adcpcal4',5,1),('tdcpcal4',6,1),('adcftof4',8,1),('tdcftof4',9,1),('EB14',2,3);
/*!40000 ALTER TABLE `sector4_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector4_script`
--

DROP TABLE IF EXISTS `sector4_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector4_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector4_script`
--

LOCK TABLES `sector4_script` WRITE;
/*!40000 ALTER TABLE `sector4_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector4_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5`
--

DROP TABLE IF EXISTS `sector5`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5` (
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
-- Dumping data for table `sector5`
--

LOCK TABLES `sector5` WRITE;
/*!40000 ALTER TABLE `sector5` DISABLE KEYS */;
INSERT INTO `sector5` VALUES ('adcecal5','{hps1_master.so usr} {hps2.so usr} ','','EB15:adcecal5','yes','','25'),('tdcecal5','{hps1_slave.so usr} {hps2.so usr} ','','EB15:adcecal5','yes','adcpcal5','26'),('adcpcal5','{hps1_slave.so usr} {hps2.so usr} ','','EB15:adcecal5','no','tdcpcal5','27'),('tdcpcal5','{hps1_slave.so usr} {hps2.so usr} ','','EB15:adcecal5','no','adcftof5','28'),('adcftof5','{hps1_slave.so usr} {hps2.so usr} ','','EB15:adcecal5','no','tdcftof5','29'),('tdcftof5','{hps1_slave.so usr} {hps2.so usr} ','','EB15:adcecal5','no','','30'),('EB15','{CODA} {CODA} ','adcecal5:adcecal5 tdcpcal5:tdcpcal5 tdcecal5:tdcecal5 tdcftof5:tdcftof5 adcftof5:adcftof5 adcpcal5:adcpcal5','ET15:adcecal5','yes','','no'),('ET15','{500} {200000} ','EB15:adcecal5','','yes','','no');
/*!40000 ALTER TABLE `sector5` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_er`
--

DROP TABLE IF EXISTS `sector5_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_er` (
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
-- Dumping data for table `sector5_er`
--

LOCK TABLES `sector5_er` WRITE;
/*!40000 ALTER TABLE `sector5_er` DISABLE KEYS */;
INSERT INTO `sector5_er` VALUES ('adcecal5','{fadc1_master.so usr} {fadc2.so usr} ','','EB15:adcecal5','yes','','25'),('tdcecal5','{tdc1_slave.so usr} {tdc2.so usr} ','','EB15:adcecal5','yes','adcpcal5','26'),('adcpcal5','{fadc1_slave.so usr} {fadc2.so usr} ','','EB15:adcecal5','no','tdcpcal5','27'),('tdcpcal5','{tdc1_slave.so usr} {tdc2.so usr} ','','EB15:adcecal5','no','adcftof5','28'),('adcftof5','{fadc1_slave.so usr} {fadc2.so usr} ','','EB15:adcecal5','no','tdcftof5','29'),('tdcftof5','{tdc1_slave.so usr} {tdc2.so usr} ','','EB15:adcecal5','no','','30'),('EB15','{CODA} {CODA} ','adcecal5:adcecal5 tdcpcal5:tdcpcal5 tdcecal5:tdcecal5 tdcftof5:tdcftof5 adcftof5:adcftof5 adcpcal5:adcpcal5','','yes','','no'),('ER15','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER15:adcecal5','','yes','','no');
/*!40000 ALTER TABLE `sector5_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_er_option`
--

DROP TABLE IF EXISTS `sector5_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector5_er_option`
--

LOCK TABLES `sector5_er_option` WRITE;
/*!40000 ALTER TABLE `sector5_er_option` DISABLE KEYS */;
INSERT INTO `sector5_er_option` VALUES ('dataLimit','0'),('eventLimit','10000'),('tokenInterval','64'),('dataFile','/work/stage_in/sector5'),('rocMask','0x00000000 0x00000000 0x00000000 0x7e000000'),('confFile','none');
/*!40000 ALTER TABLE `sector5_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_er_pos`
--

DROP TABLE IF EXISTS `sector5_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector5_er_pos`
--

LOCK TABLES `sector5_er_pos` WRITE;
/*!40000 ALTER TABLE `sector5_er_pos` DISABLE KEYS */;
INSERT INTO `sector5_er_pos` VALUES ('adcecal5',1,1),('tdcecal5',2,1),('adcpcal5',3,1),('tdcpcal5',4,1),('adcftof5',5,1),('tdcftof5',6,1),('EB15',1,3),('ER15',3,3),('coda_0',3,5);
/*!40000 ALTER TABLE `sector5_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_er_script`
--

DROP TABLE IF EXISTS `sector5_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector5_er_script`
--

LOCK TABLES `sector5_er_script` WRITE;
/*!40000 ALTER TABLE `sector5_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector5_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_option`
--

DROP TABLE IF EXISTS `sector5_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector5_option`
--

LOCK TABLES `sector5_option` WRITE;
/*!40000 ALTER TABLE `sector5_option` DISABLE KEYS */;
INSERT INTO `sector5_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','/usr/clas12/release/0.3/parms/trigger/FC/s5/sector5_tet00_mode1.cnf'),('rocMask','0x00000000 0x00000000 0x00000000 0x7e000000');
/*!40000 ALTER TABLE `sector5_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_pos`
--

DROP TABLE IF EXISTS `sector5_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector5_pos`
--

LOCK TABLES `sector5_pos` WRITE;
/*!40000 ALTER TABLE `sector5_pos` DISABLE KEYS */;
INSERT INTO `sector5_pos` VALUES ('adcecal5',1,1),('tdcecal5',2,1),('adcpcal5',3,1),('tdcpcal5',4,1),('adcftof5',5,1),('tdcftof5',6,1),('EB15',1,3),('ET15',1,5);
/*!40000 ALTER TABLE `sector5_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector5_script`
--

DROP TABLE IF EXISTS `sector5_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector5_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector5_script`
--

LOCK TABLES `sector5_script` WRITE;
/*!40000 ALTER TABLE `sector5_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector5_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6`
--

DROP TABLE IF EXISTS `sector6`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6` (
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
-- Dumping data for table `sector6`
--

LOCK TABLES `sector6` WRITE;
/*!40000 ALTER TABLE `sector6` DISABLE KEYS */;
INSERT INTO `sector6` VALUES ('adcecal6','{fadc1_master.so usr} {fadc2.so usr} ','','EB16:adcecal6','yes','','31'),('adcpcal6','{fadc1_slave.so usr} {fadc2.so usr} ','','EB16:adcecal6','yes','tdcpcal6','33'),('tdcpcal6','{tdc1_slave.so usr} {tdc2.so usr} ','','EB16:adcecal6','no','adcftof6','34'),('adcftof6','{fadc1_slave.so usr} {fadc2.so usr} ','','EB16:adcecal6','no','tdcftof6','35'),('tdcftof6','{tdc1_slave.so usr} {tdc2.so usr} ','','EB16:adcecal6','no','','36'),('EB16','{CODA} {CODA} ','adcecal6:adcecal6 adcftof6:adcftof6 tdcftof6:tdcftof6 tdcpcal6:tdcpcal6 adcpcal6:adcpcal6','','yes','','no');
/*!40000 ALTER TABLE `sector6` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_er`
--

DROP TABLE IF EXISTS `sector6_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_er` (
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
-- Dumping data for table `sector6_er`
--

LOCK TABLES `sector6_er` WRITE;
/*!40000 ALTER TABLE `sector6_er` DISABLE KEYS */;
INSERT INTO `sector6_er` VALUES ('adcecal6','{fadc1_master.so usr} {fadc2.so usr} ','','EB16:adcecal6','yes','','31'),('adcpcal6','{fadc1_slave.so usr} {fadc2.so usr} ','','EB16:adcecal6','yes','tdcpcal6','33'),('tdcpcal6','{tdc1_slave.so usr} {tdc2.so usr} ','','EB16:adcecal6','no','adcftof6','34'),('adcftof6','{fadc1_slave.so usr} {fadc2.so usr} ','','EB16:adcecal6','no','tdcftof6','35'),('tdcftof6','{tdc1_slave.so usr} {tdc2.so usr} ','','EB16:adcecal6','no','','36'),('EB16','{CODA} {CODA} ','adcecal6:adcecal6 adcftof6:adcftof6 tdcftof6:tdcftof6 tdcpcal6:tdcpcal6 adcpcal6:adcpcal6','','yes','','no'),('ER16','{CODA}  ','','coda_2','yes','','no'),('coda_2','','ER16:adcecal6','','yes','','no');
/*!40000 ALTER TABLE `sector6_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_er_option`
--

DROP TABLE IF EXISTS `sector6_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector6_er_option`
--

LOCK TABLES `sector6_er_option` WRITE;
/*!40000 ALTER TABLE `sector6_er_option` DISABLE KEYS */;
INSERT INTO `sector6_er_option` VALUES ('dataLimit','0'),('eventLimit','10000'),('tokenInterval','64'),('dataFile','/work/stage_in/sector6'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x0000001e 0x80000000');
/*!40000 ALTER TABLE `sector6_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_er_pos`
--

DROP TABLE IF EXISTS `sector6_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector6_er_pos`
--

LOCK TABLES `sector6_er_pos` WRITE;
/*!40000 ALTER TABLE `sector6_er_pos` DISABLE KEYS */;
INSERT INTO `sector6_er_pos` VALUES ('adcecal6',2,1),('adcpcal6',5,1),('tdcpcal6',6,1),('adcftof6',8,1),('tdcftof6',9,1),('EB16',2,3),('ER16',4,3),('coda_2',4,5);
/*!40000 ALTER TABLE `sector6_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_er_script`
--

DROP TABLE IF EXISTS `sector6_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector6_er_script`
--

LOCK TABLES `sector6_er_script` WRITE;
/*!40000 ALTER TABLE `sector6_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector6_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_option`
--

DROP TABLE IF EXISTS `sector6_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector6_option`
--

LOCK TABLES `sector6_option` WRITE;
/*!40000 ALTER TABLE `sector6_option` DISABLE KEYS */;
INSERT INTO `sector6_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('confFile','none'),('rocMask','0x00000000 0x00000000 0x0000001e 0x80000000');
/*!40000 ALTER TABLE `sector6_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_pos`
--

DROP TABLE IF EXISTS `sector6_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector6_pos`
--

LOCK TABLES `sector6_pos` WRITE;
/*!40000 ALTER TABLE `sector6_pos` DISABLE KEYS */;
INSERT INTO `sector6_pos` VALUES ('adcecal6',2,1),('adcpcal6',5,1),('tdcpcal6',6,1),('adcftof6',8,1),('tdcftof6',9,1),('EB16',2,3);
/*!40000 ALTER TABLE `sector6_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sector6_script`
--

DROP TABLE IF EXISTS `sector6_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sector6_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sector6_script`
--

LOCK TABLES `sector6_script` WRITE;
/*!40000 ALTER TABLE `sector6_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sector6_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sergey`
--

DROP TABLE IF EXISTS `sergey`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sergey` (
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
-- Dumping data for table `sergey`
--

LOCK TABLES `sergey` WRITE;
/*!40000 ALTER TABLE `sergey` DISABLE KEYS */;
INSERT INTO `sergey` VALUES ('highbtest3','{aa} {ss} ','','','yes','','no');
/*!40000 ALTER TABLE `sergey` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sergey_option`
--

DROP TABLE IF EXISTS `sergey_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sergey_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sergey_option`
--

LOCK TABLES `sergey_option` WRITE;
/*!40000 ALTER TABLE `sergey_option` DISABLE KEYS */;
INSERT INTO `sergey_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64');
/*!40000 ALTER TABLE `sergey_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sergey_pos`
--

DROP TABLE IF EXISTS `sergey_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sergey_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sergey_pos`
--

LOCK TABLES `sergey_pos` WRITE;
/*!40000 ALTER TABLE `sergey_pos` DISABLE KEYS */;
INSERT INTO `sergey_pos` VALUES ('highbtest3',2,1);
/*!40000 ALTER TABLE `sergey_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sergey_script`
--

DROP TABLE IF EXISTS `sergey_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `sergey_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sergey_script`
--

LOCK TABLES `sergey_script` WRITE;
/*!40000 ALTER TABLE `sergey_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `sergey_script` ENABLE KEYS */;
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
INSERT INTO `sessions` VALUES ('ctoftest',0,'ctof1 clasrun 2508 9998','yes','','RunControl',4235,'ctof1'),('clastest',0,'clonpc3.jlab.org boiarino 1538 146','yes','','RunControl',11054,'ltcc0'),('clasprod',0,'adcecal1 clasrun 2508 9998','yes','','RunControl',573,'sector1'),('clastest0',0,'clonpc0.jlab.org boiarino 1538 146','yes','','RunControl',115,'test0'),('claswolf',0,'clon00 vpk 3803 146','yes','claswolf_msg','RunControl',47266,'wolf_er'),('fadc1720',0,'clon00 battagli 2004 146','yes','','RunControl',156,'fadc1'),('clashps',0,'hps1 clasrun 2508 9998','yes','','RunControl',1618,'hpsgtp'),('fadc1720a',0,'clon00 battagli 2004 146','yes','fadc1720a_msg','RunControl',0,'fadc1'),('classvt',0,'svt5 boiarino 1538 146','yes','classvt_msg','RunControl',276,''),('clasftof',0,'ftof0 clasrun 2508 9998','yes','clasftof_msg','RunControl',602,'ftof'),('clasprod1',0,'adcecal1 clasrun 2508 9998','yes','clasprod1_msg','RunControl',79,'sector1'),('claspcal',0,'pcal0 boiarino 1538 146','no','claspcal_msg','RunControl',0,''),('clasltcc',0,'ltcc0 clasrun 2508 9998','yes','clasltcc_msg','RunControl',173,'ltcc0'),('clasdcrb',0,'dcrb1 braydo 6507 146','yes','clasdcrb_msg','RunControl',118,'dcrb1'),('clasprod4',0,'adcecal4 clasrun 2508 9998','yes','clasprod4_msg','RunControl',81,'sector4'),('clasprod5',0,'adcecal5 clasrun 2508 9998','yes','clasprod5_msg','RunControl',670,'sector5'),('clasprod6',0,'adcecal6 clasrun 2508 9998','yes','clasprod6_msg','RunControl',20,'sector6'),('clasprod2',0,'adcecal2 clasrun 2508 9998','yes','clasprod2_msg','RunControl',37,'sector2'),('clasprod3',0,'adcecal3 clasrun 2508 9998','yes','clasprod3_msg','RunControl',51,'sector3'),('fttest',0,'jlab12daq1.jlab.org clasrun 2508 9998','yes','fttest_msg','RunControl',0,'FT');
/*!40000 ALTER TABLE `sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `standalone_option`
--

DROP TABLE IF EXISTS `standalone_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `standalone_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `standalone_option`
--

LOCK TABLES `standalone_option` WRITE;
/*!40000 ALTER TABLE `standalone_option` DISABLE KEYS */;
INSERT INTO `standalone_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','1');
/*!40000 ALTER TABLE `standalone_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `standalone_parallel_option`
--

DROP TABLE IF EXISTS `standalone_parallel_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `standalone_parallel_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `standalone_parallel_option`
--

LOCK TABLES `standalone_parallel_option` WRITE;
/*!40000 ALTER TABLE `standalone_parallel_option` DISABLE KEYS */;
INSERT INTO `standalone_parallel_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','1');
/*!40000 ALTER TABLE `standalone_parallel_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `svt_option`
--

DROP TABLE IF EXISTS `svt_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `svt_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `svt_option`
--

LOCK TABLES `svt_option` WRITE;
/*!40000 ALTER TABLE `svt_option` DISABLE KEYS */;
INSERT INTO `svt_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','16');
/*!40000 ALTER TABLE `svt_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tdcecal1_option`
--

DROP TABLE IF EXISTS `tdcecal1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tdcecal1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tdcecal1_option`
--

LOCK TABLES `tdcecal1_option` WRITE;
/*!40000 ALTER TABLE `tdcecal1_option` DISABLE KEYS */;
INSERT INTO `tdcecal1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/adcecal1/tdcecal1'),('rocMask','524288');
/*!40000 ALTER TABLE `tdcecal1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test0`
--

DROP TABLE IF EXISTS `test0`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test0` (
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
-- Dumping data for table `test0`
--

LOCK TABLES `test0` WRITE;
/*!40000 ALTER TABLE `test0` DISABLE KEYS */;
INSERT INTO `test0` VALUES ('zedboard1','{rol1.so usr} {rol2.so usr} ','','EB0:clonpc0','yes','','45'),('EB0','{CODA} {CODA} ','zedboard1:zedboard1','','yes','','no'),('ER0','{CODA}  ','','coda_0','yes','','no'),('coda_0','','ER0:clonpc0','','yes','','no');
/*!40000 ALTER TABLE `test0` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test0_option`
--

DROP TABLE IF EXISTS `test0_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test0_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test0_option`
--

LOCK TABLES `test0_option` WRITE;
/*!40000 ALTER TABLE `test0_option` DISABLE KEYS */;
INSERT INTO `test0_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','0x00000000 0x00000000 0x00002000 0x00000000'),('dataFile','/work/stage_in/zedboard1'),('confFile','none');
/*!40000 ALTER TABLE `test0_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test0_pos`
--

DROP TABLE IF EXISTS `test0_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test0_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test0_pos`
--

LOCK TABLES `test0_pos` WRITE;
/*!40000 ALTER TABLE `test0_pos` DISABLE KEYS */;
INSERT INTO `test0_pos` VALUES ('zedboard1',2,1),('EB0',2,3),('ER0',4,3),('coda_0',4,5);
/*!40000 ALTER TABLE `test0_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test0_script`
--

DROP TABLE IF EXISTS `test0_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test0_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test0_script`
--

LOCK TABLES `test0_script` WRITE;
/*!40000 ALTER TABLE `test0_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `test0_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test0vx_option`
--

DROP TABLE IF EXISTS `test0vx_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test0vx_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test0vx_option`
--

LOCK TABLES `test0vx_option` WRITE;
/*!40000 ALTER TABLE `test0vx_option` DISABLE KEYS */;
INSERT INTO `test0vx_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','8388608'),('dataFile','test.dat');
/*!40000 ALTER TABLE `test0vx_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts1_option`
--

DROP TABLE IF EXISTS `test_ts1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts1_option`
--

LOCK TABLES `test_ts1_option` WRITE;
/*!40000 ALTER TABLE `test_ts1_option` DISABLE KEYS */;
INSERT INTO `test_ts1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','-2013265920'),('dataFile','test.dat');
/*!40000 ALTER TABLE `test_ts1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts2_er_option`
--

DROP TABLE IF EXISTS `test_ts2_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts2_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts2_er_option`
--

LOCK TABLES `test_ts2_er_option` WRITE;
/*!40000 ALTER TABLE `test_ts2_er_option` DISABLE KEYS */;
INSERT INTO `test_ts2_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/stage_in/test_%06d.A00'),('SPLITMB','2047'),('rocMask','134217729');
/*!40000 ALTER TABLE `test_ts2_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts2_option`
--

DROP TABLE IF EXISTS `test_ts2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts2_option`
--

LOCK TABLES `test_ts2_option` WRITE;
/*!40000 ALTER TABLE `test_ts2_option` DISABLE KEYS */;
INSERT INTO `test_ts2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','134217729'),('dataFile','BOS'),('SPLITMB','2047');
/*!40000 ALTER TABLE `test_ts2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts3_er_option`
--

DROP TABLE IF EXISTS `test_ts3_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts3_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts3_er_option`
--

LOCK TABLES `test_ts3_er_option` WRITE;
/*!40000 ALTER TABLE `test_ts3_er_option` DISABLE KEYS */;
INSERT INTO `test_ts3_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/stage_in/fadc250_%06d.A00'),('SPLITMB','2047'),('rocMask','671088640');
/*!40000 ALTER TABLE `test_ts3_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts3_fanuc_option`
--

DROP TABLE IF EXISTS `test_ts3_fanuc_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts3_fanuc_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts3_fanuc_option`
--

LOCK TABLES `test_ts3_fanuc_option` WRITE;
/*!40000 ALTER TABLE `test_ts3_fanuc_option` DISABLE KEYS */;
INSERT INTO `test_ts3_fanuc_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','536870912');
/*!40000 ALTER TABLE `test_ts3_fanuc_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts3_option`
--

DROP TABLE IF EXISTS `test_ts3_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts3_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts3_option`
--

LOCK TABLES `test_ts3_option` WRITE;
/*!40000 ALTER TABLE `test_ts3_option` DISABLE KEYS */;
INSERT INTO `test_ts3_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','671088640');
/*!40000 ALTER TABLE `test_ts3_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts3_tid_er_option`
--

DROP TABLE IF EXISTS `test_ts3_tid_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts3_tid_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts3_tid_er_option`
--

LOCK TABLES `test_ts3_tid_er_option` WRITE;
/*!40000 ALTER TABLE `test_ts3_tid_er_option` DISABLE KEYS */;
INSERT INTO `test_ts3_tid_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','/work/stage_in/tid_%06d.A00'),('SPLITMB','2047'),('rocMask','536870912');
/*!40000 ALTER TABLE `test_ts3_tid_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts3_tid_fast_option`
--

DROP TABLE IF EXISTS `test_ts3_tid_fast_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts3_tid_fast_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts3_tid_fast_option`
--

LOCK TABLES `test_ts3_tid_fast_option` WRITE;
/*!40000 ALTER TABLE `test_ts3_tid_fast_option` DISABLE KEYS */;
INSERT INTO `test_ts3_tid_fast_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','536870912');
/*!40000 ALTER TABLE `test_ts3_tid_fast_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts3_tid_option`
--

DROP TABLE IF EXISTS `test_ts3_tid_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts3_tid_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts3_tid_option`
--

LOCK TABLES `test_ts3_tid_option` WRITE;
/*!40000 ALTER TABLE `test_ts3_tid_option` DISABLE KEYS */;
INSERT INTO `test_ts3_tid_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','536870912');
/*!40000 ALTER TABLE `test_ts3_tid_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts5_er_option`
--

DROP TABLE IF EXISTS `test_ts5_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts5_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts5_er_option`
--

LOCK TABLES `test_ts5_er_option` WRITE;
/*!40000 ALTER TABLE `test_ts5_er_option` DISABLE KEYS */;
INSERT INTO `test_ts5_er_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('dataFile','test.dat'),('SPLITMB','2047'),('rocMask','-2013265918');
/*!40000 ALTER TABLE `test_ts5_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_ts5_option`
--

DROP TABLE IF EXISTS `test_ts5_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_ts5_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_ts5_option`
--

LOCK TABLES `test_ts5_option` WRITE;
/*!40000 ALTER TABLE `test_ts5_option` DISABLE KEYS */;
INSERT INTO `test_ts5_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','1207959552');
/*!40000 ALTER TABLE `test_ts5_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_unixroc_option`
--

DROP TABLE IF EXISTS `test_unixroc_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test_unixroc_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test_unixroc_option`
--

LOCK TABLES `test_unixroc_option` WRITE;
/*!40000 ALTER TABLE `test_unixroc_option` DISABLE KEYS */;
INSERT INTO `test_unixroc_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','16777216'),('dataFile','/usr/local/clas/devel_new/source/coda/src/rol/main/Linux_i686/obj/bonusrol1.so u');
/*!40000 ALTER TABLE `test_unixroc_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `testxt1_ts1_option`
--

DROP TABLE IF EXISTS `testxt1_ts1_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `testxt1_ts1_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `testxt1_ts1_option`
--

LOCK TABLES `testxt1_ts1_option` WRITE;
/*!40000 ALTER TABLE `testxt1_ts1_option` DISABLE KEYS */;
INSERT INTO `testxt1_ts1_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','-2013265920');
/*!40000 ALTER TABLE `testxt1_ts1_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ts2_option`
--

DROP TABLE IF EXISTS `ts2_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ts2_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ts2_option`
--

LOCK TABLES `ts2_option` WRITE;
/*!40000 ALTER TABLE `ts2_option` DISABLE KEYS */;
INSERT INTO `ts2_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','134217728');
/*!40000 ALTER TABLE `ts2_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf`
--

DROP TABLE IF EXISTS `wolf`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf` (
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
-- Dumping data for table `wolf`
--

LOCK TABLES `wolf` WRITE;
/*!40000 ALTER TABLE `wolf` DISABLE KEYS */;
INSERT INTO `wolf` VALUES ('wolfram1','{$CODA/src/rol/rol/VXWORKS_ppc/obj/wolfrol1.o usr} {$CODA/VXWORKS_ppc/rol/rol2_tt.o usr} ','','EB2:clon00','yes','','25'),('EB2','{BOS} {BOS} ','wolfram1:wolfram1','','yes','','no');
/*!40000 ALTER TABLE `wolf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_er`
--

DROP TABLE IF EXISTS `wolf_er`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_er` (
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
-- Dumping data for table `wolf_er`
--

LOCK TABLES `wolf_er` WRITE;
/*!40000 ALTER TABLE `wolf_er` DISABLE KEYS */;
INSERT INTO `wolf_er` VALUES ('wolfram1','{$CODA/src/rol/rol/VXWORKS_ppc/obj/wolfrol1.o usr} {$CODA/VXWORKS_ppc/rol/rol2_tt.o usr} ','','EB2:clon00','yes','','25'),('EB2','{BOS} {BOS} ','wolfram1:wolfram1','','yes','','no'),('ER2','{FPACK}  ','','file_0','yes','','no'),('file_0','','ER2:clon00','','yes','','no');
/*!40000 ALTER TABLE `wolf_er` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_er_option`
--

DROP TABLE IF EXISTS `wolf_er_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_er_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `wolf_er_option`
--

LOCK TABLES `wolf_er_option` WRITE;
/*!40000 ALTER TABLE `wolf_er_option` DISABLE KEYS */;
INSERT INTO `wolf_er_option` VALUES ('dataLimit','0'),('eventLimit','100000'),('tokenInterval','64'),('dataFile','/work/wolfram/wolfram_%06d.A00'),('SPLITMB','2047'),('rocMask','33554432'),('confFile','none');
/*!40000 ALTER TABLE `wolf_er_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_er_pos`
--

DROP TABLE IF EXISTS `wolf_er_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_er_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `wolf_er_pos`
--

LOCK TABLES `wolf_er_pos` WRITE;
/*!40000 ALTER TABLE `wolf_er_pos` DISABLE KEYS */;
INSERT INTO `wolf_er_pos` VALUES ('wolfram1',3,1),('EB2',3,3),('ER2',5,3),('file_0',5,5);
/*!40000 ALTER TABLE `wolf_er_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_er_script`
--

DROP TABLE IF EXISTS `wolf_er_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_er_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `wolf_er_script`
--

LOCK TABLES `wolf_er_script` WRITE;
/*!40000 ALTER TABLE `wolf_er_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `wolf_er_script` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_option`
--

DROP TABLE IF EXISTS `wolf_option`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_option` (
  `name` char(32) NOT NULL default '',
  `value` char(80) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `wolf_option`
--

LOCK TABLES `wolf_option` WRITE;
/*!40000 ALTER TABLE `wolf_option` DISABLE KEYS */;
INSERT INTO `wolf_option` VALUES ('dataLimit','0'),('eventLimit','0'),('tokenInterval','64'),('rocMask','33554432');
/*!40000 ALTER TABLE `wolf_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_pos`
--

DROP TABLE IF EXISTS `wolf_pos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_pos` (
  `name` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
  `row` int(11) NOT NULL default '0',
  `col` int(11) NOT NULL default '0',
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `wolf_pos`
--

LOCK TABLES `wolf_pos` WRITE;
/*!40000 ALTER TABLE `wolf_pos` DISABLE KEYS */;
INSERT INTO `wolf_pos` VALUES ('wolfram1',3,1),('EB2',3,3);
/*!40000 ALTER TABLE `wolf_pos` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `wolf_script`
--

DROP TABLE IF EXISTS `wolf_script`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `wolf_script` (
  `name` char(32) NOT NULL default '',
  `state` char(32) NOT NULL default '',
  `script` char(128) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `wolf_script`
--

LOCK TABLES `wolf_script` WRITE;
/*!40000 ALTER TABLE `wolf_script` DISABLE KEYS */;
/*!40000 ALTER TABLE `wolf_script` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-01-26 16:08:33
