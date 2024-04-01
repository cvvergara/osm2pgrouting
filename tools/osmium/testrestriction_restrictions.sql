
BEGIN;
--DROP TABLE if EXISTS foo.osm_restrictions;

--
-- osm2pgRouting osm_restrictions dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: osm_restrictions; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE SCHEMA IF NOT EXISTS foo;

CREATE TABLE IF NOT EXISTS foo.osm_restrictions(
    osm_id BIGINT PRIMARY KEY,
    osm_from    BIGINT[],
    osm_to      BIGINT[],
    osm_via     BIGINT[],
    via_type char,
    osm_location_hint BIGINT,
    attributes hstore,
    tags hstore);

-- On a created table the temporary table is not needed
CREATE TEMP TABLE __osm_restrictions_tmp
ON COMMIT DROP
AS
SELECT *
FROM foo.osm_restrictions
WITH NO DATA;


-- On a created table Use the original table
COPY __osm_restrictions_tmp (
    osm_id,
    osm_from, osm_to, osm_via,
    via_type,
    osm_location_hint,
    attributes, tags) FROM STDIN;
2654080	{30513235}	{30513221}	{336812979}	n	\N	version=>1,timestamp=>2012-12-22T17:01:50Z,changeset=>14368535,uid=>381316,user=>Schermy	except=>hgv,restriction=>no_right_turn,type=>restriction 
7715924	{35575825}	{35575826}	{416884019}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715926	{35575831}	{35575832}	{75753405}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715925	{35575834}	{35575823}	{87383157}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715930	{35604373}	{35604377}	{336814845}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715929	{35604384}	{35604369}	{75753390}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
2801218	{51392054}	{52385458}	{666177031}	n	\N	version=>1,timestamp=>2013-03-04T22:08:50Z,changeset=>15252425,uid=>191979,user=>"It''s so funny"	restriction=>no_left_turn,type=>restriction 
9197760	{61406676}	{76587242}	{550024969}	n	\N	version=>1,timestamp=>2019-01-09T00:13:09Z,changeset=>66144658,uid=>134881,user=>Laci	restriction=>no_left_turn,type=>restriction 
2654079	{93448457}	{93448446}	{75736605}	n	\N	version=>1,timestamp=>2012-12-22T17:01:50Z,changeset=>14368535,uid=>381316,user=>Schermy	restriction=>no_right_turn,type=>restriction 
9894634	{103233065}	{51450192}	{910539164}	n	\N	version=>2,timestamp=>2019-08-07T16:00:43Z,changeset=>73119153,uid=>5589496,user=>willisturm	restriction=>only_right_turn,type=>restriction 
9894697	{51450645}	{123269724}	{344471055}	n	\N	version=>1,timestamp=>2019-08-07T16:00:43Z,changeset=>73119153,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
1811355	{134667169}	{134667172}	{365181022}	n	\N	version=>1,timestamp=>2011-10-26T16:57:11Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
1810047	{32479918}	{134667172}	{365181022}	n	\N	version=>1,timestamp=>2011-10-25T17:00:11Z,changeset=>9653862,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
1810084	{30512516}	{134667174}	{75740064}	n	\N	version=>2,timestamp=>2022-02-01T19:02:24Z,changeset=>116880741,uid=>8513125,user=>GroßarlFuchs3847	hour_off=>"20:00",hour_on=>"06:00",restriction=>only_right_turn,type=>restriction 
1810046	{134667259}	{134667174}	{1480288168}	n	\N	version=>2,timestamp=>2013-02-05T21:28:19Z,changeset=>14927175,uid=>62882,user=>friedl	restriction=>only_right_turn,type=>restriction 
1810042	{134667267}	{32479917}	{256343965}	n	\N	version=>2,timestamp=>2022-02-02T15:01:29Z,changeset=>116917533,uid=>8513125,user=>GroßarlFuchs3847	restriction=>only_straight_on,type=>restriction 
1810071	{134669690}	{117553923}	{1324080118}	n	\N	version=>1,timestamp=>2011-10-25T17:34:25Z,changeset=>9654142,uid=>381316,user=>Schermy	restriction=>no_left_turn,type=>restriction 
1810091	{117553923}	{134669690}	{1324080118}	n	\N	version=>1,timestamp=>2011-10-25T17:34:29Z,changeset=>9654142,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
10487495	{134669693}	{117539011}	{1323971153}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>no_left_turn,type=>restriction 
10487494	{117539011}	{134669693}	{1323971153}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>only_right_turn,type=>restriction 
10487493	{134669693}	{117539026}	{1323971411}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>no_left_turn,type=>restriction 
10487499	{134669695}	{117539017}	{1323971282}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>no_left_turn,type=>restriction 
10487496	{117539017}	{134669695}	{1323971282}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>only_right_turn,type=>restriction 
10490818	{117531897}	{134669698}	{9473900814}	n	\N	version=>3,timestamp=>2022-02-06T15:07:12Z,changeset=>117082546,uid=>161619,user=>FvGordon	restriction=>only_straight_on,type=>restriction 
1811241	{134792472}	{134792461}	{1481382061}	n	\N	version=>2,timestamp=>2022-02-01T18:54:34Z,changeset=>116880741,uid=>8513125,user=>GroßarlFuchs3847	restriction=>only_left_turn,type=>restriction 
1811350	{29979299}	{134800098}	{130105922}	n	\N	version=>1,timestamp=>2011-10-26T16:57:10Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
1811346	{27786794}	{134800098}	{75740038}	n	\N	version=>1,timestamp=>2011-10-26T16:57:10Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>no_left_turn,type=>restriction 
1811348	{134800098}	{134800101}	{75740038}	n	\N	version=>1,timestamp=>2011-10-26T16:57:10Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
2253066	{134800104}	{134800101}	{1481435308}	n	\N	version=>1,timestamp=>2012-06-26T21:17:32Z,changeset=>12031072,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
1811353	{134800103}	{134800105}	{1481435078}	n	\N	version=>3,timestamp=>2012-12-22T17:01:50Z,changeset=>14368535,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
1811352	{134800102}	{134800107}	{1481434979}	n	\N	version=>1,timestamp=>2011-10-26T16:57:11Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
1811354	{35684420}	{134800107}	{1480288621}	n	\N	version=>1,timestamp=>2011-10-26T16:57:11Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
1811356	{134800114}	{134800101}	{1481435308}	n	\N	version=>1,timestamp=>2011-10-26T16:57:11Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
1811359	{134800168}	{134800103}	{1481435062}	n	\N	version=>1,timestamp=>2011-10-26T16:57:12Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
1811357	{134800161}	{134800168}	{1481435048}	n	\N	version=>1,timestamp=>2011-10-26T16:57:12Z,changeset=>9661858,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
4610880	{103233043}	{135384444}	{1546525085}	n	\N	version=>2,timestamp=>2019-08-07T15:13:35Z,changeset=>73117404,uid=>5589496,user=>willisturm	restriction=>no_u_turn,type=>restriction 
5837233	{140130044}	{111360011}	{500897772}	n	\N	version=>1,timestamp=>2016-01-08T09:20:21Z,changeset=>36440706,uid=>381316,user=>Schermy	except=>hgv;psv,restriction=>only_right_turn,type=>restriction 
1811358	{169162876}	{134800100}	{1481435234}	n	\N	version=>2,timestamp=>2012-06-26T21:17:33Z,changeset=>12031072,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
2654078	{184614937}	{178195372}	{87383165}	n	\N	version=>2,timestamp=>2019-07-22T10:21:00Z,changeset=>72508084,uid=>61526,user=>"gwefui  "	except=>hgv,restriction=>no_left_turn,type=>restriction 
1610805	{116138561}	{192504097}	{310135708}	n	\N	version=>2,timestamp=>2012-11-24T18:55:14Z,changeset=>14017349,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
2671756	{198931052}	{198931055}	{500876392}	n	\N	version=>1,timestamp=>2012-12-31T14:04:02Z,changeset=>14477789,uid=>8748,user=>ToniE	restriction=>only_straight_on,type=>restriction 
2689202	{200145938}	{23037866}	{248540238}	n	\N	version=>1,timestamp=>2013-01-08T12:02:27Z,changeset=>14573821,uid=>202392,user=>imagic	restriction=>no_u_turn,type=>restriction 
2689203	{200145940}	{200145943}	{656081270}	n	\N	version=>1,timestamp=>2013-01-08T12:02:27Z,changeset=>14573821,uid=>202392,user=>imagic	restriction=>no_u_turn,type=>restriction 
2691024	{200257263}	{82719424}	{305106223}	n	\N	version=>3,timestamp=>2017-09-04T15:41:50Z,changeset=>51724177,uid=>115042,user=>gnlpfth	restriction=>only_straight_on,type=>restriction 
2691023	{82719424}	{200257275}	{2102544542}	n	\N	version=>5,timestamp=>2017-09-04T15:41:20Z,changeset=>51724177,uid=>115042,user=>gnlpfth	restriction=>only_straight_on,type=>restriction 
2691022	{200257276}	{82719425}	{305106228}	n	\N	version=>2,timestamp=>2017-11-07T17:39:23Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>only_straight_on,type=>restriction 
2691021	{200257283}	{200257271}	{1806989866}	n	\N	version=>5,timestamp=>2018-02-13T13:17:59Z,changeset=>56323464,uid=>115042,user=>gnlpfth	restriction=>only_straight_on,type=>restriction 
2742488	{204313935}	{45066207}	{2143198454}	n	\N	version=>1,timestamp=>2013-02-05T21:28:19Z,changeset=>14927175,uid=>62882,user=>friedl	restriction=>only_straight_on,type=>restriction 
1810086	{117549965}	{204313945}	{1324042037}	n	\N	version=>2,timestamp=>2013-02-05T21:28:20Z,changeset=>14927175,uid=>62882,user=>friedl	restriction=>only_right_turn,type=>restriction 
2742486	{204313938}	{204313945}	{2143198462}	n	\N	version=>1,timestamp=>2013-02-05T21:28:19Z,changeset=>14927175,uid=>62882,user=>friedl	restriction=>only_straight_on,type=>restriction 
1811347	{134800100}	{217941423}	{75740047}	n	\N	version=>3,timestamp=>2013-04-19T10:15:20Z,changeset=>15783262,uid=>45347,user=>eriosw	restriction=>only_straight_on,type=>restriction 
2982836	{133352940}	{224511834}	{1467975033}	n	\N	version=>1,timestamp=>2013-06-05T16:54:44Z,changeset=>16434595,uid=>12295,user=>mapper_07	restriction=>only_left_turn,type=>restriction 
2982838	{133352941}	{224511836}	{1467975041}	n	\N	version=>1,timestamp=>2013-06-05T16:54:44Z,changeset=>16434595,uid=>12295,user=>mapper_07	restriction=>only_right_turn,type=>restriction 
4185810	{224511836}	{133352942}	{1546531682}	n	\N	version=>1,timestamp=>2014-11-11T20:23:19Z,changeset=>26721235,uid=>2675,user=>"Eckhart Wörner"	restriction=>no_left_turn,type=>restriction 
4185813	{224511834}	{224511837}	{1546531681}	n	\N	version=>2,timestamp=>2019-11-29T11:32:57Z,changeset=>77724720,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
4185814	{312313393}	{32952425}	{1467975030}	n	\N	version=>1,timestamp=>2014-11-11T20:23:20Z,changeset=>26721235,uid=>2675,user=>"Eckhart Wörner"	restriction=>only_straight_on,type=>restriction 
12160603	{55286974}	{529556788}	{694591436}	n	\N	version=>1,timestamp=>2021-01-09T23:51:26Z,changeset=>97232627,uid=>134881,user=>Laci	restriction=>only_straight_on,type=>restriction 
7715918	{539038351}	{539038348}	{1806989895}	n	\N	version=>1,timestamp=>2017-11-07T17:38:48Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715919	{539038347}	{539038352}	{5216562555}	n	\N	version=>1,timestamp=>2017-11-07T17:38:48Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715920	{539038353}	{539038350}	{305106214}	n	\N	version=>2,timestamp=>2018-12-26T12:00:14Z,changeset=>65782444,uid=>8513125,user=>GroßarlFuchs3847	restriction=>no_u_turn,type=>restriction 
7715922	{539038361}	{539038360}	{5216562600}	n	\N	version=>1,timestamp=>2017-11-07T17:38:48Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715923	{539038359}	{539038362}	{305106211}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715921	{539038364}	{539038363}	{2620884259}	n	\N	version=>1,timestamp=>2017-11-07T17:38:48Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715928	{539038383}	{539038390}	{1193034190}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7715927	{539038393}	{539038396}	{5216562804}	n	\N	version=>1,timestamp=>2017-11-07T17:38:49Z,changeset=>53587706,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
7771305	{543744040}	{126733795}	{5256319667}	n	\N	version=>1,timestamp=>2017-11-30T18:02:30Z,changeset=>54212457,uid=>381316,user=>Schermy	restriction=>only_right_turn,type=>restriction 
7771306	{543744073}	{103233019}	{1191908663}	n	\N	version=>1,timestamp=>2017-11-30T18:02:30Z,changeset=>54212457,uid=>381316,user=>Schermy	restriction=>only_straight_on,type=>restriction 
7715931	{597871922}	{539038414}	{417285342}	n	\N	version=>2,timestamp=>2018-06-14T17:45:58Z,changeset=>59853774,uid=>781412,user=>TBKMrt	restriction=>no_u_turn,type=>restriction 
8594387	{619798094}	{619759717}	{5856359300}	n	\N	version=>1,timestamp=>2018-08-25T14:27:28Z,changeset=>61986451,uid=>8513125,user=>GroßarlFuchs3847	restriction=>no_left_turn,type=>restriction 
8593850	{638691041}	{621189387}	{5756893766}	n	\N	version=>3,timestamp=>2018-10-28T10:00:38Z,changeset=>63945701,uid=>8513125,user=>GroßarlFuchs3847	restriction=>no_left_turn,type=>restriction 
9894636	{77857488}	{661615388}	{915254297}	n	\N	version=>1,timestamp=>2019-08-07T15:34:45Z,changeset=>73118273,uid=>5589496,user=>willisturm	restriction=>only_right_turn,type=>restriction 
9894635	{661615388}	{4302733}	{915254297}	n	\N	version=>1,timestamp=>2019-08-07T15:34:45Z,changeset=>73118273,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
12460501	{661615389}	{61406676}	{766715627}	n	\N	version=>1,timestamp=>2021-03-18T09:03:08Z,changeset=>101243160,uid=>8954340,user=>MartorCZ	restriction=>no_left_turn,type=>restriction 
9894526	{141278165}	{711717778}	{1546525064}	n	\N	version=>1,timestamp=>2019-08-07T15:05:00Z,changeset=>73116998,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
9894527	{135384442}	{711717779}	{1546525100}	n	\N	version=>1,timestamp=>2019-08-07T15:05:00Z,changeset=>73116998,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
9894698	{103233058}	{711726134}	{1375275591}	n	\N	version=>1,timestamp=>2019-08-07T16:00:43Z,changeset=>73119153,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
9894694	{711726135}	{103233053}	{1191908884}	n	\N	version=>1,timestamp=>2019-08-07T16:00:43Z,changeset=>73119153,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
7771307	{661615377}	{711726135}	{344471413}	n	\N	version=>3,timestamp=>2019-08-07T15:34:45Z,changeset=>73118273,uid=>5589496,user=>willisturm	restriction=>only_left_turn,type=>restriction 
9894699	{661615378}	{711733220}	{26016595}	n	\N	version=>1,timestamp=>2019-08-07T16:00:43Z,changeset=>73119153,uid=>5589496,user=>willisturm	restriction=>only_left_turn,type=>restriction 
9894695	{711733220}	{447004318}	{26016597}	n	\N	version=>1,timestamp=>2019-08-07T16:00:43Z,changeset=>73119153,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
2533684	{93448446}	{749018997}	{75736605}	n	\N	version=>2,timestamp=>2019-11-23T16:55:57Z,changeset=>77463810,uid=>9322634,user=>michael_77	restriction=>only_right_turn,type=>restriction 
10330427	{749197894}	{749197894}	{1445258148}	n	\N	version=>1,timestamp=>2019-11-24T14:17:38Z,changeset=>77483196,uid=>8513125,user=>GroßarlFuchs3847	restriction=>no_u_turn,type=>restriction 
9894696	{749852650}	{711726132}	{26016592}	n	\N	version=>2,timestamp=>2019-11-26T16:40:08Z,changeset=>77587312,uid=>5589496,user=>willisturm	restriction=>only_straight_on,type=>restriction 
1810090	{754852324}	{134669694}	{1323917172}	n	\N	version=>3,timestamp=>2019-12-12T09:48:16Z,changeset=>78308840,uid=>5589496,user=>willisturm	restriction=>only_right_turn,type=>restriction 
1810072	{134669694}	{754852324}	{1323917172}	n	\N	version=>3,timestamp=>2019-12-12T09:48:16Z,changeset=>78308840,uid=>5589496,user=>willisturm	restriction=>no_left_turn,type=>restriction 
10487500	{758998012}	{117539018}	{1323971156}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>no_left_turn,type=>restriction 
10487498	{758998012}	{117539025}	{1323971282}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>no_left_turn,type=>restriction 
10487497	{117539025}	{758998012}	{1323971282}	n	\N	version=>1,timestamp=>2019-12-27T16:23:11Z,changeset=>78933937,uid=>5589496,user=>willisturm	restriction=>only_right_turn,type=>restriction 
13339819	{619761649}	{784448224}	{7328347388}	n	\N	version=>1,timestamp=>2021-10-18T16:38:42Z,changeset=>112661201,uid=>817113,user=>santon	restriction=>no_u_turn,type=>restriction 
13339820	{784448229}	{784448230}	{7328347393}	n	\N	version=>1,timestamp=>2021-10-18T16:38:42Z,changeset=>112661201,uid=>817113,user=>santon	restriction=>no_u_turn,type=>restriction 
9894637	{103233053}	{918594823}	{1191908952}	n	\N	version=>3,timestamp=>2021-03-18T09:03:08Z,changeset=>101243160,uid=>8954340,user=>MartorCZ	restriction=>only_straight_on,type=>restriction 
1810039	{134667165}	{1026803350}	{9466765052}	n	\N	version=>6,timestamp=>2022-02-06T15:19:38Z,changeset=>117082987,uid=>161619,user=>FvGordon	restriction=>no_left_turn,type=>restriction 
1810077	{1028485838}	{134669698}	{9473900814}	n	\N	version=>6,timestamp=>2022-02-06T15:07:12Z,changeset=>117082546,uid=>161619,user=>FvGordon	restriction=>only_straight_on,type=>restriction 
1810048	{123764395}	{1028669458}	{9466899021}	n	\N	version=>5,timestamp=>2022-02-06T15:19:38Z,changeset=>117082987,uid=>161619,user=>FvGordon	hour_off=>"20:00",hour_on=>"06:00",restriction=>only_right_turn,type=>restriction 
1810044	{1026803349}	{1028669458}	{9466765065}	n	\N	version=>5,timestamp=>2022-02-06T15:19:38Z,changeset=>117082987,uid=>161619,user=>FvGordon	hour_off=>"20:00",hour_on=>"06:00",restriction=>only_right_turn,type=>restriction 
\.

-- On a created table the delete is not needed
-- FROM pg >= 9.5 this delete is not needed
DELETE FROM __osm_restrictions_tmp WHERE osm_id IN (SELECT osm_id FROM foo.osm_restrictions);

INSERT INTO foo.osm_restrictions
SELECT *
FROM __osm_restrictions_tmp;
-- FROM pg >= 9.5 this line is needed
-- ON CONFLICT DO NOTHING

-- With if the table had the primary key then it wont create it
DO $$
    BEGIN
        BEGIN
            ALTER TABLE ONLY foo.osm_restrictions
            ADD CONSTRAINT foo.osm_restrictions_pkey PRIMARY KEY (osm_id);
            EXCEPTION
               WHEN OTHERS THEN
                RAISE NOTICE 'Primary key "foo.osm_restrictions_pkey" EXISTS'
                USING HINT = 'postgreSQL code #' ||  SQLSTATE || ': ' || SQLERRM;
        END;
    END
$$;

COMMIT;
