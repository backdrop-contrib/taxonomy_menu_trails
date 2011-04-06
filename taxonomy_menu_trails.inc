<?php


/**
 * @file
 * Processing functions for taxonomy_menu_trails.
 *
 * @todo: write description for each function
 *
 * @author Dmitriy.trt      <http://drupal.org/user/329125>
 */

/* TODO integration with Taxonomy Menu if it is still needed
function _taxonomy_menu_trails_get_path($term) {
  //Integration with Taxonomy Menu modules
  if (module_exists('taxonomy_menu') && variable_get('taxonomy_menu_trails_integration_with_tm', TRUE))
    return taxonomy_menu_create_path($term->vid, $term->tid);
  else
    return taxonomy_term_path($term);
}*/

function _taxonomy_menu_trails_process($entity_type, $bundle, $entity) {
  list($settings) = _taxonomy_menu_trails_get_settings($entity_type, $bundle, FALSE);
  $instances = $settings['instances'];
  $tids = array();
  //TODO figure out meaning of language in term reference fields
  foreach ($instances as $field) {
    foreach ($entity->$field as $lang => $terms) {
      foreach ($terms as $term) {
        $tids[] = $term['tid'];
      }
    }
  }

  
  if ($settings['term_with_menu']) {
    // rather crazy method to get terms with menu items
    // can we replace it with something more correct?
    
    $filtered_tids = array();
    $paths = array();
    foreach ($tids as $tid) {
      //TODO taxonomy menu integration
      $paths[] = 'taxonomy/term/' . $tid;
    }
    //TODO fetch full link object (grab code from menu_link_get_preferred())
    // this will save us one query, so we can remove term_with_menu option
    // completely (force it to "enabled")
    $result = db_select('menu_links', 'ml')
      ->fields('ml', array('mlid', 'link_path'))
      ->condition('ml.link_path', $paths, 'IN')
      ->condition('ml.hidden', FALSE)
      ->execute()
      ->fetchAllKeyed();
    
    $tid_position = strlen('taxonomy/term/');
    foreach ($result as $mlid => $path) {
      $tid = substr($path, $tid_position);
      $tid = intval($tid);

      $filtered_tids[] = $tid;
    }
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

  //TODO taxonomy menu integration if needed
  $selected_path = 'taxonomy/term/' . $selected_tid;
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
