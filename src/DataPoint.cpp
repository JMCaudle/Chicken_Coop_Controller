#include <DataPoint.h>

DataPoint::DataPoint()
{
  onChanged = nullptr;
}

void DataPoint::processValue(){}

void DataPoint::setAndUseCallback(std::function<void(String)> cbk)
{
  onChanged = cbk;
  processValue();
}

void DataPoint::clearCallback()
{
  onChanged = nullptr;
}


// BoolData::BoolData() : DataPoint() {}

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
    processValue();
  }
}

// IntData::IntData() : DataPoint() {}

int IntData::getValue()
{
  return _value;
}

void IntData::processValue()
{
  if (onChanged != nullptr)
  {
    onChanged((String)_value);
  }
}

void IntData::setValue(int val)
{
  if (val != _value)
  {
    _value = val;
    processValue();
  }
}

// FloatData::FloatData() : DataPoint() {}

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
    processValue();
  }
}

DoubleData::DoubleData(bool fractionalMinute) : DataPoint() 
{
  _fractionalMinute = fractionalMinute;
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
    processValue();
  }
}

// StringData::StringData() : DataPoint() {}

String StringData::getValue()
{
  return _value;
}

void StringData::setValue(String val)
{
  if (val != _value)
  {
    _value = val;
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
