<?php

class Meta_Array_Test extends PHPUnit_Framework_TestCase {
  function test_remove_using_key() {
    // Use Meta_Object just to trigger autoloading MetaHub since Meta_Array is mostly internal and not a part of autoload.
    $object = new Meta_Object();
    $array['key'] = 'value';
    Meta_Array::remove($array, 'value');
    $this->assertEquals(0, count($array));
  }
  
  function test_remove_using_index() {
    // Use Meta_Object just to trigger autoloading MetaHub since Meta_Array is mostly internal and not a part of autoload.
    $object = new Meta_Object();
    $array[0] = 'value';
    Meta_Array::remove($array, 'value');
    $this->assertEquals(0, count($array));
  }

}