#include "AlarmSyslog.hpp"

#include "AlarmDataDef.hpp"

AlarmSyslog::AlarmSyslog() 
{
}


AlarmSyslog::~AlarmSyslog()
{
}


void AlarmSyslog::
SendSyslog( ST_AlarmEventMsg & _stAlarmEventMsg )
{
	int			nLogLevel = LOG_DEBUG;
	std::string	strMessage;

	setlogmask (LOG_UPTO (LOG_NOTICE));

	openlog ( "vOFCS", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	//ex) Jun 16 14:32:25 GM ./a.out[31829]: Program started by User 1001
	
	if( _stAlarmEventMsg.event_type_id_ == ALM::eALARM )
	{
		nLogLevel = LOG_CRIT;
	}
	else if( _stAlarmEventMsg.event_type_id_ == ALM::eFAULT )
	{
		nLogLevel = LOG_WARNING;
	}
	else
	{
		nLogLevel = LOG_NOTICE;
	}

	strMessage = GetAlarmSeverity( _stAlarmEventMsg.severity_id_ );
	strMessage.append( std::string(" ") );
	strMessage.append( GetAlarmGroup( _stAlarmEventMsg.event_group_id_ ) );
	strMessage.append( std::string(" ") );
	strMessage.append( _stAlarmEventMsg.probable_cause_ );
	strMessage.append( std::string(" ") );
	strMessage.append( _stAlarmEventMsg.location_ );
	strMessage.append( std::string(" ") );
	strMessage.append( _stAlarmEventMsg.value_ );

	syslog (nLogLevel, strMessage.c_str() );

	closelog ();

	/*
	body["message"]         =   _stAlarmEventMsg.message_;
	body["node_seq_id"]     =   _stAlarmEventMsg.sequence_id_;

	body["node_no"]         =   _stAlarmEventMsg.node_no_;
	body["node_name"]       =   _stAlarmEventMsg.node_name_;
	body["proc_no"]         =   _stAlarmEventMsg.proc_no_;
	body["severity_id"]     =   _stAlarmEventMsg.severity_id_;

	body["pkg_name"]        =   _stAlarmEventMsg.pkg_name_;
	body["event_type_id"]   =   _stAlarmEventMsg.event_type_id_;
	body["event_group_id"]  =   _stAlarmEventMsg.event_group_id_;
	body["code"]            =   _stAlarmEventMsg.code_;
	body["alias_code"]      =   _stAlarmEventMsg.alias_code_;
	body["probable_cause"]  =   _stAlarmEventMsg.probable_cause_;
	body["additional_text"] =   _stAlarmEventMsg.additional_text_;
	body["location"]        =   _stAlarmEventMsg.location_;
	body["target"]          =   _stAlarmEventMsg.target_;
	body["complement"]      =   _stAlarmEventMsg.complement_;
	body["value"]           =   _stAlarmEventMsg.value_;
	body["node_version"]    =   _stAlarmEventMsg.node_version_;
	body["node_type"]       =   _stAlarmEventMsg.node_type_;
	body["prc_date"]        =   _stAlarmEventMsg.prc_date_;
	body["dst_yn"]          =   _stAlarmEventMsg.dst_yn_;
	body["vnfm_yn"]         =   _stAlarmEventMsg.vnfm_yn_;
	*/

}

std::string AlarmSyslog::
GetAlarmGroup( int a_nAlarmGroupId )
{
	std::string	strAlarmGroup;

	switch( a_nAlarmGroupId )
	{
	case 0:
		strAlarmGroup = std::string( "Unknown" );
		break;
	case 1:
		strAlarmGroup = std::string( "Communication" );
		break;
	case 2:
		strAlarmGroup = std::string( "Processing_Error" );
		break;
	case 3:
		strAlarmGroup = std::string( "Environmental" );
		break;
	case 4:
		strAlarmGroup = std::string( "Quality_of_Service" );
		break;
	case 5:
		strAlarmGroup = std::string( "Equipment" );
		break;
	case 6:
		strAlarmGroup = std::string( "Threshold_Crossing_Alert" );
		break;
	default:
		strAlarmGroup = std::string( "Etc" );
		break;
	}

	return strAlarmGroup;
}

std::string AlarmSyslog::
GetAlarmSeverity( int a_nAlarmSeverityId )
{
	std::string	strAlarmSeverity;

	switch( a_nAlarmSeverityId )
	{
	case 0:
		strAlarmSeverity = std::string( "INTERMINATE" );
		break;
	case 1:
		strAlarmSeverity = std::string( "CRITICAL" );
		break;
	case 2:
		strAlarmSeverity = std::string( "MAJOR" );
		break;
	case 3:
		strAlarmSeverity = std::string( "MINOR" );
		break;
	case 4:
		strAlarmSeverity = std::string( "WARNING" );
		break;
	case 5:
		strAlarmSeverity = std::string( "CLEARED" );
		break;
	case 6:
		strAlarmSeverity = std::string( "FAULT" );
		break;
	default:
		strAlarmSeverity = std::string( "NOTICE" );
		break;
	}

	return strAlarmSeverity;
}
