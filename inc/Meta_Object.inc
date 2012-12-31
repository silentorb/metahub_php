<?php

class Meta_Connection {

  public $other = null;
  public $parent = null;
  public $type = '';

  public function __construct($parent, $other, $type) {
    $this->parent = $parent;
    $this->other = $other;
    $this->type = $type;
  }

}

class MetaHub {
  static function get_connection($a, $b) {
    foreach ($a->internal_connections as $connection) {
      if ($connection->other === $b) {
        return $connection;
      }
    }

    return null;
  }

}

class Meta_Array {
  static function remove(&$array, $item) {
    $index = array_search($item, $array);
    if ($index !== false) {
      if (is_numeric($index)) {
        array_splice($array, $index, 1);
      }
      else {
        unset($array[$index]);
      }
    }
  }

}

class Meta_Object {

  public $internal_connections = array();
  public $is_meta_object = true;
  public $events = array();

  protected static function connect_objects($first, $other, $type) {
    $connection = MetaHub::get_connection($first, $other);
    if ($connection) {
      if ($connection->type != $type && $type) {
        $connection->type = $type;
        return true;
      }

      return false;
    }

    if ($type === 'parent')
      $first->parent = $other;

    $connection = new Meta_Connection($first, $other, $type);
    $first->internal_connections[] = $connection;
    return true;
  }

  function disconnect_objects($first, $other) {
    $connection = MetaHub::get_connection($first, $other);
    if ($connection) {
      $type = $connection->type;
      Meta_Array::remove($first->internal_connections, $connection);

      foreach ($other->events as $name => $event) {
        $first->unlisten($other, $name);
      }

      $connection->parent = null;
      $connection->other = null;

      $first->invoke('disconnect.' . $type, $other, $first);

      if ($connection->type === 'parent') {
        $parents = $first->get_connections('parent');
        if ($parents->length == 0) {
          unset($first->parent);
          if (!$first->__disconnecting_everything) {
            $first->disconnect_all();
          }
        }
        else {
          $first->parent = $parents[0];
        }
      }
    }
  }

  public function listen($other, $event, $method) {
    if ($other !== $this) {
      if (!$other->is_meta_object) {
        $this->connect($other);
      }
    }

//    if (!is_array($other->events[$event]))
//      $other->events[$event] = array();

    $info = new stdClass();
    $info->method = $method;
    $info->listener = $this;

    $other->events[$event][] = $info;
  }

  function invoke($event) {
    if ($this->events[$event]) {

      $args = func_get_args();
      array_shift($args);
      $info = $this->events[$event];
      foreach ($info as $listener) {
        call_user_func_array(array($listener->listener, $listener->method), $args);
      }
    }
    else if ($this->events['all']) {
      $args = func_get_args();
      $info = $this->events['all'];
      foreach ($info as $listener) {
        call_user_func_array(array($listener->listener, $listener->method), $args);
      }
    }
  }

  public function connect($other, $type = '', $other_type = null) {
    if ($other_type == null) {
      $other_type = $type;
    }

    if (!$other->is_meta_object)
      return;

    // The process_connect function can be added to a Meta_Object
    // to intercept potential connections
    if (gettype($this->process_connect) == 'function') {
      if ($this->process_connect($other, $type, $other_type) === false) {
        return;
      }
      else if (gettype($other->process_connect) == 'function') {
        if ($other->process_connect($this, $other_type, $type) === false) {
          return;
        }
      }
    }

    if (!Meta_Object::connect_objects($this, $other, $type, $other_type)) {
      return;
    }

    Meta_Object::connect_objects($other, $this, $other_type, $type);

    $this->invoke('connect.' . $type, $other, $this);
    $other->invoke('connect.' . $other_type, $this, $other);
  }

  function disconnect($other) {
    Meta_Object::disconnect_objects($this, $other);
    Meta_Object::disconnect_objects($other, $this);
  }

  function disconnect_all($type = null) {
    if ($type == null) {
      // $this is set to prevent repeated calls to disconnect_all->
      $this->__disconnecting_everything = true;
      for ($x = $this->internal_connections->length - 1; $x >= 0; --$x) {
        $this->disconnect($this->internal_connections[$x]->other);
      }
      $this->internal_connections = array();
      $this->invoke('disconnect-all', $this);
    }
    else {
      $connections = $this->get_connections($type);
      for ($x = $connections->length - 1; $x >= 0; --$x) {
        $this->disconnect($connections[$x]);
      }
    }

    unset($this->__disconnecting_everything);
  }

  function get_connections() {
    $filters = func_get_args();
    $first_filter = array_shift($filters);

    $result = array();
    if (is_string($first_filter)) {
      foreach ($this->internal_connections as $connection) {
        if ($connection->type == $first_filter) {
          $result[] = $connection->other;
        }
      }
    }
    else if (gettype($first_filter) == 'function') {
      foreach ($this->internal_connections as $connection) {
        if ($first_filter($connection->other)) {
          $result[] = $connection->other;
        }
      }
    }

    foreach ($filters as $filter) {
      if (is_string($filter)) {
        for ($x = count($result) - 1; $x >= 0; $x--) {
          if ($this->internal_connections[$result[$x]]->type != $filter) {
            array_splice($result, $x, 1);
          }
        }
      }
      else if (gettype($filter) == 'function') {
        for ($x = count($result) - 1; $x >= 0; $x--) {
          if (!$filter($result[$x])) {
            array_splice($result, $x, 1);
          }
        }
      }
    }

    return $result;
  }

  function get_connection($filter) {
    $connections = $this->get_connections($filter);
    return $connections[0];
  }

  function unlisten($other, $name) {
    if (!isset($other->events[$name]))
      return;

    $list = $other->events[$name];
    for ($i = count($list) - 1; $i >= 0; --$i) {
      if ($list[$i]->listener === $this) {
        array_splice($list, $i, 1);
      }
    }

    if (count($list) == 0) {
      unset($other->events[$name]);
    }
  }

}
