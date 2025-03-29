#include <DataPoint.h>

extern Preferences preferences;

DataPoint::DataPoint()
{
  onChanged = nullptr;
  _key = "";
}

void DataPoint::makeDataPersist(String key)
{
  // Utility::status(key);
  key.remove(10);
  _key = key;
  // Utility::status(_key);
}

void DataPoint::processValue(){}

void DataPoint::setCallback(std::function<void(String)> cbk)
{
  onChanged = cbk;
}

void DataPoint::clearCallback()
{
  onChanged = nullptr;
}


// BoolData::BoolData() : DataPoint() {}

void BoolData::makeDataPersist(String key, bool preset)
{
  DataPoint::makeDataPersist(key);
  if (!preferences.isKey((const char *)_key.c_str()))
  {
    preferences.putBool((const char *)_key.c_str(), preset);
  }
  _value = preferences.getBool((const char *)_key.c_str());
}

bool BoolData::getValue()
{
  return _value;
}

void BoolData::processValue()
{
  if (onChanged != nullptr)
  {
    onChanged((String)_value);
  }
}

void BoolData::setValue(bool val)
{
  if (val != _value)
  {
    _value = val;
    if(_key != "")
    {
      preferences.putBool((const char *)_key.c_str(), _value);
    }
    processValue();
  }
}

IntData::IntData(bool minutes) : DataPoint() 
{
  _minutes = minutes;
}

void IntData::makeDataPersist(String key, int preset)
{
  // Utility::status(key);
  DataPoint::makeDataPersist(key);
  if (!preferences.isKey((const char *)_key.c_str()))
  {
    preferences.putInt((const char *)_key.c_str(), preset);
  }
  _value = preferences.getInt((const char *)_key.c_str());
}

int IntData::getValue()
{
  return _value;
}

void IntData::processValue()
{
  if (onChanged != nullptr)
  {
    if(_minutes)
    {
      onChanged(Utility::minutesToTimeString(_value));
    }
    else
    {
      onChanged((String)_value);
    }
  }
}

void IntData::setValue(int val)
{
  if (val != _value)
  {
    _value = val;
    if(_key != "")
    {
      // Utility::status("saving " + (String)_key + " as " + (String)_value);
      preferences.putInt((const char *)_key.c_str(), _value);
    }
    processValue();
  }
}

// FloatData::FloatData() : DataPoint() {}

void FloatData::makeDataPersist(String key, float preset)
{
  DataPoint::makeDataPersist(key);
  if (!preferences.isKey((const char *)_key.c_str()))
  {
    preferences.putFloat((const char *)_key.c_str(), preset);
  }
  _value = preferences.getFloat((const char *)_key.c_str());
}

float FloatData::getValue()
{
  return _value;
}

void FloatData::processValue()
{
  if (onChanged != nullptr)
  {
    onChanged((String)_value);
  }
}

void FloatData::setValue(float val)
{
  if (val != _value)
  {
    _value = val;
    if(_key != "")
    {
      preferences.putFloat((const char *)_key.c_str(), _value);
    }
    processValue();
  }
}

DoubleData::DoubleData(bool fractionalMinute) : DataPoint() 
{
  _fractionalMinute = fractionalMinute;
}

void DoubleData::makeDataPersist(String key, double preset)
{
  DataPoint::makeDataPersist(key);
  if (!preferences.isKey((const char *)_key.c_str()))
  {
    preferences.putDouble((const char *)_key.c_str(), preset);
  }
  _value = preferences.getDouble((const char *)_key.c_str());
}

double DoubleData::getValue()
{
  return _value;
}

void DoubleData::processValue()
{
  if (onChanged != nullptr)
  {
    if (_fractionalMinute)
    {
      onChanged(Utility::fractionalMinutesToTimeString(_value));
    }
    else
    {
      onChanged((String)_value);
    }
  }
}

void DoubleData::setValue(double val)
{
  if (val != _value)
  {
    _value = val;
    if(_key != "")
    {
      preferences.putDouble((const char *)_key.c_str(), _value);
    }
    processValue();
  }
}

// StringData::StringData() : DataPoint() {}

void StringData::makeDataPersist(String key, String preset)
{
  DataPoint::makeDataPersist(key);
  if (!preferences.isKey((const char *)_key.c_str()))
  {
    preferences.putString((const char *)_key.c_str(), preset);
  }
  // might need to pass a buffer to getString(), possibly not for Arduino String()s
  _value = preferences.getString((const char *)_key.c_str());
}

String StringData::getValue()
{
  return _value;
}

void StringData::setValue(String val)
{
  if (val != _value)
  {
    _value = val;
    if (_key != "")
    {
      preferences.putString((const char *)_key.c_str(), _value);
    }
    processValue();
  }
}

void StringData::processValue()
{
  if (onChanged != nullptr)
  {
    onChanged(_value);
  }
}
