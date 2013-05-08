<?php

define('REPORT_NAME_MAX_LENGTH', 27);

/**
 * Provides possibility to create reports
 */
function report_builder_form($form, &$form_state, $report = NULL) {
  $form['title'] = array(
    '#title' => t('Report title'),
    '#type' => 'textfield',
    '#description' => t('Specify report title'),
    '#default_value' => isset($report->title) ? $report->title : ''
  );
  $form['machine_name'] = array(
    '#type' => 'textfield',
    '#title' => t('machine name'),
    '#default_value' => isset($report->machine_name) ? $report->machine_name : '',
    '#maxlength' => REPORT_NAME_MAX_LENGTH,
    '#description' => t('A unique name to construct the report machine name. It must only contain lowercase letters, numbers and underscores.'),
    '#element_validate' => array('report_builder_machine_name_validate'),
    '#machine_name' => array(
      'exists' => 'report_builder_machine_name_exists',
      'source' => array('title'),
      'label' => t('Machine name'),
      'replace_pattern' => '[^a-z0-9_]+',
      'replace' => '_',
    ),
  );
  $form['sql_query'] = array(
    '#title' => t('SQL query'),
    '#description' => t('Determine SQL query, that would select necessary data from DB. After WHERE statement use
    placeholder !!PARAMS!!'), // @todo write good descriptions and determine placeholder
    '#type' => 'textarea',
    '#default_value' => isset($report->query) ? $report->query : ''
  );
  $form['header'] = array(
    '#type' => 'text_format',
    '#title' => t('Header'),
    '#description' => t('Create title here'),
    '#default_value' => isset($report->header) ? theme('table', array('rows' => $report->header)) : '',
  );
  $form['fields'] = array(
    '#title' => t('Fields'),
    '#description' => t('Please, specify fields to display in report separated by comma'),
    '#type' => 'textarea',
    '#default_value' => isset($report->fields) ? $report->fields : ''
  );
  $params_count = isset($form_state['params_count']) ? $form_state['params_count'] :
    (isset($report->params) ? count($report->params) : 1);

  $form['params'] = array(
    '#prefix' => '<div id = "params-wrapper">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );
  $elements = Elements_Factory::getElements();
  for ($i = 0; $i < $params_count; $i++) {
    $form['params'][$i] = array(
      '#theme' => 'table_elements',
      '#cols' => 5
    );
    $form['params'][$i]['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Parameter name'),
      '#default_value' => isset($report->params[$i]['name']) ? $report->params[$i]['name'] : '',
    );
    $form['params'][$i]['database_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Parameter name in database'),
      '#default_value' => isset($report->params[$i]['database_name']) ? $report->params[$i]['database_name'] : '',
    );
    $form['params'][$i]['type'] = array(
      '#type' => 'select',
      '#title' => t('Parameter type'),
      '#options' => $elements,
      '#default_value' => isset($report->params[$i]['type']) ? $report->params[$i]['type'] : '',
    );
    $form['params'][$i]['options'] = array(
      '#type' => 'textarea',
      '#title' => t('Specify params for select'),
      '#states' => array(
        'visible' => array(
          ':input[name="params['.$i.'][type]"]' => array('value' => 'List_Element'), //@todo
      ),
      ),
      '#default_value' => isset($report->params[$i]['options']) ? $report->params[$i]['options'] : '',
    );
    $form['params'][$i]['options_is_query'] = array(
      '#type' => 'checkbox',
      '#title' => t('Chech if options is sql query'),
      '#states' => array(
        'visible' => array(
          ':input[name="params['.$i.'][type]"]' => array('value' => 'List_Element'), //@todo
        ),
      ),
      '#default_value' => isset($report->params[$i]['options_is_query']) ? $report->params[$i]['options_is_query'] : '',
    );
  }
  $form['add_more'] = array(
    '#type' => 'submit',
    '#value' => t('Add more'),
    '#name' => 'add_more', // If we leave here simple $term->tid it generates fatal error
    '#submit' => array('report_builder_add_more'),
    '#ajax' => array(
      'callback' => 'report_builder_params_js',
      'wrapper' => 'params-wrapper',
      'effect' => 'fade',
    ),
  );
  $form['id'] = array(
    '#type' => 'value',
    '#value' => isset($report->id) ? $report->id : '',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
 
  return $form;
}

/**
 * Implements hook_form_submit
 */
function report_builder_form_submit($form, &$form_state) {
  require_once drupal_realpath('sites/all/libraries/phpQuery/phpQuery.php');
  $doc = phpQuery::newDocumentHTML($form_state['values']['header']['value']);

  $rows = array();
  foreach($doc->find('tr') as $v) {
    $row = array();
    foreach (pq($v)->find('td') as $vv) {
      $td = pq($vv);
      $element = array();
      $element['colspan'] = $td->attr('colspan');
      $element['rowspan'] = $td->attr('rowspan');
      $element['data'] = trim($td->html());
      $row[] = $element;
    }
    $rows[] = $row;
  }
  $params = array(
    'query' => $form_state['values']['sql_query'],
    'title' => $form_state['values']['title'],
    'header' => $rows,
    'params' => array_filter($form_state['values']['params'], function($val) { return !empty($val['name']); }),
    'machine_name' => $form_state['values']['machine_name'],
    'id' => $form_state['values']['id'],
    'fields' => $form_state['values']['fields'],
  );
  try {
    $report = new Report_Builder($params);
    $report->save();
  } catch (Exception $e) {
    drupal_set_message(t($e->getMessage()), 'error');
  }
  drupal_set_message(t('Report added'));
}

function report_builder_add_more($form, &$form_state) {
  $form_state['params_count'] = count($form_state['values']['params']) + 1;
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback
 */
function report_builder_params_js($form, $form_state) {
  return $form['params'];
}

function report_builder_machine_name_validate($form, &$form_state) {
  die('a');
}

function report_builder_machine_name_exists($value) {
  $link_exists = db_result(db_query_range("SELECT 1 FROM {reports} WHERE machine_name = '%s'", 0, 1, $value));

  return $link_exists;
}

/**
 * Displays filter for specified report
 */
function report_builder_filter_form($form, &$form_state, $report) {
  $form['filter'] = array(
    '#tree' => TRUE
  );
  foreach ($report->get_filter() as $element) {
     $form['filter'][$element['database_name']] = Elements_Factory::createElement($element['type'], $element)->getPresentation();
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create'),
  );
  return $form;
}

/**
 * Prepares data for report
 */
function report_builder_filter_form_submit($form, &$form_state) {
  $_SESSION['filters'] = array();
  //Build filter data
  foreach ($form_state['values']['filter'] as $name => $filter) {
    if (!empty($filter['element'])) {
      $_SESSION['filters'][] = array(
        'name' => $name,
        'value' => $filter['type'] !== 'timestamp' ? $filter['element'] : strtotime($filter['element']),
        'operation' => $filter['operation'],
      );
    }
  }
  $form_state['redirect'] = array('reports/'.arg(1).'/view');
}

