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
  
  if ($settings['term_with_menu']) {
    $paths_tids = array();
    foreach ($tids as $tid) {
      $path = _taxonomy_menu_trails_get_path($tid, $settings, $vids);
      $paths_tids[$path] = $tid;
    }
    //TODO fetch full link object (grab code from menu_link_get_preferred())
    // this will save us one query, so we can remove term_with_menu option
    // completely (force it to "enabled")
    $result = db_select('menu_links', 'ml')
      ->fields('ml', array('mlid', 'link_path'))
      ->condition('ml.link_path', array_keys($paths_tids), 'IN')
      ->condition('ml.hidden', FALSE)
      ->execute()
      ->fetchAllKeyed();
    
    $filtered_tids = array();
    foreach ($result as $mlid => $path) {
      if (isset($paths_tids[$path])) {
        $tid = $paths_tids[$path];
        $filtered_tids[$tid] = $tid;
      }
    }

    //preserve tids order from original array
    $filtered_tids = array_intersect($tids, $filtered_tids);
  }
  else {
    $filtered_tids =& $tids;
  }

  $selected_tid = NULL;
  switch ($settings['selection_method']) {
    case 'first':
      $selected_tid = reset($filtered_tids);
      break;
    case 'last':
      $selected_tid = end($filtered_tids);
      break;
    //TODO case 'deepest-in-lineage':
    default:
      return;
  }
  
  if (!$selected_tid) {
    return;
  }

  //TODO use already fetched path
  $selected_path = _taxonomy_menu_trails_get_path($selected_tid, $settings, $vids);
  //TODO use already fetched link object (minus one sql query)
  $active_item = menu_link_get_preferred($selected_path);
  $preferred_links = &drupal_static('menu_link_get_preferred');
  $preferred_links[$_GET['q']] = $active_item;
}

/* TODO function _taxonomy_menu_trails_get_deepest_term($node, $vids) {
  $res = db_query('SELECT tn.tid, t.vid, th.parent FROM {term_node} tn INNER JOIN {term_data} t ON tn.tid = t.tid INNER JOIN {term_hierarchy} th ON tn.tid = th.tid WHERE tn.vid = %d AND t.vid IN(' . db_placeholders($vids) . ')', array_merge(array($node->vid), array_values($vids)));
  $terms = array();
  while ($row = db_fetch_object($res)) {
    $terms[$row->tid] = $row;
  }
  if (empty($terms)) {
    return NULL;
  }
  $term = NULL;
  $maxdepth = 0;
  foreach ($terms as $tid => $t) {
    if (!isset($t->depth)) {
      $parent = $t->parent;
      $depth = 1;
      $parents = array($tid);
      while ($parent != 0 && !empty($terms[$parent])) {
        if (isset($terms[$parent]->depth)) {
          $depth += $terms[$parent]->depth;
          break;
        }
        $parents[] = $parent;
        $parent = $terms[$parent]->parent;
        $depth++;
      }
      while ($pid = array_shift($parents)) {
        $terms[$pid]->depth = $depth--;
      }
    }
    if ($terms[$tid]->depth > $maxdepth) {
      $maxdepth = $terms[$tid]->depth;
      $term = $t;
    }
  }
  return $term;
}*/
