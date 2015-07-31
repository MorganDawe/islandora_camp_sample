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
function islandora_camp_sample_admin(array $form, array $form_state) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora_camp_sample', 'includes/admin.form');
  $form = array();
  $objects = islandora_solution_packs_get_required_objects('islandora_camp_sample');
  $form['islandora_camp_sample_required_object'] = array(
    '#type' => 'fieldset',
    '#title' => t('Islandora Camp Sample Objects'),
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

  $form['islandora_camp_sample_required_object']['objects'] = array(
    '#type' => 'tableselect',
    '#header' => $headers,
    '#options' => $options,
  );

  $form['islandora_camp_sample_required_object']['islandora_camp_sample_required_object_install'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Install Objects'),
  );

  $form['islandora_camp_sample_required_object']['islandora_camp_sample_required_object_delete'] = array(
    '#type' => 'button',
    '#executes_submit_callback' => TRUE,
    '#value' => t('Delete Objects'),
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
function islandora_camp_sample_admin_submit($form, $form_state) {
  switch ($form_state['clicked_button']['#id']) {
    case 'edit-islandora-camp-sample-required-object-install':
      $not_checked = array();
      foreach ($form_state['values']['objects'] as $key => $object) {
        if ($object === 0) {
          $not_checked[] = $form['islandora_camp_sample_required_object']['objects']['#options'][$key]['pid'];
        }
      }
      $batch = islandora_solution_pack_get_batch('islandora_camp_sample', $not_checked);
      batch_set($batch);
      break;

    case 'edit-islandora-camp-sample-required-object-delete':
      $checked = array();
      foreach ($form_state['values']['objects'] as $key => $object) {
        if ($object !== 0) {
          $checked[] = $form['islandora_camp_sample_required_object']['objects']['#options'][$key];
        }
      }
      $batch = islandora_camp_sample_delete_batch($checked);
      batch_set($batch);
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
function islandora_camp_sample_delete_batch($checked = array()) {
  $batch = array(
    'title' => t('Installing / Updating solution pack objects'),
    'file' => drupal_get_path('module', 'islandora_camp_sample') . '/includes/admin.form.inc',
    'operations' => array(),
  );
  foreach ($checked as $check_op) {
    $object = islandora_object_load($check_op['pid']);
    if ($object) {
      $batch['operations'][] = array('islandora_camp_sample_delete_object', array($object));
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
function islandora_camp_sample_delete_object(AbstractObject $object, array &$context) {
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
