-- MySQL dump 10.13  Distrib 8.0.33, for Win64 (x86_64)
--
-- Host: localhost    Database: Tracker
-- ------------------------------------------------------
-- Server version	8.0.33

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `Tracker`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `Tracker` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci */ /*!80016 DEFAULT ENCRYPTION='N' */;

USE `Tracker`;

--
-- Table structure for table `Message`
--

DROP TABLE IF EXISTS `Message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `Message` (
  `fromUserID` int NOT NULL,
  `toUserID` int unsigned NOT NULL,
  `dateSent` varchar(15) DEFAULT NULL,
  `fromUserName` varchar(32) NOT NULL,
  `body` varchar(255) NOT NULL,
  `flags` int DEFAULT NULL,
  `deleteDate` varchar(15) DEFAULT NULL,
  `messageID` int NOT NULL AUTO_INCREMENT,
  `minimumBuild` int DEFAULT '0',
  `maximumBuild` int DEFAULT '65535',
  PRIMARY KEY (`messageID`),
  UNIQUE KEY `messageID_UNIQUE` (`messageID`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `User`
--

DROP TABLE IF EXISTS `User`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `User` (
  `userID` int unsigned NOT NULL,
  `firstName` varchar(32) DEFAULT NULL,
  `lastName` varchar(32) DEFAULT NULL,
  `nickName` varchar(32) DEFAULT NULL,
  `password` varchar(32) DEFAULT NULL,
  `emailAddress` varchar(128) DEFAULT NULL,
  `firstLogon` varchar(15) DEFAULT NULL,
  `lastLogon` varchar(15) DEFAULT NULL,
  `logonCount` int DEFAULT NULL,
  PRIMARY KEY (`userID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `UserAuth`
--

DROP TABLE IF EXISTS `UserAuth`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `UserAuth` (
  `userID` int unsigned NOT NULL,
  `targetID` int unsigned DEFAULT NULL,
  `authLevel` int DEFAULT NULL,
  PRIMARY KEY (`userID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `UserBlocked`
--

DROP TABLE IF EXISTS `UserBlocked`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `UserBlocked` (
  `userID` int unsigned NOT NULL,
  `blockedID` int DEFAULT NULL,
  PRIMARY KEY (`userID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `UserDistribution`
--

DROP TABLE IF EXISTS `UserDistribution`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `UserDistribution` (
  `serverName` varchar(32) NOT NULL,
  `catalogName` varchar(32) DEFAULT NULL,
  `backupServerName` varchar(32) DEFAULT NULL,
  `backupCatalogName` varchar(32) DEFAULT NULL,
  `userIDMinimum` int DEFAULT NULL,
  `userIDMaximum` int DEFAULT NULL,
  `nextValidUserId` int DEFAULT NULL,
  `dbtype` int DEFAULT '0',
  PRIMARY KEY (`serverName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `UserStatus`
--

DROP TABLE IF EXISTS `UserStatus`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `UserStatus` (
  `userID` int unsigned NOT NULL,
  `serverID` int unsigned DEFAULT NULL,
  `status` int DEFAULT NULL,
  `userIP` int unsigned DEFAULT NULL,
  `userPort` int unsigned DEFAULT NULL,
  `sessionID` int unsigned DEFAULT NULL,
  `lastModified` datetime DEFAULT NULL,
  `buildNumber` int DEFAULT NULL,
  `gameIP` int unsigned DEFAULT NULL,
  `gamePort` int unsigned DEFAULT NULL,
  `gameType` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`userID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Watcher`
--

DROP TABLE IF EXISTS `Watcher`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `Watcher` (
  `watcherID` int unsigned NOT NULL,
  `watcherSessionID` int unsigned DEFAULT NULL,
  `watcherServerID` int unsigned DEFAULT NULL,
  PRIMARY KEY (`watcherID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `WatchMap`
--

DROP TABLE IF EXISTS `WatchMap`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `WatchMap` (
  `watcherID` int unsigned NOT NULL,
  `targetID` int unsigned NOT NULL,
  PRIMARY KEY (`watcherID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2023-05-06 17:23:41
