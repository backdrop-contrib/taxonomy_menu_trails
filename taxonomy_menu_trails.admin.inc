<?php

/**
 * @file
 * Settings forms alteration for taxonomy_menu_trails.
 *
 * @todo: write description for each function
 *
 * @author Dmitriy.trt      <http://drupal.org/user/329125>
 */

function _taxonomy_menu_trails_sort_by_weight_helper($a, $b) {
  $vars = &drupal_static('_taxonomy_menu_trails_sort_by_weight');
  $a_info = field_info_instance($vars['entity_type'], $a, $vars['bundle']);
  $b_info = field_info_instance($vars['entity_type'], $b, $vars['bundle']);
  return $a_info['widget']['weight'] - $b_info['widget']['weight'];
}

function _taxonomy_menu_trails_sort_by_weight(&$instances, $entity_type, $bundle) {
  $static = &drupal_static(__FUNCTION__, array());
  $static = compact('entity_type', 'bundle');
  uksort($instances, '_taxonomy_menu_trails_sort_by_weight_helper');
  $static = array();
}

function _taxonomy_menu_trails_alter_entity_form(&$form, $entity_type, $bundle) {
  $bundle = field_extract_bundle($entity_type, $bundle);
  list($settings) = _taxonomy_menu_trails_get_settings($entity_type, $bundle);

  $all_instances = field_info_instances($entity_type, $bundle);
  $instances_options = array();
  foreach ($all_instances as $instance) {
    $field_info = field_info_field($instance['field_name']);
    if ($field_info['type'] == 'taxonomy_term_reference') {
      $instances_options[$instance['field_name']] = $instance['label'];
    }
  }
  
  if (!empty($instances_options)) {
    _taxonomy_menu_trails_sort_by_weight($instances_options, $entity_type, $bundle);

    $subform = array(
      '#tmt_entity' => $entity_type,
      '#tmt_bundle' => $bundle,
      'instances' => array(
        '#type' => 'checkboxes',
        '#title' => t('Term references for setting active trail'),
        '#options' => $instances_options,
        '#default_value' => $settings['instances'],
      ),
      'selection_method' => array(
        // TODO convert it to 'value' if there is one term reference with
        // Number of values == 1
        '#type' => 'select',
        '#title' => t("Term selection method"),
        '#description' => t("This option defines how module chooses term for processing:<ul><li><b>First/Last</b> - just select lightest/haviest term (but it can be the term without menu item). Best performance.</li><li><b>Deepest in lineage</b> - use deepest term in selected lineage. All parent terms in lineage must also be attached for this function to work. It's highly recommended to use Hierarchical Select for lineage saving. Low performance.</ul>"),
        '#default_value' => $settings['selection_method'],
        '#options' => array(
          'first' => t('First term'),
          'last' => t('Last term'),
          //TODO 'deepest-in-lineage' => t('Deepest in lineage'),
        ),
        /*TODO depends on http://drupal.org/node/1057748
        '#states' => array(
          'invisible' => array(
            'input[name^="taxonomy_menu_trails[instances]"]' => array('checked' => FALSE),
          ),
        ),*/
      ),
      'term_with_menu' => array(
        '#type' => 'checkbox',
        '#title' => t("Skip terms without enabled menu item"),
        '#default_value' => $settings['term_with_menu'],
        /*TODO also depends on http://drupal.org/node/1057748
        '#states' => array(
          'invisible' => array(
            'input[name^="taxonomy_menu_trails[instances]"]' => array('checked' => FALSE),
          ),
        ),*/
      ),
      'only_without_menu' => array(
        '#type' => 'checkbox',
         //TODO replace "object" with human-friendly entity type
        '#title' => t("Only if object doesn't have enabled menu item"),
        '#default_value' => $settings['only_without_menu'],
        /*TODO also depends on http://drupal.org/node/1057748
        '#states' => array(
          'invisible' => array(
            'input[name^="taxonomy_menu_trails[instances]"]' => array('checked' => FALSE),
          ),
        ),*/
      ),
    );
    
    // adding our submit function before default handler because
    // we don't want our elements to be saved as persistent variables
    // by default node type submit handler
    array_unshift($form['#submit'], 'taxonomy_menu_trails_entity_form_submit');
  }
  else {
    $subform = array(
      'notice' => array(
        '#markup' => '<p>' . t('Add some "Term reference" fields to use Taxonomy Menu Trails.') . '</p>'
      ),
    );
  }
  
  $form['taxonomy_menu_trails'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('Taxonomy menu trails'),
  ) + $subform;
}

function taxonomy_menu_trails_entity_form_submit($form, &$state) {
  //TODO handle machine-readable name change
  $entity_type = $form['taxonomy_menu_trails']['#tmt_entity'];
  $bundle = $form['taxonomy_menu_trails']['#tmt_bundle'];
  list($settings, $var_name, $var_exists) = _taxonomy_menu_trails_get_settings($entity_type, $bundle);
  $values = !empty($state['values']['taxonomy_menu_trails']) ? $state['values']['taxonomy_menu_trails'] : array();
  $settings = array_merge($settings, $values);
  $settings['instances'] = array_filter($settings['instances']);
  if (!empty($settings['instances'])) {
    //sort by instances weight again, because it could be changed since form generation
    _taxonomy_menu_trails_sort_by_weight($settings['instances'], $entity_type, $bundle);
    
    variable_set($var_name, $settings);
  }
  elseif ($var_exists) {
    variable_del($var_name);
  }
  unset($state['values']['taxonomy_menu_trails']);
}

function _taxonomy_menu_trails_alter_field_form(&$form) {
  list($settings) = _taxonomy_menu_trails_get_settings($form['#instance']['entity_type'], $form['#instance']['bundle'], FALSE);

  $form['instance']['taxonomy_menu_trails'] = array(
    '#tree' => FALSE,
    '#type' => 'fieldset',
    '#title' => t('Taxonomy menu trails'),
    'taxonomy_menu_trails_enabled' => array(
      '#type' => 'checkbox',
      '#title' => t('Use this term reference to set active trail'),
      '#default_value' => !empty($settings['instances'][$form['#field']['field_name']]),
    ),
  );
  $form['#submit'][] = 'taxonomy_menu_trails_field_form_submit';
}

function taxonomy_menu_trails_field_form_submit($form, &$state) {
  $entity_type = $form['#instance']['entity_type'];
  $bundle = $form['#instance']['bundle'];
  list($settings, $var_name, $var_exists) = _taxonomy_menu_trails_get_settings($entity_type, $bundle);
  $field_name = $form['#field']['field_name'];
  if (!empty($state['values']['taxonomy_menu_trails_enabled'])) {
    $settings['instances'][$field_name] = $field_name;
    _taxonomy_menu_trails_sort_by_weight($settings['instances'], $entity_type, $bundle);
  }
  else {
    unset($settings['instances'][$field_name]);
  }
  if (!empty($settings['instances'])) {
    variable_set($var_name, $settings);
  }
  elseif ($var_exists) {
    variable_del($var_name);
  }
}
