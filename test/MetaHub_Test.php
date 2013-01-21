<?php

class Test_Class_A {

  public $defined_but_not_set;
  public $defined_and_set = 'set';
  public $defined_but_never_changed = 'original';

}

class MetaHub_Test extends PHPUnit_Framework_TestCase {
  function test_extend() {
    $a = new Test_Class_A();
    $b = new stdClass();
    $b->defined_but_not_set = 100;
    $b->defined_and_set = 'set again';
    $b->not_defined = true;

    MetaHub::extend($a, $b, true);
    $this->assertEquals(100, $a->defined_but_not_set);
    $this->assertEquals('set again', $a->defined_and_set);
    $this->assertSame(null, $a->not_defined);
  }

  function test_extend_with_array_source() {
    $a = new Test_Class_A();
    $b = array(
        'defined_but_not_set' => 100,
        'defined_and_set' => 'set again',
        'not_defined' => true
    );
    MetaHub::extend($a, $b, true);
    $this->assertEquals(100, $a->defined_but_not_set);
    $this->assertEquals('set again', $a->defined_and_set);
    $this->assertSame(null, $a->not_defined);
  }

}
