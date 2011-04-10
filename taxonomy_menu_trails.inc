<?php


/**
 * @file
 * Processing functions for taxonomy_menu_trails.
 *
 * @todo: write description for each function
 *
 * @author Dmitriy.trt      <http://drupal.org/user/329125>
 */

function _taxonomy_menu_trails_get_path($tid, $settings, $vids = array()) {
  if ($settings['tm_integration'] && module_exists('taxonomy_menu') && isset($vids[$tid])) {
    return taxonomy_menu_create_path($vids[$tid], $tid);
  } else {
    return 'taxonomy/term/' . $tid;
  }
}

function _taxonomy_menu_trails_process($entity, $settings) {
  $instances = $settings['instances'];
  $tids = array();
  //TODO figure out: maybe we have to choose language in term reference fields
  foreach ($instances as $field) {
    foreach ($entity->$field as $lang => $terms) {
      foreach ($terms as $term) {
        $tids[] = $term['tid'];
      }
    }
  }

  if ($settings['selection_method'] == 'last') {
    $tids = array_reverse($tids);
  }

  $vids = array();
  if ($settings['tm_integration']) {
    // Unfortunately, there is no way to get tids-vids map without DB query
    // If you know one, please create issue and describe method
    $vids = db_select('taxonomy_term_data', 'ttd')
      ->fields('ttd', array('tid', 'vid'))
      ->condition('ttd.tid', $tids, 'IN')
      ->execute()
      ->fetchAllKeyed();
  }

  $selected_item = FALSE;
  $paths = array();
  foreach ($tids as $tid) {
    $path = _taxonomy_menu_trails_get_path($tid, $settings, $vids);
    $paths[] = $path;
  }

  // Retrieve a list of menu names, ordered by preference.
  $menu_names = menu_get_active_menu_names();

  $query = db_select('menu_links', 'ml', array('fetch' => PDO::FETCH_ASSOC));
  $query->leftJoin('menu_router', 'm', 'm.path = ml.router_path');
  $query->fields('ml');
  // Weight must be taken from {menu_links}, not {menu_router}.
  $query->addField('ml', 'weight', 'link_weight');
  $query->fields('m');
  $query->condition('ml.menu_name', $menu_names, 'IN');
  $query->condition('ml.link_path', $paths, 'IN');
  $query->condition('ml.hidden', '0');
  $query->orderBy('ml.weight', 'ASC');
  $results = $query->execute();

  // Sort candidates by link path and menu name.
  $candidates = array();
  foreach ($results as $candidate) {
    //Prefer the lightest of items with the same menu and path
    if (!isset($candidates[$candidate['link_path']][$candidate['menu_name']])) {
      $candidate['weight'] = $candidate['link_weight'];
      $candidates[$candidate['link_path']][$candidate['menu_name']] = $candidate;
    }
  }

  if (!empty($candidates)) {
    // Pick the most specific link, in the most preferred menu.
    foreach ($paths as $link_path) {
      if (!isset($candidates[$link_path])) {
        continue;
      }
      foreach ($menu_names as $menu_name) {
        if (!isset($candidates[$link_path][$menu_name])) {
          continue;
        }
        $candidate_item = $candidates[$link_path][$menu_name];
        $map = explode('/', $link_path);
        _menu_translate($candidate_item, $map);
        if ($candidate_item['access']) {
          $selected_item = $candidate_item;
        }
        break 2;
      }
    }
  }
  
  if (!$selected_item) {
    return;
  }

  $preferred_links = &drupal_static('menu_link_get_preferred');
  $preferred_links[$_GET['q']] = $selected_item;
}
