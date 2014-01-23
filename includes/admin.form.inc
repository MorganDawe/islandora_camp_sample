<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function dgi_sample_admin(array $form, array $form_state) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'dgi_sample', 'includes/admin.form');
  $form = array();
  $objects = islandora_solution_packs_get_required_objects('dgi_sample');
  $form['dgi_sample_required_object'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sample Object'),
    '#weight' => 0,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $headers = array(
    'label' => t('Label'),
    'pid' => t('PID'),
    'status' => t('status'),
  );

  $options = array();
  $ok_image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
  $warning_image = theme_image(array('path' => 'misc/watchdog-warning.png', 'attributes' => array()));
  $status_info = array(
    'up_to_date' => array(
      'solution_pack' => t('All required objects are installed and up-to-date.'),
      'image' => $ok_image,
      'button' => t("Force reinstall objects"),
    ),
    'modified_datastream' => array(
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects"),
    ),
    'out_of_date' => array(
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects"),
    ),
    'missing_datastream' => array(
      'solution_pack' => t('Some objects must be reinstalled. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Reinstall objects"),
    ),
    'missing' => array(
      'solution_pack' => t('Some objects are missing and must be installed. See objects list for details.'),
      'image' => $warning_image,
      'button' => t("Install objects"),
    ),
  );
  foreach ($objects['objects'] as $object) {
    $object_status = islandora_check_object_status($object);
    $object_status_info = $status_info[$object_status['status']];
    $exists = $object_status['status'] != 'missing';
    $label = $exists ? l($object->label, "islandora/object/{$object->id}") : $object->label;
    $status_msg = "{$object_status_info['image']}&nbsp{$object_status['status_friendly']}";
    $options[$object->id] = array(
      'label' => $label,
      'pid' => $object->id,
      'status' => $status_msg);
  }

  $node_headers = array(
    'title' => t('Title'),
    'nid' => t('Node ID'),
    'status' => t('Status'),
  );

  $nodes = array(
    'About' => '3dfbfeaf-cb77-4ac0-b16f-145af17649b5',
    'Contact' => '329a2134-140a-4447-a59f-389948bef2ba',
  );

  $node_options = array();
  foreach ($nodes as $key => $uuid) {
    $nid = dgi_sample_uuid_get_nid($uuid);
    $node_state = node_load($nid);
    if ($node_state) {
      $href = url("node/$nid");
      $node_title = t('<a href="!href">!key</a>', array('!href' => $href, '!key' => $key));
    }
    else {
      $node_title = t("!key", array('!key' => $key));
    }

    $node_options[$key] = array(
      'title' => $node_title,
      'nid' => $nid,
      'status' => $node_state ? "{$status_info['up_to_date']['image']}&nbspup-to-date" : "{$status_info['missing']['image']}&nbspmissing",
    );
  }

  $form['dgi_sample_required_object']['objects'] = array(
    '#type' => 'tableselect',
    '#header' => $headers,
    '#options' => $options,
  );

  $form['dgi_sample_required_object']['dgi_sample_required_object_install'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Install Objects'),
  );

  $form['dgi_sample_required_object']['dgi_sample_required_object_delete'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Delete Objects'),
  );

  $form['dgi_sample_required_node'] = array(
    '#type' => 'fieldset',
    '#title' => t('Required Node'),
    '#weight' => 5,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['dgi_sample_required_node']['nodes'] = array(
    '#type' => 'tableselect',
    '#header' => $node_headers,
    '#options' => $node_options,
  );
  $form['dgi_sample_required_node']['dgi_sample_required_node_install'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Install Nodes'),
  );

  $form['dgi_sample_required_node']['dgi_sample_required_node_delete'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Delete Nodes'),
  );

  $form['dgi_sample_pid_list'] = array(
    '#type' => 'fieldset',
    '#title' => t('Accordion Pid List'),
    '#weight' => 5,
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['dgi_sample_pid_list']['velue'] = array(
    '#type' => 'item',
    '#markup' => t('<b>Accordion Pid List value:</b>&nbsp!value', array('!value' => dgi_sample_get_pid_list())),
  );
  $form['dgi_sample_pid_list']['dgi_sample_pid_list_button'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Reset Accordion Pid List'),
  );

  return $form;
}

/**
 * Form submit.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function dgi_sample_admin_submit($form, $form_state) {
  switch ($form_state['clicked_button']['#id']) {
    case 'edit-dgi-sample-required-object-install':
      $not_checked = array();
      foreach ($form_state['values']['objects'] as $key => $object) {
        if ($object === 0) {
          $not_checked[] = $form['dgi_sample_required_object']['objects']['#options'][$key]['pid'];
        }
      }
      $batch = islandora_solution_pack_get_batch('dgi_sample', $not_checked);
      batch_set($batch);
      break;

    case 'edit-dgi-sample-required-object-delete':
      $checked = array();
      foreach ($form_state['values']['objects'] as $key => $object) {
        if ($object !== 0) {
          $checked[] = $form['dgi_sample_required_object']['objects']['#options'][$key];
        }
      }
      $batch = dgi_sample_delete_batch($checked);
      batch_set($batch);
      break;

    case 'edit-dgi-sample-required-node-install':
      if ($form_state['values']['nodes']['About']) {
        dgi_sample_required_about_node();
      }
      if ($form_state['values']['nodes']['Contact']) {
        dgi_sample_required_contact_node();
      }
      break;

    case 'edit-dgi-sample-required-node-delete':
      $about_nid = dgi_sample_uuid_get_nid('3dfbfeaf-cb77-4ac0-b16f-145af17649b5');
      $contact_nid = dgi_sample_uuid_get_nid('329a2134-140a-4447-a59f-389948bef2ba');
      if (node_load($about_nid) && $form_state['values']['nodes']['About']) {
        node_delete($about_nid);
      }
      if (node_load($contact_nid) && $form_state['values']['nodes']['Contact']) {
        node_delete($contact_nid);
      }
      break;

    case 'edit-dgi-sample-pid-list-button':
      dgi_sample_accordion_pid_list();
      break;
  }
}

/**
 * Get the delete batch array.
 * 
 * @param array $checked
 *   An array of checked object.
 * 
 * @return array
 *   The batch array.
 */
function dgi_sample_delete_batch($checked = array()) {
  $batch = array(
    'title' => t('Installing / Updating solution pack objects'),
    'file' => drupal_get_path('module', 'dgi_sample') . '/includes/admin.form.inc',
    'operations' => array(),
  );
  foreach ($checked as $check_op) {
    $object = islandora_object_load($check_op['pid']);
    if ($object) {
      $batch['operations'][] = array('dgi_sample_delete_object', array($object));
    }
  }
  return $batch;
}

/**
 * Batch operation to delete required object(s).
 *
 * @param AbstractObject $object
 *   The object to delete.
 * @param array $context
 *   The context of this batch operation.
 */
function dgi_sample_delete_object(AbstractObject $object, array &$context) {
  $existing_object = islandora_object_load($object->id);
  if ($existing_object) {
    $deleted = islandora_delete_object($existing_object);
    if (!$deleted) {
      $object_link = l($existing_object->label, "islandora/object/{$existing_object->id}");
      drupal_set_message(filter_xss(t('Failed to purge existing object !object_link.', array(
        '!object_link' => $object_link,
          ))), 'error');
      return;
    }
  }
}

/**
 * Get the node id by uuid.
 * 
 * @param string $uuid
 *   The uuid.
 * 
 * @return int
 *   The node id or NULL if the node was not found.
 */
function dgi_sample_uuid_get_nid($uuid) {
  $query = new EntityFieldQuery();
  $entities = $query->entityCondition('entity_type', 'node')
      ->propertyCondition('uuid', $uuid)
      ->range(0, 1)
      ->execute();
  $nodes = isset($entities['node']) ? array_keys($entities['node']) : array();
  return empty($nodes) ? NULL : $nodes[0];
}
