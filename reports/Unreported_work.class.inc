<?php
/**
 *
 */
class Unreported_work extends Base_Report {
  public static $title = "CONSTATAREA UTILIZĂRII MUNCII NEDECLARATE";
  function get_header() {
    $header = array(
      array(
        array('data' => 'Nr. d/o', 'header' => TRUE, ),
        array('data' => 'Denumirea agentului economic, locul amplasării','header' => TRUE, ),
        array('data' => 'Genul de activitate, lucrările la care erau antrenaţi', 'header' => TRUE, ),
        array('data' => 'Numărul de lucrători', 'header' => TRUE, ),
        array('data' => 'Procesul-verbal de control ', 'header' => TRUE, ),
        array('data' => 'Data efectuării controlului', 'header' => TRUE, ),
        array('data' => 'Numele, prenumele inspectorului de muncă', 'header' => TRUE, ),
        array('data' => 'Măsurile întreprinse(nr. art. CCA;decizia de sancţionare; valoarea amenzii; înlăturarea încălcărilor)', 'header' => TRUE, ),
      )
    );
    return $header;
  }
  function get_title() {
    return self::$title;
  }
  function query_data() {
    $query = db_select('im_act_control', 'ic');
    $query->fields('ic', array(
      'id',
      'company_tid',
      'author',
      'act_date',
      'number',
      'emp_disabled'))
      ->condition('ic.emp_disabled', 0, '>');
    $query = $query->execute();
    $it = 0;
    foreach ($query as $object) {
      $row = array();
      $row[] = ++$it;
      $company = taxonomy_term_load($object->company_tid);
      $row[] = $company->name . ', ' . $company->field_address[LANGUAGE_NONE][0]['value'];
      $activityType = taxonomy_term_load($company->field_activity_type[LANGUAGE_NONE][0]['tid']);
      $row[] = $activityType->field_activity_code[LANGUAGE_NONE][0]['value'];
      $row[] = $object->emp_disabled;
      $row[] = $object->number;
      $dossier = Dossier::getDossierId($object->id, Dossier::DOCUMENT_TYPE_ACT_CONTROL);
      $did = db_select('im_dossier_document', 'idd')
        ->fields('idd', array('document_id'))
        ->condition('dossier_id', $dossier)
        ->condition('document_type', Dossier::DOCUMENT_TYPE_DISPOSITION)
        ->execute()
        ->fetchField();
      $disposition = Disposition::loadDisposition($did);
      $row[] = date('d.m.Y', strtotime($object->act_date));
      $row[] = implode('<br />', array_map(function($value) {return theme('userfullname', array('user' => user_load($value)));}, $disposition->inspectors));
      $row[] = '';
      $this->data[] = $row;
    }
    return $this->data;
  }

}