<?php

class Test_Object extends Meta_Object {

  public $fired = false;

  function fire() {
    $this->fired = true;
  }

}

function test_fire($target, $event) {
  $object = new Test_Object();
  $object->listen($target, $event, fire);
  return $object;
}

class Meta_Object_Test extends PHPUnit_Framework_TestCase {
  function test_get_connection() {
    $parent = new Meta_Object();
    $child = new Meta_Object();
    $parent->connect($child, 'child', 'parent');

    $this->assertEquals(gettype($parent->internal_connections), 'array', "Parent should have a connection list->");
    $this->assertGreaterThan(0, count($parent->internal_connections));

    $connections = $parent->get_connections('child');
    $this->assertEquals(1, count($connections), "Parent should have one child.");
  }

  function test_metahub_get_connection() {
    $parent = new Meta_Object();
    $child = new Meta_Object();
    $parent->connect($child, 'child', 'parent');

    $connection = MetaHub::get_connection($parent, $child);
    $this->assertNotNull($connection);
    $connection = MetaHub::get_connection($child, $parent);
    $this->assertNotNull($connection);
  }

  function test_invoke() {
    $parent = new Meta_Object();
    $child = new Meta_Object();
    $parent->connect($child, 'child', 'parent');

    $object = test_fire($parent, 'invoke-test');
    $this->assertTrue(array_key_exists('invoke-test', $parent->events));
    $parent->invoke('invoke-test');
    $this->assertTrue($object->fired, "invoke-test was fired.");
  }

  function test_disconnect_objects() {
    $parent = new Meta_Object();
    $child = new Meta_Object();
    $parent->connect($child, 'child', 'parent');

    Meta_Object::disconnect_objects($parent, $child);
    Meta_Object::disconnect_objects($child, $parent);
    $this->assertEquals(0, count($parent->get_connections('child')), 'Parent should have no more children');
  }

  function test_disconnect() {
    $parent = new Meta_Object();
    $child = new Meta_Object();
    $parent->connect($child, 'child', 'parent');

    $object = test_fire($parent, 'disconnect.child');
    $parent->disconnect($child);

    $this->assertTrue($object->fired, "disconnect.child was fired");
    $this->assertEquals(count($parent->get_connections('child')), 0, 'Parent should have no more children');
    $this->assertEquals($child->$parent, null, "child should have no parent");
  }

}
