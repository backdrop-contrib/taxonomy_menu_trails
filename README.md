# Taxonomy Menu Trails

Toggle the active trail of menus with taxonomy term items based on terms attached
to content. Also sets the active breadcrumb trail.

Plays nicely with
 [Taxonomy Menu](https://backdropcms.org/project/taxonomy_menu), but that
 module's not required.

You need this module if you have a nested menu tree with taxonomy terms, e.g.:

```
    Term 1
        Term 1-1
    Term 2
        Term 2-1
```

And you have a node referring to the term in menu. For example, node refers to the Term 1-1.
 On the node page you'll see only the first level of menu, because the node itself doesn't
 have a menu item and the menu becomes collapsed:

```
    Term 1
    Term 2
```

The module solves this problem, so the menu will be expanded to display the term referred by current node:

```
    Term 1
        Term 1-1
    Term 2
```

## Installation

- Install this module using the official 
  [Backdrop CMS instructions](https://docs.backdropcms.org/documentation/extend-with-modules)

**Quick start**

Configure the module for each node type you want to expand the menu:

- Attach a term reference field to your node type (if not yet)
- Go to the node type settings page
- Switch to "Taxonomy Menu Trails" vertical tab
- Configure and save preferences

## Issues

Bugs and Feature requests should be reported in the 
[Issue Queue](https://github.com/backdrop-contrib/taxonomy_menu_trails/issues)

## Current Maintainers

- Minimally maintained by [Indigoxela](https://github.com/indigoxela)
- Additional maintainers welcome!

## Credits

- Originally written for Drupal by [Dmitriy.trt](https://www.drupal.org/u/dmitriytrt)

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
