<?php

class FormElementValidateError extends Exception {
  private $child_element;
  private $untranslated_message;
  private $message_params;

  /**
   * @param string $message
   *  Untranslated message with tokens
   * @param array $params
   *  Replacement tokens for $message
   * $param string $child_element
   *  Partial string for child form element like 'min][year'
   */
  public function __construct($message, $params = array(), $child_element = NULL, $code = 0, Exception $previous = null) {
    $this->untranslated_message = $message;
    $this->message_params = $params;
    $this->child_element = $child_element;

    parent::__construct(t($message, $params), $code, $previous);
  }

  /**
   * Build string compatible with first argument of form_set_error()
   *
   * @param string $parent
   *  String that identifies parent form element like 'filter][period'
   * @return string
   *  String like 'filter][period][min'
   */
  public function getFormElementName($parent) {
    if ($this->child_element) {
      return $parent . $this->child_element;
    } else {
      return $parent;
    }
  }
}

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
      'DateExact_Element' => t('Date (exact))'),
      'List_Element' => t('List'),
      'Number_Element' => t('Number'),
      'NumberExact_Element' => t('Number'),
      'Text_Element' => t('Text'),
      'DatePeriod_Element' => t('Date period'),
    );
  }
}

abstract class Element {
  protected $title = '';
  protected $default;
  protected $operation;

  public function __construct($vars) {
    $this->title = $vars['name'];
    if (isset($vars['operation'])) {
      $this->operation = $vars['operation'];
    }
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

  /**
   * Checks if default values are valid
   *
   * @throws Exception
   */
  abstract public function validate();
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

  public function validate() {
    if ($this->default['operation'] == 'contains' && $this->default['element'] == '') {
      throw new FormElementValidateError('Contains operation requires text value.', array(), 'element');
    }
  }
}

class Date_Element extends Element {

  public function getPresentation($type = 'timestamp') {
    $element = array(
      '#tree' => TRUE,
      'element' => array(
        '#type' => 'date_popup',
        '#title' => $this->title,
        '#date_format' => 'd.m.Y',
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'type' => array(
        '#type' => 'value',
        '#value' => $type,
      ),
    );
    
    if ($this->operation) {
      $element['operation'] = array(
        '#type' => 'value',
        '#value' => $this->operation,
      );
    } else {
      $element['operation'] = array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => isset($this->default['operation']) ? $this->default['operation'] : '',
      );
      $element['#theme'] = 'table_elements';
    }

    return $element;
  }
  
  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
  
  public function validate() {
    if (!strtotime($this->default['element'])) {
      throw new FormElementValidateError('Invalid date value @value.', array('@value' => $this->default['element']), 'element');
    }
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
  
  public function validate() {
    if (!strtotime($this->default['min_value'])) {
      throw new FormElementValidateError('Invalid date value @value.', array('@value' => $this->default['min_value']), 'min_value');
    }
    if (!strtotime($this->default['max_value'])) {
      throw new FormElementValidateError('Invalid date value @value.', array('@value' => $this->default['max_value']), 'max_value');
    }
  }
}


class Number_Element extends Element {

  public function getPresentation() {
    $element = array(
      '#tree' => TRUE,
      'element' => array(
        '#type' => 'textfield',
        '#title' => $this->title,
        '#element_validate' => array('element_validate_integer'),
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : '',
      ),
      'operation' => array()
    );
    if ($this->operation) {
      $element['operation'] = array(
        '#type' => 'value',
        '#value' => $this->operation,
      );
    } else {
      $element['operation'] = array(
        '#type' => 'select',
        '#title' => t('Operation'),
        '#options' => $this->getParams(),
        '#default_value' => isset($this->default['operation']) ? $this->default['operation'] : '',
      );
      $element['#theme'] = 'table_elements';
    }

    return $element;
  }
  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
    
  public function validate() {
    if (!is_numeric($this->default['element'])) {
      throw new FormElementValidateError('Invalid number value @value.', array('@value' => $this->default['element']), 'element');
    }
  }
}

class List_Element extends Element {

  protected $options = array();

  public function __construct($vars) {
    parent::__construct($vars);
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

  public function validate() {
    //Drupal does all validation for select elements
  }

}

/**
 * Checkboxes param implementation class.
 *
 * @see Element
 */
class Checkboxes_Element extends Element {

  protected $options = array();

  public function __construct($vars) {
    parent::__construct($vars);
    $this->options = $vars['options'];
  }

  public function getPresentation() {
    return array(
      '#tree' => TRUE,
      '#theme' => 'table_elements',
      'element' => array(
        '#type' => 'checkboxes',
        '#title' => $this->title,
        '#options' => $this->options,
        '#default_value' => isset($this->default['element']) ? $this->default['element'] : array(),
      ),
    );
  }

  public function getParams() {
    return array('=' => '=', '>' => '>', '<' => '<');
  }
  
  public function validate() {
    //Drupal does all validation for checkboxes elements
  }
}
