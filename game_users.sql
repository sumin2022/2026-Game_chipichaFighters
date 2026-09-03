-- MySQL dump 10.13  Distrib 9.5.0, for Win64 (x86_64)
--
-- Host: localhost    Database: game
-- ------------------------------------------------------
-- Server version	9.5.0

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
SET @MYSQLDUMP_TEMP_LOG_BIN = @@SESSION.SQL_LOG_BIN;
SET @@SESSION.SQL_LOG_BIN= 0;

--
-- GTID state at the beginning of the backup 
--

SET @@GLOBAL.GTID_PURGED=/*!80000 '+'*/ 'bf6c25f6-d7d9-11f0-809a-d843ae0d696b:1-410';

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `id` varchar(50) NOT NULL,
  `password_hash` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES ('asdf',''),('Bot1','BOT_TEST'),('Bot10','BOT_TEST'),('Bot100','BOT_TEST'),('Bot11','BOT_TEST'),('Bot12','BOT_TEST'),('Bot13','BOT_TEST'),('Bot14','BOT_TEST'),('Bot15','BOT_TEST'),('Bot16','BOT_TEST'),('Bot17','BOT_TEST'),('Bot18','BOT_TEST'),('Bot19','BOT_TEST'),('Bot2','BOT_TEST'),('Bot20','BOT_TEST'),('Bot21','BOT_TEST'),('Bot22','BOT_TEST'),('Bot23','BOT_TEST'),('Bot24','BOT_TEST'),('Bot25','BOT_TEST'),('Bot26','BOT_TEST'),('Bot27','BOT_TEST'),('Bot28','BOT_TEST'),('Bot29','BOT_TEST'),('Bot3','BOT_TEST'),('Bot30','BOT_TEST'),('Bot31','BOT_TEST'),('Bot32','BOT_TEST'),('Bot33','BOT_TEST'),('Bot34','BOT_TEST'),('Bot35','BOT_TEST'),('Bot36','BOT_TEST'),('Bot37','BOT_TEST'),('Bot38','BOT_TEST'),('Bot39','BOT_TEST'),('Bot4','BOT_TEST'),('Bot40','BOT_TEST'),('Bot41','BOT_TEST'),('Bot42','BOT_TEST'),('Bot43','BOT_TEST'),('Bot44','BOT_TEST'),('Bot45','BOT_TEST'),('Bot46','BOT_TEST'),('Bot47','BOT_TEST'),('Bot48','BOT_TEST'),('Bot49','BOT_TEST'),('Bot5','BOT_TEST'),('Bot50','BOT_TEST'),('Bot51','BOT_TEST'),('Bot52','BOT_TEST'),('Bot53','BOT_TEST'),('Bot54','BOT_TEST'),('Bot55','BOT_TEST'),('Bot56','BOT_TEST'),('Bot57','BOT_TEST'),('Bot58','BOT_TEST'),('Bot59','BOT_TEST'),('Bot6','BOT_TEST'),('Bot60','BOT_TEST'),('Bot61','BOT_TEST'),('Bot62','BOT_TEST'),('Bot63','BOT_TEST'),('Bot64','BOT_TEST'),('Bot65','BOT_TEST'),('Bot66','BOT_TEST'),('Bot67','BOT_TEST'),('Bot68','BOT_TEST'),('Bot69','BOT_TEST'),('Bot7','BOT_TEST'),('Bot70','BOT_TEST'),('Bot71','BOT_TEST'),('Bot72','BOT_TEST'),('Bot73','BOT_TEST'),('Bot74','BOT_TEST'),('Bot75','BOT_TEST'),('Bot76','BOT_TEST'),('Bot77','BOT_TEST'),('Bot78','BOT_TEST'),('Bot79','BOT_TEST'),('Bot8','BOT_TEST'),('Bot80','BOT_TEST'),('Bot81','BOT_TEST'),('Bot82','BOT_TEST'),('Bot83','BOT_TEST'),('Bot84','BOT_TEST'),('Bot85','BOT_TEST'),('Bot86','BOT_TEST'),('Bot87','BOT_TEST'),('Bot88','BOT_TEST'),('Bot89','BOT_TEST'),('Bot9','BOT_TEST'),('Bot90','BOT_TEST'),('Bot91','BOT_TEST'),('Bot92','BOT_TEST'),('Bot93','BOT_TEST'),('Bot94','BOT_TEST'),('Bot95','BOT_TEST'),('Bot96','BOT_TEST'),('Bot97','BOT_TEST'),('Bot98','BOT_TEST'),('Bot99','BOT_TEST'),('hjkl','hjkl'),('qwer',''),('uuuu','uuuu');
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
SET @@SESSION.SQL_LOG_BIN = @MYSQLDUMP_TEMP_LOG_BIN;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-09-03 20:37:32
