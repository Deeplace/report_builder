<?php
/**
 * Interface Report
 */
interface Report {

  /**
   * Get report identifier (machine name)
   *
   * @return string - report machine name
   */
  function get_machine_name();

  /**
   * Builds report data
   *
   * @return array - array to pass into theme_table function
   */
  function query_data();

  /**
   * Creates header for report
   *
   * @return array - header rows for report table
   */
  function get_header();

  /**
   * Returns report title
   *
   * @return string
   */
  function get_title();

  /**
   * Creates filter for report
   *
   * Filter array. Format:
   *     [0]=> - Element number
   *       array(5) {
   *         ["name"] => string - filter name for displaying in UI
   *         ["database_name"]=> string - name of field in database
   *         ["type"] => string - for element type @see Elements.class.inc
   *         ["options"] => array - Optional. Options for list elements
   *         ["options_is_query"] => 1 or 0 - Optional. Only For List elements in cases when options are created
   *                                          from SQL QUERY
   *     }
   *
   * @return array - list of elements for building the filter
   */
  function get_filter();
}

/**
 * Class Base_Report
 *
 * Base class for Reports
 */
class Base_Report implements Report {

  public function get_machine_name() {
    return get_class($this);
  }
  
  function get_filter() {
    return array();
  }

  function query_data() {
    return array();
  }

  function get_header() {
    return array();
  }

  function get_title() {
    return '';
  }
}

/**
 * Class Report_Factory
 *
 * Factory class for reports
 */
class Report_Factory {

  /**
   * List of directories containing .inc files with report classes
   */
  protected $report_paths = array();

  /**
   * Constructor
   *
   * @param string $path
   *  Path to reports directory
   * @see spl_autoload()
   * @link http://us.php.net/manual/en/function.spl-autoload.php
   */
  public function __construct($path) {
    $this->report_paths[] = $path;
    //Used by spl_autoload()
    set_include_path(get_include_path() . PATH_SEPARATOR . './' . $path);
  }

  /**
   * Builds list of existing reports
   *
   * @return array
   */
  public function get_reports() {
    $db_reports = self::db_reports();
    $file_reports = $this->file_reports();
    return array_merge($file_reports, $db_reports);
  }
  /**
   * Gets list of reports from database
   *
   * @return array $result - list of reports
   */
  public static function db_reports() {
    $query=db_select('reports','n')
      ->fields('n',array('machine_name','title'))
      ->execute();
    $result = array();
    foreach ($query as $row) {
      $result[$row->machine_name] = $row->title;
    }
    return $result;
  }

  /**
   * Returns list of "handmade" reports
   *
   * @return array $result - list of reports. machine_name => title
   */
  protected function file_reports() {
    $reports = array();
    foreach ($this->report_paths as $path) {
      $files = file_scan_directory($path, '/(.*)\.inc/');
      foreach ($files as $file) {
        $class_name = ucfirst($file->name);
        $reports[$class_name] = $class_name::$title;
      }
    }
    return $reports;
  }

  public static function create_report($rep_name) {
    try {
      $report = new Report_Builder($rep_name);
    } catch (Exception $e) {
      if (class_exists($rep_name)) {
        $report = new $rep_name();
      }
      else {
        $report = FALSE;
      }
    }
    return $report;
  }

}

/**
 * Class Report_Builder
 *
 * Creates report from database entries
 */
class Report_Builder implements Report {

  public $title = '';
  public $machine_name = '';
  public $query = '';
  public $params = '';
  public $header = '';
  public $id = '';
  public $fields = '';

  protected $where;

  public function __construct($param) {
    if (is_array($param)) {
      foreach ($param as $key => $element) {
        if (isset($this->$key)) {
          $this->$key = $element;
        }
      }
    }
    else {
      $this->machine_name = $param;
      $this->load();
    }
    return $this;
  }

  public function get_machine_name() {
    return $this->machine_name;
  }

  public function save() {
    try {
      $this->params = serialize($this->params);
      $this->header = serialize($this->header);
      unset($this->where); //@todo rewrite
      if ($this->id) {
        $num_updated = db_update('reports')
          ->condition('id', $this->id)
          ->fields((array)$this)
          ->execute();
      }
      else {
        $this->id = db_insert('reports')
          ->fields(array_filter((array)$this))
          ->execute();

      }
    } catch (PDOException $e) {
      watchdog('report_builder', 'failed to insert/update report @details', array('@details' => $e->getMessage()), WATCHDOG_ERROR);
      throw new Exception('failed to insert/update report');
    }
    watchdog('report_builder', 'Report inserted successfully @details', array('@details' => $this->id), WATCHDOG_NOTICE);
  }

  public function delete(){
    $num_deleted = db_delete('reports')
      ->condition('machine_name', $this->machine_name)
      ->execute();

    if (!$num_deleted) {
      watchdog('report_builder', 'Failed to delete report @details', array('@details' => var_export($this, TRUE)), WATCHDOG_ERROR);
      throw new Exception('Report can not be deleted');
      return;
    }

    watchdog('report_builder', 'Report deleted @details', array('@details' => var_export($this, TRUE)), WATCHDOG_NOTICE);
  }
   
  protected function load() {
    $report = db_select('reports', 'r')
      ->fields('r')
      ->condition('machine_name', $this->machine_name)
      ->execute()
      ->fetchObject();
    if (!$report) {
      throw new Exception('Report does not exists in database');
    }
    $this->id = $report->id;
    $this->header = unserialize($report->header);
    $this->params = unserialize($report->params);
    $this->query = $report->query;
    $this->title = $report->title;
    $this->fields = $report->fields;
  }

  public function query_data() {
    $sql = $this->build_where();
    $query = str_replace('!!PARAMS!!', $sql['where'], $this->query);
    try {
      $result = db_query($query, $sql['params']);
    } catch(PDOException $e) {
      watchdog('report_builder', 'SQL ERROR occurred @details', array('@details' => $e->getMessage()), WATCHDOG_ERROR);
      throw new Exception('Error Occurred! try again later');
    }
    $fields = $this->getFields();

    $rows = array();
    foreach ($result as $object) {
      $row = array();
      foreach ($fields as $field) {
        $row[] = $object->$field;
      }
      $rows[] = $row;
    }
    return $rows;
  }
  function get_title() {
    return $this->title;
  }
  protected function build_where() {
    $result = '1=1';
    $params = array();
    if (!empty($this->where)) {
      foreach ($this->where as $condition) {
        $placeholder = ":" . str_replace('.', '_', $condition['name']);
        if ($condition['operation'] == 'contains') {
          $result .= ' AND ' . $condition['name'] . " LIKE " . $placeholder;
          $params[$placeholder] = '%' . db_like($condition['value']) . '%';
        }
        else {
          $result .= ' AND ' . $condition['name'] . " " . $condition['operation'] . " " . $placeholder;
          $params[$placeholder] = $condition['value'];
        }
      }
    }
    return array('where' => $result, 'params' => $params);
  }

  public function set_where($where) {
    $this->where = $where;
  }

  protected function getFields() {
    return array_map('trim', explode(',', $this->fields));
  }

  public function get_header() {
    foreach ($this->header as &$row) {
      $row = array_map(function($value) { $value['header'] = TRUE; return $value; }, $row);
    }
    return $this->header;
  }

  public function get_filter() {
    return $this->params;
  }
}
