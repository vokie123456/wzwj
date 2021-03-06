<?php
/**
 * @file
 * Functions for integrating the Rules module with HybridAuth.
 */

/**
 * Implements hook_rules_action_info().
 */
function hybridauth_rules_action_info() {
  return array(
    'hybridauth_url_managed_file' => array(
      'label' => t('Save remote file to managed file'),
      'group' => t('HybridAuth'),
      'parameter' => array(
        'url' => array(
          'label' => t('URL'),
          'type' => 'uri',
          'description' => t('URL to the remote file.'),
        ),
        'directory' => array(
          'label' => t('File directory'),
          'type' => 'text',
          'description' => t('File directory to save remote file to.'),
        ),
      ),
      'provides' => array(
        'file' => array(
          'label' => 'Managed file entity',
          'type' => 'file',
        ), 
      ),
    ),
  );
}

function hybridauth_url_managed_file($url, $directory) {
  if ($url) {
    $url_info = parse_url($url);
    $directory = file_default_scheme() . '://' . $directory . '/' . $url_info['host'] . dirname($url_info['path']);
    file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
    $content = drupal_http_request($url);
	  $file = file_save_data($content->data, $directory . '/' . basename($url_info['path']), FILE_EXISTS_REPLACE);
	  
	  $path_info = pathinfo($url);
	  if (empty($path_info['extension'])) {
	    $info = image_get_info($file->uri);
	    $file = file_move($file, $directory . '/' . basename($url_info['path']) . '.' . $info['extension'], FILE_EXISTS_REPLACE);
	  }
  }
  
  return array('file' => $file);
}

/**
 * Implements hook_rules_data_info().
 * @ingroup rules
 */
function hybridauth_rules_data_info() {
  $types = array();
  
  $types['hybridauth'] = array(
    'label' => 'HybridAuth identity data',
    'wrap' => TRUE,
    'property info' => _hybridauth_rules_data_properties(),
  );
  
  return $types;
}

function _hybridauth_rules_data_properties() {
  $properties = array();
  
  if (function_exists('hybridauth_token_info')) {
    $token_info = hybridauth_token_info();
  }
  else {
    include_once(drupal_get_path('module', 'hybridauth') . '/hybridauth.tokens.inc');
    $token_info = hybridauth_token_info();
  }
  foreach ($token_info['tokens']['hybridauth'] as $key => $value) {
    $properties[$key] = array(
      'label' => $value['name'],
      'type' => 'text',
      'description' => $value['description'],
    );
  }
  
  return $properties;
}

/**
 * Implements hook_rules_event_info().
 * @ingroup rules
 */
function hybridauth_rules_event_info() {
  $events = array();
  
  $events['hybridauth_identity_added'] = array(
    'label' => t('User added HybridAuth identity'),
    'group' => t('HybridAuth'),
    'variables' => array(
      'hybridauth_user' => array(
        'label' => t('User adding the identity'),
        'type' => 'user',
      ),
      'hybridauth' => array(
        'label' => t('HybridAuth identity data'),
        'type' => 'hybridauth',
      ),
    ),
    'module' => 'hybridauth',
  );
  $events['hybridauth_identity_deleted'] = array(
    'label' => t('User deleted HybridAuth identity'),
    'group' => t('HybridAuth'),
    'variables' => array(
      'hybridauth_user' => array(
        'label' => t('User deleting the identity'),
        'type' => 'user',
      ),
      'hybridauth' => array(
        'label' => t('HybridAuth identity data'),
        'type' => 'hybridauth',
      ),
    ),
    'module' => 'hybridauth',
  );
  $events['hybridauth_user_insert'] = array(
    'label' => t('User registered through HybridAuth'),
    'group' => t('HybridAuth'),
    'variables' => array(
      'hybridauth_user' => array(
        'label' => t('registered user'),
        'type' => 'user',
      ),
      'hybridauth' => array(
        'label' => t('HybridAuth identity data'),
        'type' => 'hybridauth',
      ),
    ),
    'module' => 'hybridauth',
  );
  $events['hybridauth_user_login'] = array(
    'label' => t('User logged in through HybridAuth'),
    'group' => t('HybridAuth'),
    'variables' => array(
      'hybridauth_user' => array(
        'label' => t('logged in user'),
        'type' => 'user',
      ),
      'hybridauth' => array(
        'label' => t('HybridAuth identity data'),
        'type' => 'hybridauth',
      ),
    ),
    'module' => 'hybridauth',
  );
  
  return $events;
}

/**
 * Implements hook_rules_condition_info().
 * @ingroup rules
 */
function hybridauth_rules_condition_info() {
  $conditions = array();
  
  $conditions['hybridauth_user_created_hybridauth'] = array(
    'label' => t('User created by HybridAuth'),
    'group' => t('HybridAuth'),
    'parameter' => array(
      'account' => array('label' => t('User'), 'type' => 'user'),
    ),
  );
  
  return $conditions;
}

function hybridauth_user_created_hybridauth($account) {
  global $hybridauth_data;
  return (!empty($hybridauth_data) || !empty($account->data['hybridauth']));
}
