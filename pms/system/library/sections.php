<?php
require_once '../../../myMyPlex.inc';

# Bail if we're not happy with the auth token in the URL
$auth = '';
if (array_key_exists('auth_token', $_GET) && strlen($_GET['auth_token'])) {
	$auth = preg_replace('/\W/', '', $_GET['auth_token']);
}
if ($auth == '' || $auth != $_GET['auth_token']) {
	header('HTTP/1.1 401 Authorization Required');
	die("Invalid auth token\n");
}

# Lookup the user
$user = get_user('', $auth);
if (! $user['username']) {
	header('HTTP/1.1 401 Authorization Required');
	die("Unknown auth token\n");
}

# Lookup all the user's servers
$servers = get_server_list($user['username']);

# Lookup all the sections in those servers
$sections = array();
foreach ($servers as $name) {
	$server = get_server($user['username'], $name);
	$sections[] = array(
		'version' => $server['version'],
		'title' => 'myMyMovies',
		'port' => $server['port'],
		'lang' => 'en',
		'server' => $server['name'],
		'id' => $server['id'],
		'type' => 'movie',
		'agent' => 'com.plexapp.agents.imdb',
		'path' => '/library/sections/1',
		'address' => $server['address'],
		'scanner' => 'Plex Movie Scanner',	
	);
}

# Build an XML response
$doc = new DomDocument('1.0');
$root = $doc->createElement('MediaContainer');
$root = $doc->appendChild($root);
$root->setAttribute('title1', 'myMyPlex Library');
$root->setAttribute('identifier', 'com.plexapp.plugins.myplex');
$root->setAttribute('friendlyName', 'myMyPlex');
$root->setAttribute('size', count($sections));
$root->setAttribute('machineIdentifier', '1');

foreach ($sections as $sec) {
	$elm = $doc->createElement('Directory');
	$elm->setAttribute('address', $sec['address']);
	$elm->setAttribute('local', '0');
	$elm->setAttribute('serverVersion', $sec['version']);
	$elm->setAttribute('accessToken', '');
	$elm->setAttribute('title', $sec['title']);
	$elm->setAttribute('updatedAt', date('U'));
	$elm->setAttribute('port', $sec['port']);
	$elm->setAttribute('art', 'http://' . $sec['address'] . ':' . $sec['port'] . '/:/resources/show-fanart.jpg');
	$elm->setAttribute('language', $sec['lang']);
	$elm->setAttribute('serverName', $sec['server']);
	$elm->setAttribute('machineIdentifier', $sec['id']);
	$elm->setAttribute('type', $sec['type']);
	$elm->setAttribute('agent', $sec['agent']);
	$elm->setAttribute('path', $sec['path']);
	$elm->setAttribute('owned', '1');
	$elm->setAttribute('host', $sec['address']);
	$elm->setAttribute('unique', '0');
	$elm->setAttribute('key', 'http://' . $sec['address'] . ':' . $sec['port'] . $sec['path']);
	$elm->setAttribute('scanner', $sec['scanner']);

	$elm = $root->appendChild($elm);
}

# Send our XML to the client
header('HTTP/1.1 200 OK');
print $doc->saveXML();

?>
