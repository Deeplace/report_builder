<?php
class Elements_Factory {

  public static function createElement($elementName, $vars) {
    if (class_exists($elementName)) {
      return new $elementName($vars);
    }
    return FALSE;
  }

  public static function getElements() {
    return array(
      'Date_Element' => t('Date'),
      'List_Element' => t('List'),
      'Number_Element' => t('Number'),
      'Text_Element' => t('Text'),
    );
  }
}

abstract class Element {
  protected $title = '';

  public function __construct($vars) {
    $this->title = $vars['name'];
  }

  abstract function getPresentation();
  abstract function getParams();
}

class Text_Element extends Element {

  public function getPresentation() {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      'element' => array(
        '#type' => 'textfield',
        '#title' => $this->title,
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
      )
    );
  }

  /**
   * Creates list of parameters
   */
  public function getParams() {
    return array('=' => '=', 'contains' => t('Contains'));
  }
}

class Date_Element extends Element {

  public function getPresentation($type = 'timestamp') {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      'element' => array(
        '#type' => 'date_popup',
        '#title' => $this->title,
        '#date_format' => 'd.m.Y',
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
      ),
      'type' => array(
        '#type' => 'value',
        '#value' => $type,
      ),
    );
  }
  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
}

class Number_Element extends Element {

  public function getPresentation() {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      'element' => array(
        '#type' => 'textfield',
        '#title' => $this->title,
        '#element_validate' => array('element_validate_integer'),
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
      )
    );
  }
  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
}

class List_Element extends Element {

  protected $options = array();

  public function __construct($vars) {
    $this->title = $vars['name'];
    $this->options = $vars['options_is_query'] == 1 ? $this->queryOptions($vars['options']) : $this->convertOptions($vars['options']);
  }

  public function getPresentation() {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      'element' => array(
        '#type' => 'select',
        '#title' => $this->title,
        '#options' => $this->options
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
      )
    );
  }

  protected function queryOptions($query) {
    $query = db_query($query);
    $options = array();

    foreach ($query as $row) {
      $options[$row->key] = $row->value;
    }
    return $options;
  }

  protected function convertOptions($options) {
    $options = explode("\n", $options);
    $result = array();
    foreach ($options as $option) {
      list($key, $value) = explode('|', $option);
      $result[$key] = $value;
    }
    return $result;
  }

  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
}