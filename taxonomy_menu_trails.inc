<?php

// $Id$

/**
 * @file
 * Processing functions for taxonomy_menu_trails.
 *
 * @author Dmitriy.trt      <http://drupal.org/user/329125>
 */

function _taxonomy_menu_trails_get_path($term) {
  //Integration with Taxonomy Menu modules
  if (module_exists('taxonomy_menu') && variable_get('taxonomy_menu_trails_integration_with_tm', TRUE))
    return taxonomy_menu_create_path($term->vid, $term->tid);
  else
    return taxonomy_term_path($term);
}

function _taxonomy_menu_trails_process($item) {
  $href = NULL;
  $taxonomy = &$item['page_arguments'][0]->taxonomy;
  switch (variable_get('taxonomy_menu_trails_selection_method', 'first')) {
    case 'first':
      $term = reset($taxonomy);
      break;
    case 'last':
      $term = end($taxonomy);
      break;
    case 'first-with-menu':
      for ($term = reset($taxonomy); $term !== FALSE; $term = next($taxonomy)) {
        $href = _taxonomy_menu_trails_get_path($term);
        if (db_result(db_query('SELECT COUNT(*) FROM {menu_links} WHERE link_path = "%s" AND hidden = 0', $href)) > 0) {
          break;
        }
      }
      break;
    case 'last-with-menu':
      for ($term = end($taxonomy); $term !== FALSE; $term = prev($taxonomy)) {
        $href = _taxonomy_menu_trails_get_path($term);
        if (db_result(db_query('SELECT COUNT(*) FROM {menu_links} WHERE link_path = "%s" AND hidden = 0', $href)) > 0) {
          break;
        }
      }
      break;
    case 'deepest-in-lineage':
      $node = $item['page_arguments'][0];
      $term = _taxonomy_menu_trails_get_deepest_term($node);
      break;
    default:
      return;
  }
  
  if ($term === NULL)
    return;
  
  if ($href === NULL)
    $href = _taxonomy_menu_trails_get_path($term);
  $item['href'] = $href;
  
  menu_set_item(NULL, $item);
}

function _taxonomy_menu_trails_get_deepest_term($node) {
  $types = variable_get('taxonomy_menu_trails_node_types', array());

  if (empty($types[$node->type])) {
    return NULL;
  }
  $vids = $types[$node->type];
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
}
