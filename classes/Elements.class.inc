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
      'DatePeriod_Element' => t('Date period'),
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

  /**
   * Set #default_value property of form element
   *
   * @param mixed $default_value
   *  If Element has child form elements, then $default_value should be an 
   *  array with values for children.
   *  Otherwise, it will be #default_value property of form element.
   * @return Element
   *  $this object
   * @access public
   */
  public function set_default($default_value) {
    $this->default = $default_value;
    return $this;
  }
}

class Text_Element extends Element {

  public function getPresentation() {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      'element' => array(
        '#type' => 'textfield',
        '#title' => $this->title,
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => isset($this->default['operation']) ? $this->default['operation'] : '',
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
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => isset($this->default['operation']) ? $this->default['operation'] : '',
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

class DateExact_Element extends Element {

  public function getPresentation($type = 'timestamp') {
    return array(
      '#tree' => TRUE,
      'element' => array(
        '#type' => 'date_popup',
        '#title' => $this->title,
        '#date_format' => 'd.m.Y',
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'operation' => array(
        '#type' => 'value',
        '#default_value' => '=',
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

class DatePeriod_Element extends Element {

  public function getPresentation($type = 'timestamp') {
    return array(
      '#tree' => TRUE,
      'min_value' => array(
        '#type' => 'date_popup',
        '#title' => $this->title,
        '#date_format' => 'd.m.Y',
        '#default_value' => isset($this->default['min_value']) ? $this->default['min_value'] : '',
      ),
      'max_value' => array(
        '#type' => 'date_popup',
        '#title' => t('to'),
        '#date_format' => 'd.m.Y',
        '#default_value' => isset($this->default['max_value']) ? $this->default['max_value'] : '',
      ),
      'operation' => array(
        '#type' => 'value',
        '#value' => 'between',
      ),
      'type' => array(
        '#type' => 'value',
        '#value' => $type,
      ),
    );
  }
  public function getParams() {
    return array('between' => 'between');
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
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => isset($this->default['operation']) ? $this->default['operation'] : '',
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
        '#options' => $this->options,
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => isset($this->default['operation']) ? $this->default['operation'] : '',
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
