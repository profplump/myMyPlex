<?php

# Test params
$_SERVER['PHP_AUTH_USER'] = 'profplump';

#=========================================
# Parameters/Globals
#=========================================
$USER_DB = 'sqlite:/var/state/myMyPlex/db.sqlite';
$DBH = NULL;

# Prevent PHP from whining - the actual timezone you set is irrelevent
date_default_timezone_set('UTC');


#=========================================
# Error Handler
#=========================================
function error($errno, $errstr, $errfile = '', $errline = '') {
	header('HTTP/1.1 500 myMyPlex Processing Error');
	$msg = 'myMyPlex Processing Error';
	if ($errno) {
		$msg .= ' ' . $errno;
	}
	$msg .= ' (' . $errstr . ')';
	if ($errfile) {
		$msg .= ' in ' . $errfile;
	}
	if ($errline) {
		$msg .= ' at ' . $errline;
	}
	die($msg . "\n");
}
set_error_handler('error');


#=========================================
# DB Conneciton/Table Init
#=========================================
function get_dbh() {
	global $USER_DB;
	global $DBH;

	# If we haven't already connected in this session
	if (! $DBH) {
		# Connect
		$dbh = new PDO($USER_DB);
		if (! $dbh) {
			error(0, 'Unable to connect to DB');
		}
		$DBH = $dbh;

		# Create the user table if needed
		$dbh->query('
			CREATE TABLE IF NOT EXISTS users 
			(username VARCHAR(64), auth CHAR(32),
			PRIMARY KEY(username), UNIQUE(auth))
		');

		# Create the server table if needed
		$dbh->query('
			CREATE TABLE IF NOT EXISTS servers 
			(username VARCHAR(64), name VARCHAR(64),
			address CHAR(15), port INT, id CHAR(40),
			version VARCHAR(32),
			PRIMARY KEY(username, name), UNIQUE(id))
		');
	}

	# Return either a new or existing DB ref
	return $DBH;
}


#=========================================
# Get User
#=========================================
function get_user($username = '', $auth = '') {
	$user = array(
		'username' => '',
		'auth' => ''
	);

	try {
		# Query for the requested user
		$dbh = get_dbh();
		if (strlen($username)) {
			$stmt = $dbh->prepare('SELECT username, auth FROM users WHERE username = ?');
			$stmt->execute(array($username));
		} else {
			$stmt = $dbh->prepare('SELECT username, auth FROM users WHERE auth = ?');
			$stmt->execute(array($auth));
		}
		$row = $stmt->fetch(PDO::FETCH_ASSOC);

		# If the users exists, map it into our local array
		if ($row) {
			foreach (array_keys($user) as $key) {
				$user[$key] = $row[$key];
			}
		}
	} catch (PDOException $e) {
		error(0, 'DB error in get_user: ' . $e->getMessage());
	}

	return $user;
}

#=========================================
# Save User
#=========================================
function save_user($user) {
	try {
		# Get connected
		$dbh = get_dbh();

		# Create the user if necessary
		$stmt = $dbh->prepare('SELECT username FROM users WHERE username = ?');
		$stmt->execute(array($user['username']));
		if (! is_array($stmt->fetch())) {
			$stmt = $dbh->prepare('INSERT INTO users (username) VALUES (?)');
			$stmt->execute(array($user['username']));
		}

		# Update the DB from our local array
		$stmt = $dbh->prepare('UPDATE users SET auth = ? WHERE username = ?');
		$stmt->execute(array($user['auth'], $user['username']));
	} catch (PDOException $e) {
		error(0, 'DB error in save_user: ' . $e->getMessage());
	}

	return TRUE;
}


#=========================================
# Get Server
#=========================================
function get_server($username, $name) {
	$server = array(
		'username' => '',
		'name' => '',
		'address' => '',
		'port' => '',
		'id' => '',
		'version' => '',
	);

	try {
		# Query for the requested user
		$dbh = get_dbh();
		$stmt = $dbh->prepare('
			SELECT username, name, address, port, id, version
			FROM servers WHERE username = ? AND name = ?
		');
		$stmt->execute(array($username, $name));
		$row = $stmt->fetch(PDO::FETCH_ASSOC);

		# If the users exists, map it into our local array
		if ($row) {
			foreach (array_keys($server) as $key) {
				$server[$key] = $row[$key];
			}
		}
	} catch (PDOException $e) {
		error(0, 'DB error in get_server: ' . $e->getMessage());
	}

	return $server;
}


#=========================================
# Save Server
#=========================================
function save_server($server) {
	try {
		# Get connected
		$dbh = get_dbh();

		# Create the server if necessary
		$stmt = $dbh->prepare('SELECT name FROM servers WHERE username = ? AND name = ?');
		$stmt->execute(array($server['username'], $server['name']));
		if (! is_array($stmt->fetch())) {
			$stmt = $dbh->prepare('INSERT INTO servers (username, name, id) VALUES (?, ?, ?)');
			$stmt->execute(array($server['username'], $server['name']));
		}

		# Update the DB from our local array
		$stmt = $dbh->prepare('UPDATE servers SET address = ?, port = ?, id = ? WHERE username = ? AND name = ?');
		$stmt->execute(array($server['address'], $server['port'], $server['id'], $server['username'], $server['name']));
	} catch (PDOException $e) {
		error(0, 'DB error in save_server: ' . $e->getMessage());
	}

	return TRUE;
}


#=========================================
# Delete Server
#=========================================
function delete_server($id) {
	try {
		$dbh = get_dbh();
		$stmt = $dbh->prepare('DELETE FROM servers WHERE id = ?');
		$stmt->execute(array($id));
	} catch (PDOException $e) {
		error(0, 'DB error in save_server: ' . $e->getMessage());
	}

	return TRUE;
}


#=========================================
# Get Server List
#=========================================
function get_server_list($username) {
	$servers = array();

	try {
		# Query for the requested user
		$dbh = get_dbh();
		$stmt = $dbh->prepare('SELECT name FROM servers WHERE username = ?');
		$stmt->execute(array($username));

		$rs = $stmt->fetchAll(PDO::FETCH_ASSOC);
		foreach ($rs as $row) {
			$servers[] = $row['name'];
		}
	} catch (PDOException $e) {
		error(0, 'DB error in get_server_list: ' . $e->getMessage());
	}

	return $servers;
}


#=========================================
# Generate Auth Tokens
#=========================================
function generate_auth() {
	return sprintf('%08x%08x%08x%08x',
		mt_rand(0, 0xffffffff), mt_rand(0, 0xffffffff),
		mt_rand(0, 0xffffffff), mt_rand(0, 0xffffffff));
}

?>
