INSERT INTO TAT_STS_ALM (PRC_DATE, NODE_NO, DST_YN,
		INDETERMINATE, CRITICAL, MAJOR, MINOR, WARNING, CLEARED, FAULT, NOTICE)
	(
	 SELECT FROM_UNIXTIME(1462755600), NODE_NO, DST_YN,
	 SUM(INDETERMINATE), SUM(CRITICAL), SUM(MAJOR), SUM(MINOR), SUM(WARNING), SUM(CLEARED), SUM(FAULT), SUM(NOTICE)
	 FROM TAT_STS_ALM
	 WHERE PRC_DATE > FROM_UNIXTIME(1462752000) AND PRC_DATE <= FROM_UNIXTIME(1462755600)
	 GROUP BY NODE_NO, DST_YN
	)
ON DUPLICATE KEY UPDATE PRC_DATE = FROM_UNIXTIME(1462755600),
   NODE_NO = VALUES(NODE_NO),
   INDETERMINATE = VALUES(INDETERMINATE),
   CRITICAL =  VALUES(CRITICAL),
   MAJOR =  VALUES(MAJOR),
   MINOR =  VALUES(MINOR),
   WARNING = VALUES(WARNING),
   CLEARED = VALUES(CLEARED),
   FAULT = VALUES(FAULT),
   NOTICE = VALUES(NOTICE)