<?php

function report_view_form($form, &$form_state){
  $form=array();

  $form['select']=array(
    '#title'=>t('Type of reports'),
    '#type'=>'select',
    '#options'=>Report_Factory::get_reports(),
    '#required' => TRUE,
  );

  $form['submit']=array(
    '#value'=>t('Submit'),
    '#type'=>'submit',
  );

  return $form;
}

function report_view_form_submit($form, &$form_state) {
  $form_state['redirect'] = array( 'reports/' . $form_state['values']['select'] . '/filter-form');
}

function report_builder_list() {

  $header = array(
    'title' => t('Title'),
    'operation' => t('Operation'),
  );

  $rows = array();

  foreach(Report_Factory::db_reports() as $machine_name => $title){
    $rows[] = array(
      array(  'data'=>$title),
      array(  'data'=> l(t('edit'), 'admin/settings/report/'.$machine_name.'/edit'). '  /  ' . l(t('delete'),'admin/settings/report/'.$machine_name.'/delete'))
    );
  }

  return 
      '<div class = "link-wrapper">' .
        l(t('Add report'), 'admin/settings/report-builder', array(
          'attributes'  => array('class' => array('xls-link')),
          'html'        => FALSE,
          )) .
      '</div>' .
         theme('table', $header, $rows);
}


