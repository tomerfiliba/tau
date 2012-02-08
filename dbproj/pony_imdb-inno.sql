-- MySQL dump 10.13  Distrib 5.5.17, for Win32 (x86)
--
-- Host: localhost    Database: DbMysql10
-- ------------------------------------------------------
-- Server version	5.5.17

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

DROP SCHEMA IF EXISTS `DbMysql10`;
CREATE SCHEMA `DbMysql10`;
USE `DbMysql10`;

--
-- Table structure for table `GamePlayers`
--

DROP TABLE IF EXISTS `GamePlayers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `GamePlayers` (
  `user_id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(30) NOT NULL,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `username_UNIQUE` (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Genres`
--

DROP TABLE IF EXISTS `Genres`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Genres` (
  `genre_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(30) NOT NULL,
  PRIMARY KEY (`genre_id`),
  UNIQUE KEY `genre_name_UNIQUE` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Highscores`
--

DROP TABLE IF EXISTS `Highscores`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Highscores` (
  `highscore_id` int(11) NOT NULL AUTO_INCREMENT,
  `user` int(11) NOT NULL,
  `score` int(11) NOT NULL,
  PRIMARY KEY (`highscore_id`),
  KEY `highscore_user` (`user`),
  CONSTRAINT `highscores_user` FOREIGN KEY (`user`) REFERENCES `GamePlayers` (`user_id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `moviedirectors`
--

DROP TABLE IF EXISTS `MovieDirectors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `moviedirectors` (
  `movie_director_id` int(11) NOT NULL AUTO_INCREMENT,
  `movie` int(11) NOT NULL,
  `director` int(11) NOT NULL,
  PRIMARY KEY (`movie_director_id`),
  UNIQUE KEY `movie` (`movie`,`director`),
  KEY `md_movie` (`movie`),
  KEY `md_director` (`director`),
  CONSTRAINT `md_movie` FOREIGN KEY (`movie`) REFERENCES `Movies` (`movie_id`) ON DELETE CASCADE ON UPDATE NO ACTION,
  CONSTRAINT `md_people` FOREIGN KEY (`director`) REFERENCES `People` (`person_id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `MovieGenres`
--

DROP TABLE IF EXISTS `MovieGenres`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `MovieGenres` (
  `movie_genre_id` int(11) NOT NULL AUTO_INCREMENT,
  `genre` int(11) NOT NULL,
  `movie` int(11) NOT NULL,
  PRIMARY KEY (`movie_genre_id`),
  UNIQUE KEY `genre` (`genre`,`movie`),
  KEY `mg_movie` (`movie`),
  KEY `mg_genre` (`genre`),
  CONSTRAINT `mg_movie` FOREIGN KEY (`movie`) REFERENCES `Movies` (`movie_id`) ON DELETE CASCADE ON UPDATE NO ACTION,
  CONSTRAINT `mg_genre` FOREIGN KEY (`genre`) REFERENCES `Genres` (`genre_id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `movies`
--

DROP TABLE IF EXISTS `Movies`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Movies` (
  `movie_id` int(11) NOT NULL AUTO_INCREMENT,
  `imdb_name` varchar(100) NOT NULL,
  `is_film` TINYINT(1) DEFAULT 0,
  `name` varchar(100) DEFAULT NULL,
  `episode` varchar(100) DEFAULT NULL,
  `year` smallint(6) DEFAULT NULL,
  `rating` double DEFAULT NULL,
  `votes` int(11) DEFAULT NULL,
  PRIMARY KEY (`movie_id`),
  UNIQUE KEY `imdb_name_UNIQUE` (`imdb_name`),
  INDEX `movie_is_film` (`is_film` ASC),
  INDEX `movie_year` (`year`),
  INDEX `movie_rating` (`rating` ASC), 
  INDEX `movie_votes` (`votes` ASC),
  INDEX `movie_name` (`name` ASC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `People`
--

DROP TABLE IF EXISTS `People`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `people` (
  `person_id` int(11) NOT NULL AUTO_INCREMENT,
  `imdb_name` varchar(100) NOT NULL,
  `first_name` varchar(80) DEFAULT NULL,
  `middle_name` varchar(80) DEFAULT NULL,
  `last_name` varchar(80) DEFAULT NULL,
  `nick_name` varchar(100) DEFAULT NULL,
  `real_name` varchar(100) DEFAULT NULL,
  `birth_date` date DEFAULT NULL,
  `death_date` date DEFAULT NULL,
  `gender` enum('m','f') DEFAULT NULL,
  PRIMARY KEY (`person_id`),
  UNIQUE KEY `person_imdb_name_UNIQUE` (`imdb_name`),
  INDEX `person_first_name` (`first_name` ASC),
  INDEX `person_last_name` (`last_name` ASC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Roles`
--

DROP TABLE IF EXISTS `Roles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `roles` (
  `role_id` int(11) NOT NULL AUTO_INCREMENT,
  `movie` int(11) NOT NULL,
  `actor` int(11) NOT NULL,
  `char_name` varchar(100) DEFAULT NULL,
  `credit_pos` smallint(6) DEFAULT NULL,
  PRIMARY KEY (`role_id`),
  UNIQUE KEY `movie` (`movie`,`actor`),
  KEY `role_movie` (`movie`),
  KEY `role_actor` (`actor`),
  CONSTRAINT `roles_movie` FOREIGN KEY (`movie`) REFERENCES `Movies` (`movie_id`) ON DELETE CASCADE ON UPDATE NO ACTION,
  CONSTRAINT `roles_person` FOREIGN KEY (`actor`) REFERENCES `People` (`person_id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-02-04 23:27:14
