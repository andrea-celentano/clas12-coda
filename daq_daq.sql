-- MySQL dump 10.11
--
-- Host: clondb1    Database: daq_daq
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
-- Current Database: `daq_daq`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `daq_daq` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `daq_daq`;

--
-- Table structure for table `PMCs`
--

DROP TABLE IF EXISTS `PMCs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `PMCs` (
  `name` varchar(80) NOT NULL default '',
  `Host` varchar(80) default NULL,
  `StructAddr` int(11) default NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `PMCs`
--

LOCK TABLES `PMCs` WRITE;
/*!40000 ALTER TABLE `PMCs` DISABLE KEYS */;
INSERT INTO `PMCs` VALUES ('dc4pmc1','dc4pmc1',221922980),('dc1pmc1','dc1pmc1',221923076),('dc2pmc1','dc2pmc1',490063236),('dc3pmc1','dc3pmc1',221923076),('dc5pmc1','dc5pmc1',221923076),('dc6pmc1','dc6pmc1',221923060),('dc7pmc1','dc7pmc1',490063236),('dc8pmc1','dc8pmc1',490063236),('dc10pmc1','dc10pmc1',490063236),('dc11pmc1','dc11pmc1',-1),('ec1pmc1','ec1pmc1',-1),('sc1pmc1','sc1pmc1',-1),('ec2pmc1','ec2pmc1',-1),('dc9pmc1','dc9pmc1',221923076),('cc1pmc1','cc1pmc1',-1),('dvcs2pmc1','dvcs2pmc1',-1),('ec3pmc1','ec3pmc1',-1),('sc2pmc1','sc2pmc1',221921988),('ec4pmc1','ec4pmc1',-1),('lac1pmc1','lac1pmc1',-1),('croctest11','croctest11',-1),('croctest1pmc1','croctest1pmc1',-1),('croctest2pmc1','croctest2pmc1',-1),('tage2pmc1','tage2pmc1',-1),('tagepmc1','tagepmc1',221934636),('tage3pmc1','tage3pmc1',221928756),('primextdc1pmc1','primextdc1pmc1',221962380),('primexroc4pmc1','primexroc4pmc1',221960748),('primexroc6pmc1','primexroc6pmc1',221960748),('primexroc5pmc1','primexroc5pmc1',221960748),('lac2pmc1','lac2pmc1',-1),('','',-1),('polarpmc1','polarpmc1',-1),('trig1pmc1','trig1pmc1',-1);
/*!40000 ALTER TABLE `PMCs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `Ports`
--

DROP TABLE IF EXISTS `Ports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Ports` (
  `name` varchar(80) NOT NULL default '',
  `Host` varchar(80) default NULL,
  `Session` varchar(80) default NULL,
  `Daq_udp` int(11) default NULL,
  `Daq_tcp` int(11) default NULL,
  `tcpClient_tcp` int(11) default NULL,
  `Trigger_tcp` int(11) default NULL,
  PRIMARY KEY  (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Ports for various services';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Ports`
--

LOCK TABLES `Ports` WRITE;
/*!40000 ALTER TABLE `Ports` DISABLE KEYS */;
INSERT INTO `Ports` VALUES ('ctoftest1','ctoftest1',NULL,NULL,NULL,5001,NULL),('wolfram1','wolfram1','',-999,-999,5001,-999),('EB1','adcecal1',NULL,NULL,NULL,5004,NULL),('ER1','adcecal1',NULL,NULL,NULL,5001,NULL),('ER6','clon00',NULL,NULL,NULL,5001,NULL),('EB6','clon00',NULL,NULL,NULL,5003,NULL),('EB5','svt5',NULL,NULL,NULL,5003,NULL),('ER5','svt5',NULL,NULL,NULL,5001,NULL),('EB4','ftof0',NULL,NULL,NULL,5003,NULL),('ER4','ftof0',NULL,NULL,NULL,5001,NULL),('ER2','svt2',NULL,NULL,NULL,5001,NULL),('EB2','svt2',NULL,NULL,NULL,5003,NULL),('ER3','ltcc0',NULL,NULL,NULL,5003,NULL),('EB40','clondaq1.jlab.org',NULL,NULL,NULL,5003,NULL),('EB7','clon00',NULL,NULL,NULL,5001,NULL),('ER7','clon00',NULL,NULL,NULL,5003,NULL),('ER8','dcrb1',NULL,NULL,NULL,5001,NULL),('EB8','dcrb1',NULL,NULL,NULL,5005,NULL),('EB9','hps1',NULL,NULL,NULL,5003,NULL),('ER9','hps1',NULL,NULL,NULL,5001,NULL),('hps2','hps2',NULL,NULL,NULL,5001,6002),('hps1','hps1',NULL,NULL,NULL,5006,6002),('hps2gtp','hps2gtp',NULL,NULL,NULL,5002,NULL),('hpstracker','hps1',NULL,NULL,NULL,5005,NULL),('clonusr3.jlab.org','clonusr3.jlab.org',NULL,NULL,NULL,NULL,6002),('EB10','adcecal1',NULL,NULL,NULL,5001,NULL),('svt1','svt6',NULL,NULL,NULL,5001,NULL),('ER10','pcal0',NULL,NULL,NULL,5003,NULL),('svt2','svt2',NULL,NULL,NULL,5006,6002),('hp1','hps1',NULL,NULL,NULL,5001,NULL),('vmetest0','vmetest0',NULL,NULL,NULL,5001,NULL),('ioctest0','ioctest0',NULL,NULL,NULL,5001,NULL),('ioctest1','ioctest1',NULL,NULL,NULL,5001,NULL),('vxstest0','vxstest0',NULL,NULL,NULL,NULL,6002),('svt6','svt6',NULL,NULL,NULL,5001,NULL),('svt4','svt4',NULL,NULL,NULL,5001,NULL),('dcrb1','dcrb1',NULL,NULL,NULL,5007,6002),('dcrb2','vxstest0',NULL,NULL,NULL,5001,NULL),('ftof1','ftof1',NULL,NULL,NULL,5001,6002),('ftof0','ftof0',NULL,NULL,NULL,5005,6002),('EB3','ltcc0',NULL,NULL,NULL,5001,NULL),('adcecal1','adcecal1',NULL,NULL,NULL,5006,6002),('ltcc0','ltcc0',NULL,NULL,NULL,5005,NULL),('svt5','svt5',NULL,NULL,NULL,5005,6002),('tdcecal1','tdcecal1',NULL,NULL,NULL,5001,6002),('EB0','clonpc0.jlab.org',NULL,NULL,NULL,5003,NULL),('ER0','clonpc0.jlab.org',NULL,NULL,NULL,5001,NULL),('ROC0','zedboard1',NULL,NULL,NULL,5002,NULL),('trig1pmc1','trig1pmc1','',-999,-999,5002,-999),('trig1','trig1',NULL,NULL,NULL,5002,NULL),('ROC00','clonioc1.jlab.org',NULL,NULL,NULL,5005,NULL),('ROC5','svt5',NULL,NULL,NULL,5006,NULL),('svt','svt5',NULL,NULL,NULL,5006,NULL),('tdcpcal1','tdcpcal1',NULL,NULL,NULL,5001,6002),('dcrb1gtp','dcrb1','',-999,-999,5003,6003),('clonpc3.jlab.org','clonpc3.jlab.org',NULL,NULL,NULL,NULL,6002),('zedboard1','zedboard1',NULL,NULL,NULL,5002,NULL),('tdcftof1','tdcftof1',NULL,NULL,NULL,5001,6002),('EB11','adcecal1',NULL,NULL,NULL,5003,NULL),('ER11','adcecal1',NULL,NULL,NULL,5001,NULL),('adcpcal1','adcpcal1',NULL,NULL,NULL,5001,6002),('adcftof1','adcftof1',NULL,NULL,NULL,5001,6002),('tdcpcal6','tdcpcal6',NULL,NULL,NULL,5001,6002),('tdcpcal5','tdcpcal5',NULL,NULL,NULL,5001,6002),('tdcpcal4','tdcpcal4',NULL,NULL,NULL,5001,6002),('tdcpcal2','tdcpcal2',NULL,NULL,NULL,5001,6002),('tdcpcal3','tdcpcal3',NULL,NULL,NULL,5001,6002),('ioctest2','ioctest2',NULL,NULL,NULL,5001,6002),('tdcecal5','tdcecal5',NULL,NULL,NULL,5001,6002),('tdcftof5','tdcftof5',NULL,NULL,NULL,5001,6002),('adcftof5','adcftof5',NULL,NULL,NULL,5001,6002),('ER15','adcecal5',NULL,NULL,NULL,5003,NULL),('EB15','adcecal5',NULL,NULL,NULL,5001,NULL),('adcecal5','adcecal5',NULL,NULL,NULL,5006,NULL),('adcpcal5','adcpcal5',NULL,NULL,NULL,5001,6002),('hps1gtp','hps1gtp',NULL,NULL,NULL,5001,NULL),('adcecal4','adcecal4',NULL,NULL,NULL,5005,6002),('tdcftof4','tdcftof4',NULL,NULL,NULL,5001,6002),('adcftof4','adcftof4',NULL,NULL,NULL,5001,6002),('adcpcal4','adcpcal4',NULL,NULL,NULL,5001,6002),('tdcecal4','tdcecal4',NULL,NULL,NULL,5001,6002),('ER14','adcecal4',NULL,NULL,NULL,5001,NULL),('EB14','adcecal4',NULL,NULL,NULL,5003,NULL),('tdcecal3','tdcecal3',NULL,NULL,NULL,5001,6002),('tdcftof3','tdcftof3',NULL,NULL,NULL,5001,6002),('adcftof3','adcftof3',NULL,NULL,NULL,5001,6002),('adcpcal3','adcpcal3',NULL,NULL,NULL,5001,6002),('tdcftof2','tdcftof2',NULL,NULL,NULL,5001,6002),('adcpcal2','adcpcal2',NULL,NULL,NULL,5001,6002),('ER13','adcecal3',NULL,NULL,NULL,5005,NULL),('EB13','adcecal3',NULL,NULL,NULL,5001,NULL),('adcecal3','adcecal3',NULL,NULL,NULL,5003,NULL),('adcftof2','adcftof2',NULL,NULL,NULL,5001,6002),('adcftof6','adcftof6',NULL,NULL,NULL,5001,6002),('ER12','adcecal2',NULL,NULL,NULL,5004,NULL),('EB12','adcecal2',NULL,NULL,NULL,5001,NULL),('adcecal2','adcecal2',NULL,NULL,NULL,5005,NULL),('tdcecal2','tdcecal2',NULL,NULL,NULL,5001,6002),('tdcftof6','tdcftof6',NULL,NULL,NULL,5001,6002),('adcpcal6','adcpcal6',NULL,NULL,NULL,5001,6002),('EB16','adcecal6',NULL,NULL,NULL,5005,NULL),('ER16','adcecal6',NULL,NULL,NULL,5004,NULL),('adcecal6','adcecal6',NULL,NULL,NULL,5001,NULL);
/*!40000 ALTER TABLE `Ports` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2014-06-06 10:10:13
