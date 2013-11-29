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
      'Date_Period_Element' => t('Date period'),
    );
  }
}

abstract class Element {
  protected $title = '';

  public function __construct($vars) {
    $this->title = $vars['name'];
  }
  public function setDefaults($value) {
    $this->defaults = $value;
    return $this;
  }

  abstract function getPresentation();
  abstract function getParams();

  /**
   * Creates filter acceptable by Report Builder
   * @param  array $value
   * @return array
   */
  public static function createFilter($name, $value) {
    return array(
      'name' => $name,
      'value' => $value['type'] !== 'timestamp' ? $value['element'] : strtotime($value['element']),
      'operation' => $value['operation'],
    );
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
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
      ),
      'class' => array(
        '#type' => 'value',
        '#value' => get_class($this),
      ),
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
      'class' => array(
        '#type' => 'value',
        '#value' => get_class($this),
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
      ),
      'class' => array(
        '#type' => 'value',
        '#value' => get_class($this),
      ),
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
        '#default_value' => !empty($this->defaults) ? $this->defaults['value'] : '',
      ),
      'operation' => array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => !empty($this->defaults) ? $this->defaults['operation'] : '',
      ),
      'class' => array(
        '#type' => 'value',
        '#value' => get_class($this),
      ),
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
    if (is_array($options)) {
      return $options;
    }
    else {
      $options = explode("\n", $options);
      $result = array();
      foreach ($options as $option) {
        list($key, $value) = explode('|', $option);
        $result[$key] = $value;
      }
      return $result;
    }
  }

  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
}

class Date_Period_Element extends Element {
  protected $title = '';

  public function __construct($vars) {
    $this->title = $vars['name'];
  }

  public function getPresentation($type = 'timestamp') {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      '#cols' => 4,
      'element_from' => array(
        '#type' => 'date_popup',
        '#title' => t('From'),
        '#date_format' => 'd.m.Y',
        '#default_value' => !empty($this->defaults) ? date('Y-m-d H:i:s', $this->defaults[0]['value']) : '',
      ),
      'element_to' => array(
        '#type' => 'date_popup',
        '#title' => t('To'),
        '#date_format' => 'd.m.Y',
        '#default_value' => !empty($this->defaults) ? date('Y-m-d H:i:s', $this->defaults[1]['value']) : '',
      ),
      'type' => array(
        '#type' => 'value',
        '#value' => $type,
      ),
      'class' => array(
        '#type' => 'value',
        '#value' => get_class($this),
      ),
    );
  }
  public function getParams() {
    return array();
  }

  public static function createFilter($name, $value) {
    return
      array(
        array(
          'name' => $name,
          'value' => $value['type'] !== 'timestamp' ? $value['element_from'] : strtotime($value['element_from']),
          'operation' => '>',
        ),
        array(
          'name' => $name,
          'value' => $value['type'] !== 'timestamp' ? $value['element_to'] : strtotime($value['element_to']),
          'operation' => '<',
        ),
      );
  }
}

class Complex_List_Element extends Element {
 protected $options = array();

  public function __construct($vars) {
    $this->title = $vars['name'];
    $this->callback = $vars['populate_callback'];
    $this->options = $vars['options'];
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
      'class' => array(
        '#type' => 'value',
        '#value' => get_class($this),
      ),
      'callback' => array(
        '#type' => 'value',
        '#value' => $this->callback,
      )
    );
  }

  public function getParams() {
    return array();
  }

  /**
   * Creates filter acceptable by Report Builder
   * @param  array $value
   * @return array
   */
  public static function createFilter($name, $value) {
    return array(
      'name' => $name,
      'value' => $value['callback']($value['element']),
      'operation' => 'IN',
    );
  }
}
